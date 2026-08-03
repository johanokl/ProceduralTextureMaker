
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureproject.h"
#include "generators/empty.h"
#include "base/texturegenerator.h"
#include "global.h"
#include "texturenode.h"
#include "texturerendermanager.h"
#include "settingsmanager.h"
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
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

void TextureProject::loadFromXML(const QDomDocument& xmlfile) {
   QDomNode rootNode = xmlfile.namedItem("TextureSet");
   QDomNodeList generators = rootNode.namedItem("Generators").childNodes();
   for (int i = 0; i < generators.count(); i++) {
      QDomNode currGenerator = generators.at(i);
      if (!getGenerator(currGenerator.toElement().attribute("name"))) {
         qWarning().noquote() << QStringLiteral("Could not find texture generator with name %1.")
                                     .arg(currGenerator.toElement().attribute("name"));
      }
   }
   QMap<int, int> idMappings;
   QDomNodeList nodes = rootNode.namedItem("Nodes").childNodes();
   for (int i = 0; i < nodes.count(); i++) {
      QDomNode currNode = nodes.at(i);
      int nodeId = currNode.toElement().attribute("id").toInt();
      idMappings[nodeId] = nodeId;
      if (getNode(nodeId) != nullptr) {
         idMappings[nodeId] = getNewId();
      }
   }
   QMapIterator<int, int> nodeiterator(idMappings);
   while (nodeiterator.hasNext()) {
      newNode(nodeiterator.next().value());
   }
   for (int i = 0; i < nodes.count(); i++) {
      QDomNode currNode = nodes.at(i);
      int nodeId = currNode.toElement().attribute("id").toInt();
      getNode(idMappings[nodeId])->loadFromXML(currNode, idMappings);
   }
   modified = false;
}

