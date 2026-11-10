// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturerendermanager.h"
#include "base/executioncontroller.h"
#include "global.h"
#include "textureimage.h"
#include <QMap>
#include <QSize>
#include <QString>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <mutex>
#include <set>
#include <thread>
#include <utility>

namespace {

/// @brief Selects a small worker count based on available CPU resources.
/// @return The number of render workers to create.
std::size_t defaultWorkerCount() {
   constexpr unsigned int maxWorkerCount = 4;
   const unsigned int available = std::max(1U, std::thread::hardware_concurrency());
   return static_cast<std::size_t>(std::min(maxWorkerCount, available));
}

}  // namespace

TextureRenderManager::TextureRenderManager(ResultHandler resultHandler,
                                           FailureHandler failureHandler, std::size_t workerCount)
    : resultHandler(std::move(resultHandler)), failureHandler(std::move(failureHandler)) {
   if (workerCount == 0) {
      workerCount = defaultWorkerCount();
   }
   workers.reserve(workerCount);
   try {
      while (workers.size() < workerCount) {
         workers.emplace_back(&TextureRenderManager::runWorker, this);
      }
   } catch (...) {
      {
         std::lock_guard lock(mutex);
         stopping = true;
      }
      taskAvailable.notify_all();
      for (std::thread& worker : workers) {
         if (worker.joinable()) {
            worker.join();
         }
      }
      throw;
   }
}

TextureRenderManager::~TextureRenderManager() {
   cancel();
   {
      std::lock_guard lock(mutex);
      stopping = true;
      ++latestRenderSequence;
      runnableTasks.clear();
      currentRender.clear();
   }
   taskAvailable.notify_all();
   for (std::thread& worker : workers) {
      if (worker.joinable()) {
         worker.join();
      }
   }
}

void TextureRenderManager::render(TextureGraphSnapshot snapshot) {
   const QSize renderSize = snapshot.size;
   QSharedPointer<TextureGraphRenderState> renderState;
   try {
      renderState = createGraphRenderState(std::move(snapshot));
   } catch (const std::exception& error) {
      if (failureHandler) {
         failureHandler(TextureRenderFailure{0, renderSize, QString::fromUtf8(error.what())});
      }
      return;
   } catch (...) {
      if (failureHandler) {
         failureHandler(
             TextureRenderFailure{0, renderSize, QStringLiteral("Unknown render failure")});
      }
      return;
   }

   QSharedPointer<TextureGraphRenderState> previous;
   {
      std::lock_guard lock(mutex);
      if (stopping) {
         return;
      }
      previous = std::move(currentRender);
      ++latestRenderSequence;
      renderState->sequence = latestRenderSequence;
      runnableTasks.clear();
      currentRender = renderState;
      for (const auto& node : renderState->nodes) {
         if (node.second.remainingDependencies == 0) {
            runnableTasks.push_back(TextureNodeRenderTask{renderState, node.first});
         }
      }
   }
   if (previous) {
      previous->controller->cancel();
   }
   taskAvailable.notify_all();
}

void TextureRenderManager::cancel() {
   QSharedPointer<TextureGraphRenderState> previous;
   {
      std::lock_guard lock(mutex);
      ++latestRenderSequence;
      runnableTasks.clear();
      previous = std::move(currentRender);
   }
   if (previous) {
      previous->controller->cancel();
   }
   taskAvailable.notify_all();
}

