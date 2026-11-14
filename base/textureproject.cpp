
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureproject.h"
#include "projectfileservice.h"
#include <QJsonArray>
#include "generators/empty.h"
#include "base/texturegenerator.h"
#include "global.h"
#include "texturenode.h"
#include "texturerendermanager.h"
#include "settingsmanager.h"
#include <QDebug>
#include <QMap>
#include <QMapIterator>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QSet>
#include <QSize>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtLogging>
#include <QtCore/qtmetamacros.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

TextureProject::TextureProject(const bool automaticThumbnailRendering)
    : newIdCounter(0),
      emptygenerator(new EmptyGenerator()),
      thumbnailSize(250, 250),
      settingsManager(nullptr),
      modified(false),
      automaticThumbnailRendering(automaticThumbnailRendering) {
   renderManager = std::make_unique<TextureRenderManager>(
       [this](TextureRenderResult result) {
          QMetaObject::invokeMethod(
              this,
              [this, result = std::move(result)]() mutable {
                 publishRenderResult(std::move(result));
              },
              Qt::QueuedConnection);
       },
       [this](TextureRenderFailure failure) {
          QMetaObject::invokeMethod(
              this,
              [this, failure = std::move(failure)]() mutable {
                 publishRenderFailure(std::move(failure));
              },
              Qt::QueuedConnection);
       });
   scheduleThumbnailRender();
}

TextureProject::~TextureProject() {
   renderManager.reset();
   clear();
}

void TextureProject::setSettingsManager(SettingsManager* manager) {
   if (settingsManager) {
      QObject::disconnect(settingsManager, &SettingsManager::settingsUpdated, this,
                          &TextureProject::settingsUpdated);
   }
   settingsManager = manager;
   if (!settingsManager) {
      return;
   }
   settingsUpdated();
   QObject::connect(settingsManager, &SettingsManager::settingsUpdated, this,
                    &TextureProject::settingsUpdated);
}

void TextureProject::settingsUpdated() {
   if (!settingsManager) {
      return;
   }
   previewSize = settingsManager->getPreviewSize();
   const QSize previousThumbnailSize = thumbnailSize;
   thumbnailSize = settingsManager->getThumbnailSize();
   if (previousThumbnailSize != thumbnailSize) {
      renderManager->cancel();
      const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
      for (const TextureNodePtr& node : nodesCopy) {
         node->discardCachedImage(previousThumbnailSize);
      }
   }
   scheduleThumbnailRender();
}

void TextureProject::setName(const QString& newname) {
   name = newname;
   emit nameUpdated(name);
}

void TextureProject::loadFromJSON(const QJsonDocument& document) {
   const auto objects = document.array();
   QMap<QString, int> ids;
   for (const auto& value : objects) {
      const auto object = value.toObject();
      const QString identifier = object.value("id").toString();
      bool numeric = false;
      const int preferred = identifier.toInt(&numeric);
      const auto node = newNode(numeric && preferred > 0 && !getNode(preferred) ? preferred : 0);
      node->setIdentifier(identifier);
      ids.insert(identifier, node->getId());
   }
   for (const auto& value : objects) {
      const auto object = value.toObject();
      getNode(ids.value(object.value("id").toString()))->loadFromJSON(object, ids);
   }
   modified = false;
}

QJsonDocument TextureProject::saveAsJSON() const {
   QJsonArray result;
   for (const auto& node : nodesSnapshot()) {
      result.append(node->saveAsJSON());
   }
   return QJsonDocument(result);
}

bool TextureProject::isModified() const { return modified; }

void TextureProject::clear() {
   for (;;) {
      const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
      if (nodesCopy.isEmpty()) {
         break;
      }
      removeNode(nodesCopy.first()->getId());
   }
   {
      std::unique_lock lock(nodesMutex);
      newIdCounter = 0;
   }
}

int TextureProject::getNumNodes() const {
   std::shared_lock lock(nodesMutex);
   return nodes.count();
}