QDomDocument TextureProject::saveAsXML(bool includegenerators) {
   const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
   QDomDocument xmldoc("TextureSet");
   QDomElement rootNode = xmldoc.createElement("TextureSet");
   xmldoc.appendChild(rootNode);

   if (includegenerators) {
      QDomElement xmlGenerators = xmldoc.createElement("Generators");
      rootNode.appendChild(xmlGenerators);
      QMapIterator<QString, TextureGeneratorPtr> generatoriterator(generators);
      while (generatoriterator.hasNext()) {
         TextureGeneratorPtr generator = generatoriterator.next().value();
         QDomElement generatorNode = xmldoc.createElement("generator");
         generatorNode.setAttribute("name", generator->getName());
         xmlGenerators.appendChild(generatorNode);
         QMapIterator<QString, TextureGeneratorSetting> settingsiterator(generator->getSettings());
         while (settingsiterator.hasNext()) {
            settingsiterator.next();
            QDomElement settingnode = xmldoc.createElement("generatorsetting");
            TextureGeneratorSetting value = settingsiterator.value();
            settingnode.setAttribute("id", settingsiterator.key());
            settingnode.setAttribute("type", value.defaultvalue.typeName());
            settingnode.setAttribute("default", value.defaultvalue.toString());
            settingnode.setAttribute("name", value.name);
            settingnode.setAttribute("description", value.description);
            generatorNode.appendChild(settingnode);
         }
      }
   }
   QDomElement xmlNodes = xmldoc.createElement("Nodes");
   rootNode.appendChild(xmlNodes);
   QMapIterator<int, TextureNodePtr> nodeiterator(nodesCopy);
   while (nodeiterator.hasNext()) {
      xmlNodes.appendChild(nodeiterator.next().value()->saveAsXML(xmldoc));
   }
   return xmldoc;
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

TextureGraphSnapshot TextureProject::createTextureGraphSnapshot(QSize renderSize) const {
   const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
   TextureGraphSnapshot snapshot;
   snapshot.size = renderSize;
   snapshot.nodes.reserve(static_cast<std::size_t>(nodesCopy.size()));
   for (const TextureNodePtr& node : nodesCopy) {
      snapshot.nodes.push_back(node->createTextureNodeSnapshot(renderSize));
   }
   return snapshot;
}

void TextureProject::scheduleThumbnailRender() {
   if (automaticThumbnailRendering && renderManager) {
      renderManager->render(createTextureGraphSnapshot(thumbnailSize));
   }
}

void TextureProject::markSaved() { modified = false; }

void TextureProject::publishRenderResult(TextureRenderResult result) {
   if (result.size != thumbnailSize) {
      return;
   }
   const TextureNodePtr node = getNode(result.nodeId);
   if (!node.isNull()) {
      node->publishRenderedImage(result.size, result.revision, result.image);
   }
}

void TextureProject::publishRenderFailure(TextureRenderFailure failure) {
   qWarning().noquote() << QStringLiteral("Texture render failed for node %1 at %2x%3: %4")
                               .arg(failure.nodeId)
                               .arg(failure.size.width())
                               .arg(failure.size.height())
                               .arg(failure.message);
   emit renderFailed(failure.nodeId, failure.size, failure.message);
}

TextureNodePtr TextureProject::getNode(int id) const {
   std::shared_lock lock(nodesMutex);
   return nodes.value(id);
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
      for (auto definition = newDefinitions.cbegin(); definition != newDefinitions.cend();
           ++definition) {
         const TextureGeneratorSetting& newSetting = definition.value();
         QVariant value = newSetting.defaultvalue;
         if (value.typeId() == QMetaType::QStringList) {
            const QStringList choices = value.toStringList();
            value = newSetting.defaultindex >= 0 && newSetting.defaultindex < choices.size()
                        ? QVariant(choices.at(newSetting.defaultindex))
                        : QVariant(QString());
         }
         const auto oldDefinition = oldDefinitions.constFind(definition.key());
         if (oldDefinition != oldDefinitions.cend() && state.settings.contains(definition.key()) &&
             oldDefinition->defaultvalue.typeId() == newSetting.defaultvalue.typeId() &&
             oldDefinition->multiline == newSetting.multiline) {
            const QVariant previous = state.settings.value(definition.key());
            if (newSetting.defaultvalue.typeId() != QMetaType::QStringList ||
                newSetting.defaultvalue.toStringList().contains(previous.toString())) {
               value = previous;
            }
         }
         migratedSettings.insert(definition.key(), value);
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
   TextureNodePtr copyNode = getNode(id);
   if (copyNode.isNull()) {
      return QString();
   }
   QDomDocument copyBuffer = QDomDocument("TextureSet");
   QDomElement rootNode = copyBuffer.createElement("TextureSet");
   copyBuffer.appendChild(rootNode);
   QDomElement xmlNodes = copyBuffer.createElement("Nodes");
   rootNode.appendChild(xmlNodes);
   xmlNodes.appendChild(copyNode->saveAsXML(copyBuffer));
   return copyBuffer.toString(2);
}

int TextureProject::pasteNodes(const QString& xml) {
   QDomDocument copyBuffer;
   copyBuffer.setContent(xml);
   if (copyBuffer.isNull()) {
      return 0;
   }
   QDomNode rootNode = copyBuffer.namedItem("TextureSet");
   if (rootNode.isNull()) {
      return 0;
   }
   QDomNode nodeRoot = rootNode.namedItem("Nodes");
   if (nodeRoot.isNull()) {
      return 0;
   }
   QDomNodeList nodes = nodeRoot.childNodes();
   int pastedNodeCount = 0;
   for (int i = 0; i < nodes.count(); i++) {
      QDomNode currNode = nodes.at(i);
      if (!currNode.isElement() || currNode.nodeName() != QStringLiteral("Node")) {
         continue;
      }
      QDomElement generatornode = currNode.namedItem("generator").toElement();
      TextureNodePtr node =
          newNode(0, getGenerator(!generatornode.isNull() ? generatornode.attribute("name") : ""));
      if (node.isNull()) {
         continue;
      }
      node->loadFromXML(currNode);
      const int nodeCount = getNumNodes();
      node->setPos(
          QPointF(node->getPos().x() + nodeCount * 15, node->getPos().y() + nodeCount * 15));
      ++pastedNodeCount;
   }
   return pastedNodeCount;
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
   emit nodeRemoved(id);
   scheduleThumbnailRender();
}

void TextureProject::notifyImageUpdated(int id) {
   modified = true;
   scheduleThumbnailRender();
   emit imageUpdated(id);
}

void TextureProject::notifyImageAvailable(int id, QSize size) { emit imageAvailable(id, size); }

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