QSharedPointer<TextureRenderManager::TextureGraphRenderState>
TextureRenderManager::createGraphRenderState(TextureGraphSnapshot snapshot) {
   auto renderState = QSharedPointer<TextureGraphRenderState>::create();
   renderState->size = snapshot.size;

   for (TextureNodeSnapshot& nodeSnapshot : snapshot.nodes) {
      const int nodeId = nodeSnapshot.nodeId;
      renderState->nodes.emplace(nodeId, TextureNodeRenderState{std::move(nodeSnapshot), 0, {}});
   }

   for (auto& nodeEntry : renderState->nodes) {
      TextureNodeRenderState& node = nodeEntry.second;
      std::set<int> uniqueSources;
      for (auto source = node.snapshot.sources.cbegin(); source != node.snapshot.sources.cend();
           ++source) {
         if (source.value() != 0 && renderState->nodes.count(source.value()) != 0) {
            uniqueSources.insert(source.value());
         }
      }
      node.remainingDependencies = static_cast<int>(uniqueSources.size());
      for (const int sourceId : uniqueSources) {
         renderState->nodes.at(sourceId).receivers.push_back(nodeEntry.first);
      }
   }

   renderState->unfinishedNodes = renderState->nodes.size();
   return renderState;
}

void TextureRenderManager::runWorker() {
   for (;;) {
      TextureNodeRenderTask task;
      {
         std::unique_lock lock(mutex);
         taskAvailable.wait(lock, [this] { return stopping || !runnableTasks.empty(); });
         if (stopping) {
            return;
         }
         task = std::move(runnableTasks.front());
         runnableTasks.pop_front();
         if (task.renderState->sequence != latestRenderSequence || task.renderState->failed) {
            continue;
         }
      }

      try {
         renderNode(task);
      } catch (const std::exception& error) {
         failRender(task, QString::fromUtf8(error.what()));
      } catch (...) {
         failRender(task, QStringLiteral("Unknown generator failure"));
      }
   }
}

void TextureRenderManager::renderNode(const TextureNodeRenderTask& task) {
   if (isObsolete(task.renderState->sequence) || task.renderState->controller->isCancelled()) {
      return;
   }

   const TextureNodeSnapshot& snapshot = task.renderState->nodes.at(task.nodeId).snapshot;
   if (!snapshot.cachedImage.isNull()) {
      completeNode(task, snapshot.cachedImage, false);
      return;
   }
   if (snapshot.generator.isNull()) {
      throw std::runtime_error("A texture node snapshot has no texture generator");
   }

   QMap<QString, TextureImagePtr> sourceImages;
   {
      std::lock_guard lock(mutex);
      if (stopping || task.renderState->sequence != latestRenderSequence ||
          task.renderState->failed) {
         return;
      }
      for (const QString& slot : snapshot.generator->getSourceSlots()) {
         const int sourceId = snapshot.sources.value(slot);
         if (sourceId != 0 && task.renderState->renderedImages.contains(sourceId)) {
            sourceImages.insert(slot, task.renderState->renderedImages.value(sourceId));
         }
      }
   }

   TextureNodeSettings settings = snapshot.settings;
   const TextureGeneratorSettings& generatorSettings = snapshot.generator->getSettings();
   for (const TextureGeneratorSetting& setting : generatorSettings) {
      if (!settings.contains(setting.id)) {
         settings.insert(setting.id, setting.defaultvalue);
      }
   }

   {
      std::lock_guard lock(mutex);
      if (task.renderState->sequence != latestRenderSequence || task.renderState->failed ||
          task.renderState->controller->isCancelled()) {
         return;
      }
      task.renderState->activeNodes[task.nodeId] = std::chrono::steady_clock::now();
   }
   TextureImagePtr image = TextureImage::create(task.renderState->size);
   snapshot.generator->generateWithTiming(task.renderState->size, image->data(), sourceImages,
                                          settings, task.renderState->controller);
   completeNode(task, image, true);
}