QList<int> TextureProject::getNodeIds() const {
   std::shared_lock lock(nodesMutex);
   return nodes.keys();
}

QList<int> TextureProject::getSinkNodeIds() const {
   const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
   QList<int> sinkIds;
   for (auto node = nodesCopy.cbegin(); node != nodesCopy.cend(); ++node) {
      if (node.value()->getNumReceivers() == 0) {
         sinkIds.append(node.key());
      }
   }
   return sinkIds;
}

int TextureProject::getNewId() {
   std::unique_lock lock(nodesMutex);
   ++newIdCounter;
   while (nodes.contains(newIdCounter)) {
      ++newIdCounter;
   }
   return newIdCounter;
}

QMap<int, TextureNodePtr> TextureProject::nodesSnapshot() const {
   std::shared_lock lock(nodesMutex);
   return nodes;
}

TextureGraphSnapshot TextureProject::createTextureGraphSnapshot(QSize renderSize, int targetNodeId,
                                                                bool useCachedImages) const {
   const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
   TextureGraphSnapshot snapshot;
   snapshot.size = renderSize;
   QList<int> pending = targetNodeId == 0 ? nodesCopy.keys() : QList<int>{targetNodeId};
   QSet<int> visited;
   while (!pending.isEmpty()) {
      const int id = pending.takeLast();
      if (visited.contains(id) || !nodesCopy.contains(id)) {
         continue;
      }
      visited.insert(id);
      auto node = nodesCopy.value(id)->createTextureNodeSnapshot(renderSize);
      if (targetNodeId != 0) {
         for (const int source : node.sources) {
            if (source != 0) {
               pending.append(source);
            }
         }
      }
      if (!useCachedImages) {
         node.cachedImage.clear();
      }
      snapshot.nodes.push_back(std::move(node));
   }
   return snapshot;
}

void TextureProject::scheduleThumbnailRender() {
   if (!renderManager || !automaticThumbnailRendering) {
      return;
   }
   auto snapshot = createTextureGraphSnapshot(thumbnailSize);
   for (const auto& node : snapshot.nodes) {
      auto state = TextureRenderState::Rendering;
      if (!node.cachedImage.isNull()) {
         state = TextureRenderState::Ready;
      } else if (renderingStopped) {
         state = TextureRenderState::Stopped;
      }
      setThumbnailRenderStatus(
          node.nodeId, state,
          state == TextureRenderState::Stopped
              ? QStringLiteral("Rendering stopped. Use View > Restart rendering to continue.")
              : QString());
   }
   if (!renderingStopped) {
      renderManager->render(std::move(snapshot));
   } else {
      // Edits while stopped invalidate queued results from the previous graph.
      renderManager->cancel();
   }
   emit renderingStateChanged();
}

void TextureProject::stopRendering() {
   renderingStopped = true;
   renderManager->stop();
   const auto statuses = thumbnailStatuses;
   for (auto status = statuses.cbegin(); status != statuses.cend(); ++status) {
      if (status->state == TextureRenderState::Rendering) {
         setThumbnailRenderStatus(
             status.key(), TextureRenderState::Stopped,
             QStringLiteral("Rendering stopped. Use View > Restart rendering to continue."));
      }
   }
   emit renderingStateChanged();
}

void TextureProject::restartRendering() {
   renderingStopped = false;
   scheduleThumbnailRender();
}

bool TextureProject::canStopRendering() const {
   return !renderingStopped && renderManager && renderManager->hasPendingWork();
}

bool TextureProject::canRestartRendering() const {
   if (!renderingStopped) {
      return false;
   }
   const auto nodesCopy = nodesSnapshot();
   return std::any_of(nodesCopy.cbegin(), nodesCopy.cend(), [this](const TextureNodePtr& node) {
      return node->cachedImage(thumbnailSize).isNull();
   });
}

TextureRenderStatus TextureProject::thumbnailRenderStatus(int id) const {
   return thumbnailStatuses.value(id);
}

