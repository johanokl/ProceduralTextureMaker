// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturerendermanager.h"
#include "global.h"
#include "textureimage.h"
#include <QMap>
#include <QSize>
#include <QString>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
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
                                           FailureHandler failureHandler)
    : resultHandler(std::move(resultHandler)), failureHandler(std::move(failureHandler)) {
   const std::size_t workerCount = defaultWorkerCount();
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
   {
      std::lock_guard lock(mutex);
      stopping = true;
      ++latestRenderSequence;
      runnableTasks.clear();
      currentRender.reset();
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
   std::shared_ptr<TextureGraphRenderState> renderState;
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

   {
      std::lock_guard lock(mutex);
      if (stopping) {
         return;
      }
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
   taskAvailable.notify_all();
}

void TextureRenderManager::cancel() {
   {
      std::lock_guard lock(mutex);
      ++latestRenderSequence;
      runnableTasks.clear();
      currentRender.reset();
   }
   taskAvailable.notify_all();
}

std::shared_ptr<TextureRenderManager::TextureGraphRenderState>
TextureRenderManager::createGraphRenderState(TextureGraphSnapshot snapshot) {
   auto renderState = std::make_shared<TextureGraphRenderState>();
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
   if (isObsolete(task.renderState->sequence)) {
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

   QMap<int, TextureImagePtr> sourceImages;
   {
      std::lock_guard lock(mutex);
      if (stopping || task.renderState->sequence != latestRenderSequence ||
          task.renderState->failed) {
         return;
      }
      for (int slot = 0; slot < snapshot.generator->getNumSourceSlots(); ++slot) {
         const int sourceId = snapshot.sources.value(slot);
         if (sourceId != 0 && task.renderState->renderedImages.contains(sourceId)) {
            sourceImages.insert(slot, task.renderState->renderedImages.value(sourceId));
         }
      }
   }

   TextureNodeSettings settings = snapshot.settings;
   const TextureGeneratorSettings& generatorSettings = snapshot.generator->getSettings();
   for (auto setting = generatorSettings.cbegin(); setting != generatorSettings.cend(); ++setting) {
      if (!settings.contains(setting.key())) {
         settings.insert(setting.key(), setting.value().defaultvalue);
      }
   }

   TextureImagePtr image = TextureImage::create(task.renderState->size);
   snapshot.generator->generate(task.renderState->size, image->data(), sourceImages, &settings);
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

      task.renderState->renderedImages.insert(task.nodeId, image);
      const TextureNodeRenderState& completedNode = task.renderState->nodes.at(task.nodeId);
      for (const int receiverId : completedNode.receivers) {
         TextureNodeRenderState& receiver = task.renderState->nodes.at(receiverId);
         --receiver.remainingDependencies;
         if (receiver.remainingDependencies == 0) {
            runnableTasks.push_back(TextureNodeRenderTask{task.renderState, receiverId});
            runnableTasksAdded = true;
         }
      }

      if (task.renderState->unfinishedNodes > 0) {
         --task.renderState->unfinishedNodes;
      }
      if (task.renderState->unfinishedNodes == 0 && currentRender == task.renderState) {
         currentRender.reset();
      }
   }

   if (runnableTasksAdded) {
      taskAvailable.notify_all();
   }
   if (publish && resultHandler && !isObsolete(task.renderState->sequence)) {
      const TextureNodeSnapshot& snapshot = task.renderState->nodes.at(task.nodeId).snapshot;
      resultHandler(
          TextureRenderResult{snapshot.nodeId, snapshot.revision, task.renderState->size, image});
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
      task.renderState->failed = true;
      runnableTasks.clear();
      if (currentRender == task.renderState) {
         currentRender.reset();
      }
      reportFailure = true;
   }

   if (reportFailure && failureHandler) {
      failureHandler(TextureRenderFailure{task.nodeId, task.renderState->size, std::move(message)});
   }
}

bool TextureRenderManager::isObsolete(const std::uint64_t sequence) const {
   std::lock_guard lock(mutex);
   return stopping || sequence != latestRenderSequence;
}