void TextureRenderManager::completeNode(const TextureNodeRenderTask& task,
                                        const TextureImagePtr& image, const bool publish) {
   bool runnableTasksAdded = false;
   {
      std::lock_guard lock(mutex);
      if (stopping || task.renderState->sequence != latestRenderSequence ||
          task.renderState->failed) {
         return;
      }

      task.renderState->activeNodes.erase(task.nodeId);
      task.renderState->renderedImages.insert(task.nodeId, image);
      const TextureNodeRenderState& completedNode = task.renderState->nodes.at(task.nodeId);
      for (const int receiverId : completedNode.receivers) {
         TextureNodeRenderState& receiver = task.renderState->nodes.at(receiverId);
         --receiver.remainingDependencies;
         if (receiver.remainingDependencies == 0 && !task.renderState->controller->isCancelled()) {
            runnableTasks.push_back(TextureNodeRenderTask{task.renderState, receiverId});
            runnableTasksAdded = true;
         }
      }

      if (task.renderState->unfinishedNodes > 0) {
         --task.renderState->unfinishedNodes;
      }
      if (task.renderState->unfinishedNodes == 0 && currentRender == task.renderState) {
         currentRender.clear();
      }
   }

   if (runnableTasksAdded) {
      taskAvailable.notify_all();
   }
   if (publish && resultHandler && !isObsolete(task.renderState->sequence)) {
      const TextureNodeSnapshot& snapshot = task.renderState->nodes.at(task.nodeId).snapshot;
      resultHandler(TextureRenderResult{snapshot.nodeId, snapshot.revision, task.renderState->size,
                                        image, task.renderState->sequence});
   }
}

void TextureRenderManager::failRender(const TextureNodeRenderTask& task, QString message) {
   bool reportFailure = false;
   {
      std::lock_guard lock(mutex);
      if (stopping || task.renderState->sequence != latestRenderSequence ||
          task.renderState->failed) {
         return;
      }
      task.renderState->activeNodes.erase(task.nodeId);
      if (task.renderState->controller->isCancelled()) {
         return;
      }
      task.renderState->failed = true;
      runnableTasks.clear();
      if (currentRender == task.renderState) {
         currentRender.clear();
      }
      reportFailure = true;
   }

   task.renderState->controller->cancel();
   if (reportFailure && failureHandler) {
      failureHandler(TextureRenderFailure{task.nodeId, task.renderState->size, std::move(message),
                                          task.renderState->sequence});
   }
}

bool TextureRenderManager::isObsolete(const std::uint64_t sequence) const {
   std::lock_guard lock(mutex);
   return stopping || sequence != latestRenderSequence;
}

void TextureRenderManager::stop() {
   QSharedPointer<TextureGraphRenderState> state;
   {
      std::lock_guard lock(mutex);
      runnableTasks.clear();
      state = currentRender;
   }
   if (state) {
      state->controller->cancel();
   }
}

std::optional<TextureSlowRender> TextureRenderManager::slowNode(
    std::chrono::milliseconds interval) const {
   std::lock_guard lock(mutex);
   if (!currentRender || currentRender->controller->isCancelled()) {
      return {};
   }
   for (const auto& [nodeId, started] : currentRender->activeNodes) {
      if (std::chrono::steady_clock::now() - started >= interval) {
         return TextureSlowRender{currentRender->sequence, nodeId,
                                  currentRender->nodes.at(nodeId).snapshot.generator->getName()};
      }
   }
   return {};
}

bool TextureRenderManager::isRunning(const TextureSlowRender& node) const {
   std::lock_guard lock(mutex);
   return currentRender && currentRender->sequence == node.sequence &&
          !currentRender->controller->isCancelled() &&
          currentRender->activeNodes.count(node.nodeId) != 0;
}

void TextureRenderManager::continueRendering(const TextureSlowRender& node) {
   std::lock_guard lock(mutex);
   if (currentRender && currentRender->sequence == node.sequence &&
       currentRender->activeNodes.count(node.nodeId) != 0) {
      currentRender->activeNodes[node.nodeId] = std::chrono::steady_clock::now();
   }
}

bool TextureRenderManager::hasPendingWork() const {
   std::lock_guard lock(mutex);
   if (!currentRender || currentRender->failed || currentRender->controller->isCancelled()) {
      return false;
   }
   return std::any_of(currentRender->nodes.begin(), currentRender->nodes.end(),
                      [this](const auto& node) {
                         return node.second.snapshot.cachedImage.isNull() &&
                                !currentRender->renderedImages.contains(node.first);
                      });
}