void TextureProject::setThumbnailRenderStatus(int id, TextureRenderState state,
                                              const QString& message) {
   const auto previous = thumbnailStatuses.constFind(id);
   if (previous != thumbnailStatuses.cend() && previous->state == state &&
       previous->message == message) {
      return;
   }
   thumbnailStatuses.insert(id, {state, message});
   emit thumbnailRenderStatusChanged(id);
}

void TextureProject::markSaved() { modified = false; }

void TextureProject::publishRenderResult(TextureRenderResult result) {
   if (result.size != thumbnailSize || !renderManager->isCurrent(result.sequence)) {
      return;
   }
   const TextureNodePtr node = getNode(result.nodeId);
   if (!node.isNull()) {
      node->publishRenderedImage(result.size, result.revision, result.image);
   }
   emit renderingStateChanged();
}

void TextureProject::publishRenderFailure(TextureRenderFailure failure) {
   if (failure.sequence != 0 && !renderManager->isCurrent(failure.sequence)) {
      return;
   }
   QSet<int> affected;
   QList<int> pending{failure.nodeId};
   while (!pending.isEmpty()) {
      const int id = pending.takeLast();
      if (affected.contains(id)) {
         continue;
      }
      affected.insert(id);
      const auto node = getNode(id);
      if (!node.isNull()) {
         auto receivers = node->getReceivers();
         while (receivers.hasNext()) {
            pending.append(receivers.next());
         }
      }
   }
   const auto statuses = thumbnailStatuses;
   for (auto status = statuses.cbegin(); status != statuses.cend(); ++status) {
      if (status->state != TextureRenderState::Rendering) {
         continue;
      }
      if (status.key() == failure.nodeId || failure.nodeId == 0) {
         setThumbnailRenderStatus(status.key(), TextureRenderState::Failed, failure.message);
      } else if (affected.contains(status.key())) {
         setThumbnailRenderStatus(
             status.key(), TextureRenderState::Blocked,
             QStringLiteral("Input node %1 failed: %2").arg(failure.nodeId).arg(failure.message));
      } else {
         setThumbnailRenderStatus(
             status.key(), TextureRenderState::Cancelled,
             QStringLiteral("Rendering cancelled after node %1 failed.").arg(failure.nodeId));
      }
   }
   qWarning().noquote() << QStringLiteral("Texture render failed for node %1 at %2x%3: %4")
                               .arg(failure.nodeId)
                               .arg(failure.size.width())
                               .arg(failure.size.height())
                               .arg(failure.message);
   emit renderFailed(failure.nodeId, failure.size, failure.message);
   emit renderingStateChanged();
}

TextureNodePtr TextureProject::getNode(int id) const {
   std::shared_lock lock(nodesMutex);
   return nodes.value(id);
}

TextureNodePtr TextureProject::getNode(const QString& identifier) const {
   for (const auto& node : nodesSnapshot()) {
      if (node->getIdentifier() == identifier) {
         return node;
      }
   }
   return {};
}

TextureNodePtr TextureProject::newNode(int id, TextureGeneratorPtr generator) {
   if (generator.isNull()) {
      generator = emptygenerator;
   }

   TextureNodePtr newNode;
   {
      std::unique_lock lock(nodesMutex);
      if (id != 0 && nodes.contains(id)) {
         return nodes.value(id);
      }
      if (id == 0) {
         ++newIdCounter;
         while (nodes.contains(newIdCounter)) {
            ++newIdCounter;
         }
         id = newIdCounter;
      }
      newNode = TextureNodePtr(new TextureNode(this, generator, id));
      nodes.insert(id, newNode);
   }

   QObject::connect(newNode.data(), &TextureNode::nodesConnected, this,
                    &TextureProject::notifyNodesConnected);
   QObject::connect(newNode.data(), &TextureNode::nodesDisconnected, this,
                    &TextureProject::notifyNodesDisconnected);
   QObject::connect(newNode.data(), &TextureNode::imageUpdated, this,
                    &TextureProject::notifyImageUpdated);
   QObject::connect(newNode.data(), &TextureNode::imageAvailable, this,
                    &TextureProject::notifyImageAvailable);
   QObject::connect(newNode.data(), &TextureNode::positionUpdated, this,
                    [this](int) { modified = true; });
   QObject::connect(newNode.data(), &TextureNode::nameUpdated, this,
                    [this](int) { modified = true; });

   // TextureNode initializes its generator before the signal connections above exist, so its
   // initial imageUpdated signal cannot mark the project as changed. Node creation is itself a
   // document change and must be recorded explicitly.
   modified = true;
   emit nodeAdded(newNode);
   scheduleThumbnailRender();
   return newNode;
}

