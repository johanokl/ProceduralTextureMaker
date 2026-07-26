
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "generators/empty.h"
#include "settingsmanager.h"
#include "textureproject.h"
#include "texturerenderthread.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <mutex>
#include <shared_mutex>

TextureProject::TextureProject() {
   settingsManager = nullptr;
   nodes.clear();
   generators.clear();
   newIdCounter = 0;
   emptygenerator = TextureGeneratorPtr(new EmptyGenerator());
   modified = false;
   thumbnailSize = QSize(250, 250);
   startRenderThread(getThumbnailSize());
}

TextureProject::~TextureProject() {
   while (!renderThreads.isEmpty()) {
      QStringList values = renderThreads.firstKey().split("_");
      if (values.length() > 1) {
         stopRenderThread(QSize(values.at(0).toInt(), values.at(1).toInt()));
      }
   }
   this->clear();
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
   previewSize = settingsManager->getPreviewSize();
   if (settingsManager->getThumbnailSize() != getThumbnailSize()) {
      stopRenderThread(getThumbnailSize());
   }
   this->thumbnailSize = settingsManager->getThumbnailSize();
   startRenderThread(getThumbnailSize());
}

void TextureProject::setName(const QString& newname) {
   name = newname;
   emit nameUpdated(name);
}

void TextureProject::startRenderThread(QSize renderSize, QThread::Priority prio) {
   QString key = QString("%1_%2").arg(renderSize.width()).arg(renderSize.height());
   if (!renderThreads.contains(key)) {
      auto* renderThread = new QThread;
      TextureRenderThread* renderer = new TextureRenderThread(renderSize, nodesSnapshot());
      renderer->moveToThread(renderThread);
      renderThread->start();
      renderThread->setPriority(prio);
      QObject::connect(this, &TextureProject::nodeAdded, renderer, &TextureRenderThread::nodeAdded);
      QObject::connect(this, &TextureProject::nodeRemoved, renderer,
                       &TextureRenderThread::nodeRemoved);
      QObject::connect(this, &TextureProject::imageUpdated, renderer,
                       &TextureRenderThread::imageUpdated);
      QObject::connect(renderThread, &QThread::finished, renderThread, &QThread::deleteLater);
      QObject::connect(renderThread, &QThread::finished, renderer,
                       &TextureRenderThread::deleteLater);
      renderThreads.insert(key, renderer);
      emit imageUpdated(0);
   }
}

void TextureProject::stopRenderThread(QSize renderSize) {
   QString key = QString("%1_%2").arg(renderSize.width()).arg(renderSize.height());
   if (renderThreads.contains(key)) {
      TextureRenderThread* renderThread = renderThreads.take(key);
      QThread* workerThread = renderThread->thread();
      QObject::disconnect(this, &TextureProject::nodeAdded, renderThread,
                          &TextureRenderThread::nodeAdded);
      QObject::disconnect(this, &TextureProject::nodeRemoved, renderThread,
                          &TextureRenderThread::nodeRemoved);
      QObject::disconnect(this, &TextureProject::imageUpdated, renderThread,
                          &TextureRenderThread::imageUpdated);
      renderThread->abort();
      workerThread->quit();
      workerThread->wait();
      const QMap<int, TextureNodePtr> nodesCopy = nodesSnapshot();
      QMapIterator<int, TextureNodePtr> nodeiterator(nodesCopy);
      while (nodeiterator.hasNext()) {
         // Remove all images to prevent memory leaks
         nodeiterator.next().value()->setUpdated();
      }
   }
}

void TextureProject::loadFromXML(const QDomDocument& xmlfile) {
   QDomNode rootNode = xmlfile.namedItem("TextureSet");
   QDomNodeList generators = rootNode.namedItem("Generators").childNodes();
   for (int i = 0; i < generators.count(); i++) {
      QDomNode currGenerator = generators.at(i);
      if (!getGenerator(currGenerator.toElement().attribute("name"))) {
         ERROR_MSG(QString("Could not find texture generator with name %1.")
                       .arg(currGenerator.toElement().attribute("name")));
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
   modified = false;
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

   // TextureNode initializes its generator before the signal connections above exist, so its
   // initial imageUpdated signal cannot mark the project as changed. Node creation is itself a
   // document change and must be recorded explicitly.
   modified = true;
   emit nodeAdded(newNode);
   return newNode;
}

void TextureProject::addGenerator(const TextureGeneratorPtr& gen) {
   if (gen.isNull()) {
      return;
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

TextureGeneratorPtr TextureProject::getGenerator(const QString& name) const {
   if (!generators.contains(name)) {
      qDebug() << QString("No generator with name %1.").arg(name);
   }
   return generators.value(name, TextureGeneratorPtr(nullptr));
}

void TextureProject::copyNode(int id) {
   TextureNodePtr copyNode = getNode(id);
   if (copyNode.isNull()) {
      return;
   }
   QDomDocument copyBuffer = QDomDocument("TextureSet");
   QDomElement rootNode = copyBuffer.createElement("TextureSet");
   copyBuffer.appendChild(rootNode);
   QDomElement xmlNodes = copyBuffer.createElement("Nodes");
   rootNode.appendChild(xmlNodes);
   xmlNodes.appendChild(copyNode->saveAsXML(copyBuffer));
   QApplication::clipboard()->setText(copyBuffer.toString(2));
}

void TextureProject::cutNode(int id) {
   copyNode(id);
   removeNode(id);
}

void TextureProject::pasteNode() {
   QDomDocument copyBuffer;
   copyBuffer.setContent(QApplication::clipboard()->text());
   if (copyBuffer.isNull()) {
      return;
   }
   QDomNode rootNode = copyBuffer.namedItem("TextureSet");
   if (rootNode.isNull()) {
      return;
   }
   QDomNode nodeRoot = rootNode.namedItem("Nodes");
   if (nodeRoot.isNull()) {
      return;
   }
   QDomNodeList nodes = nodeRoot.childNodes();
   for (int i = 0; i < nodes.count(); i++) {
      QDomNode currNode = nodes.at(i);
      QDomElement generatornode = currNode.namedItem("generator").toElement();
      TextureNodePtr node =
          newNode(0, getGenerator(!generatornode.isNull() ? generatornode.attribute("name") : ""));
      node->loadFromXML(currNode);
      const int nodeCount = getNumNodes();
      node->setPos(
          QPointF(node->getPos().x() + nodeCount * 15, node->getPos().y() + nodeCount * 15));
   }
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
}

void TextureProject::notifyImageUpdated(int id) {
   modified = true;
   emit imageUpdated(id);
}

void TextureProject::notifyImageAvailable(int id, QSize size) { emit imageAvailable(id, size); }

void TextureProject::notifyNodesConnected(int sourceId, int receiverId, int slot) {
   modified = true;
   emit nodesConnected(sourceId, receiverId, slot);
}

void TextureProject::notifyNodesDisconnected(int sourceId, int receiverId, int slot) {
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