void TextureProject::addGenerator(const TextureGeneratorPtr& gen) {
   if (gen.isNull()) {
      return;
   }
   const QString settingsError = validateTextureGeneratorSettings(gen->getSettings());
   if (!settingsError.isEmpty()) {
      qWarning().noquote() << QStringLiteral("Generator '%1' has invalid settings: %2")
                                  .arg(gen->getName(), settingsError);
      return;
   }
   QSet<QString> uniqueSourceSlots;
   for (const QString& slot : gen->getSourceSlots()) {
      if (slot.trimmed().isEmpty() || uniqueSourceSlots.contains(slot)) {
         qWarning()
             << QStringLiteral("Generator '%1' has invalid source-slot names").arg(gen->getName());
         return;
      }
      uniqueSourceSlots.insert(slot);
   }
   if (generators.contains(gen->getName())) {
      emit generatorNameCollision(generators.value(gen->getName()), gen);
      return;
   }
   generators.insert(gen->getName(), gen);
   emit generatorAdded(gen);
}

void TextureProject::removeGenerator(const TextureGeneratorPtr& gen) {
   if (!gen.isNull() && generators.key(gen, "NULL") != "NULL") {
      generators.remove(gen->getName());
      emit generatorRemoved(gen);
   }
}

bool TextureProject::replaceGenerator(const TextureGeneratorPtr& oldGenerator,
                                      const TextureGeneratorPtr& newGenerator) {
   if (oldGenerator.isNull() || newGenerator.isNull() ||
       generators.value(oldGenerator->getName()) != oldGenerator) {
      return false;
   }
   const QString settingsError = validateTextureGeneratorSettings(newGenerator->getSettings());
   if (!settingsError.isEmpty()) {
      qWarning().noquote() << QStringLiteral("Generator '%1' has invalid settings: %2")
                                  .arg(newGenerator->getName(), settingsError);
      return false;
   }
   const TextureGeneratorPtr collision = generators.value(newGenerator->getName());
   if (!collision.isNull() && collision != oldGenerator) {
      emit generatorNameCollision(collision, newGenerator);
      return false;
   }

   const TextureGeneratorSettings oldDefinitions = oldGenerator->getSettings();
   const TextureGeneratorSettings newDefinitions = newGenerator->getSettings();
   const QList<int> nodeIds = getNodeIds();
   struct NodeState {
      TextureNodePtr node;
      TextureNodeSettings settings;
      QMap<QString, int> sources;
   };
   QList<NodeState> affectedNodes;
   for (const int nodeId : nodeIds) {
      const TextureNodePtr node = getNode(nodeId);
      if (!node.isNull() && node->getGenerator() == oldGenerator) {
         affectedNodes.append(NodeState{node, node->getSettings(), node->getSources()});
      }
   }

   generators.remove(oldGenerator->getName());
   generators.insert(newGenerator->getName(), newGenerator);
   emit generatorRemoved(oldGenerator);
   emit generatorAdded(newGenerator);

   for (const NodeState& state : std::as_const(affectedNodes)) {
      TextureNodeSettings migratedSettings;
      for (const TextureGeneratorSetting& newSetting : newDefinitions) {
         QVariant value = newSetting.defaultvalue;
         if (value.typeId() == QMetaType::QStringList) {
            const QStringList choices = value.toStringList();
            value = newSetting.defaultindex >= 0 && newSetting.defaultindex < choices.size()
                        ? QVariant(choices.at(newSetting.defaultindex))
                        : QVariant(QString());
         }
         const TextureGeneratorSetting* oldDefinition =
             findTextureGeneratorSetting(oldDefinitions, newSetting.id);
         if (oldDefinition != nullptr && state.settings.contains(newSetting.id) &&
             oldDefinition->defaultvalue.typeId() == newSetting.defaultvalue.typeId() &&
             oldDefinition->multiline == newSetting.multiline) {
            const QVariant previous = state.settings.value(newSetting.id);
            if (newSetting.defaultvalue.typeId() != QMetaType::QStringList ||
                newSetting.defaultvalue.toStringList().contains(previous.toString())) {
               value = previous;
            }
         }
         migratedSettings.insert(newSetting.id, value);
      }
      QMap<QString, int> migratedSources;
      for (const QString& slot : newGenerator->getSourceSlots()) {
         migratedSources.insert(slot, state.sources.value(slot));
      }
      state.node->replaceGeneratorDefinition(newGenerator, migratedSettings, migratedSources);
   }
   return true;
}

TextureGeneratorPtr TextureProject::getGenerator(const QString& name) const {
   if (!generators.contains(name)) {
      qDebug() << QString("No generator with name %1.").arg(name);
   }
   return generators.value(name, TextureGeneratorPtr(nullptr));
}

QString TextureProject::serializeNode(int id) {
   const auto node = getNode(id);
   return node ? QString::fromUtf8(
                     ProjectFileService::serializeNodes(QJsonArray{node->saveAsJSON()}))
               : QString();
}

int TextureProject::pasteNodes(const QString& json) {
   const auto document = QJsonDocument::fromJson(json.toUtf8());
   if (!document.isArray()) {
      return 0;
   }
   int count = 0;
   for (const auto& value : document.array()) {
      if (!value.isObject()) {
         continue;
      }
      auto object = value.toObject();
      if (!getGenerators().contains(object.value("generator").toString())) {
         continue;
      }
      const auto node = newNode();
      QMap<QString, int> ids;
      for (const auto& existing : nodesSnapshot()) {
         ids.insert(existing->getIdentifier(), existing->getId());
      }
      node->loadFromJSON(object, ids);
      node->setPos(node->getPos() + QPointF(getNumNodes() * 15, getNumNodes() * 15));
      ++count;
   }
   return count;
}

void TextureProject::removeNode(int id) {
   TextureNodePtr remNode = getNode(id);
   if (remNode.isNull()) {
      return;
   }
   remNode->release();
   {
      std::unique_lock lock(nodesMutex);
      nodes.remove(id);
   }
   // Removing an unconnected node emits no disconnection or image update signals.
   modified = true;
   thumbnailStatuses.remove(id);
   emit nodeRemoved(id);
   scheduleThumbnailRender();
}

void TextureProject::notifyImageUpdated(int id) {
   modified = true;
   scheduleThumbnailRender();
   emit imageUpdated(id);
}

void TextureProject::notifyImageAvailable(int id, QSize size) {
   if (size == thumbnailSize) {
      setThumbnailRenderStatus(id, TextureRenderState::Ready);
   }
   emit imageAvailable(id, size);
}

void TextureProject::notifyNodesConnected(int sourceId, int receiverId, QString slot) {
   modified = true;
   emit nodesConnected(sourceId, receiverId, slot);
}

void TextureProject::notifyNodesDisconnected(int sourceId, int receiverId, QString slot) {
   modified = true;
   emit nodesDisconnected(sourceId, receiverId, slot);
}

bool TextureProject::findLoops() const {
   const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
   QMapIterator<int, TextureNodePtr> nodeiterator(nodesCopy);
   while (nodeiterator.hasNext()) {
      if (nodeiterator.next().value()->findLoop()) {
         return true;
      }
   }
   return false;
}
