
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

/// @brief Constructor for the TextureProject class.
/// @details Initializes the project with default values and starts a render thread for the
/// thumbnail size.
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

/// @brief Destructor for the TextureProject class.
TextureProject::~TextureProject() {
   while (!renderThreads.isEmpty()) {
      QStringList values = renderThreads.firstKey().split("_");
      if (values.length() > 1) {
         stopRenderThread(QSize(values.at(0).toInt(), values.at(1).toInt()));
      }
   }
   this->clear();
}

/// @brief Sets the settings manager for the project.
/// @param manager
/// Defines the object where the settings are stored.
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

/// @brief Event handler for settings updates
/// @details If the thumbnail size has changed a new thread for the new size is created and the old
/// thread is stopped.
void TextureProject::settingsUpdated() {
   previewSize = settingsManager->getPreviewSize();
   if (settingsManager->getThumbnailSize() != getThumbnailSize()) {
      stopRenderThread(getThumbnailSize());
   }
   this->thumbnailSize = settingsManager->getThumbnailSize();
   startRenderThread(getThumbnailSize());
}

/// @brief Sets the project's public long name.
/// @param newname
void TextureProject::setName(const QString& newname) {
   name = newname;
   emit nameUpdated(name);
}

/// @brief Starts a new thread with a render instance.
/// @param renderSize Image size that will be rendered by the new thread.
/// @param prio Scheduling priority. NormalPriority if not set.
/// @note Only one thread per image size is allowed. See @c stopRenderThread()
void TextureProject::startRenderThread(QSize renderSize, QThread::Priority prio) {
   QString key = QString("%1_%2").arg(renderSize.width()).arg(renderSize.height());
   if (!renderThreads.contains(key)) {
      auto* renderThread = new QThread;
      TextureRenderThread* renderer = new TextureRenderThread(renderSize, nodes);
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

/// @brief Stops a thread started by @c startRenderThread().
/// @param renderSize Image size
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
      nodesmutex.lockForRead();
      QMapIterator<int, TextureNodePtr> nodeiterator(nodes);
      while (nodeiterator.hasNext()) {
         // Remove all images to prevent memory leaks
         nodeiterator.next().value()->setUpdated();
      }
      nodesmutex.unlock();
   }
}

/// @brief Loads a whole project including node connections and settings from an XML document.
/// @param xmlfile XML DOM object.
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

/// @brief Saves the whole project including node connections and settings to an XML document.
/// @return XML document with the whole scene including the nodes.
QDomDocument TextureProject::saveAsXML(bool includegenerators) {
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
   QMapIterator<int, TextureNodePtr> nodeiterator(nodes);
   while (nodeiterator.hasNext()) {
      xmlNodes.appendChild(nodeiterator.next().value()->saveAsXML(xmldoc));
   }
   modified = false;
   return xmldoc;
}

/// @brief Is the project modified since last save or load?
/// @return @c true if the project has been modified since last save or load.
/// @note Used for indicating to the user that a scene has been modified and needs to be saved
/// before closing to prevent changes getting lost.
bool TextureProject::isModified() const { return modified; }

/// @brief Clears the scene graph and removes all nodes.
/// @note This will remove all nodes from the project and reset the ID counter.
void TextureProject::clear() {
   while (nodes.count()) {
      removeNode(nodes.first()->getId());
   }
   newIdCounter = 0;
}

/// @brief Gets the number of nodes in the node graph.
/// @return the number of nodes in the node graph.
int TextureProject::getNumNodes() const { return nodes.count(); }

/// @brief Generates a unique node id without collisions in the node graph.
/// @return a valid node id
int TextureProject::getNewId() {
   newIdCounter++;
   while (getNode(newIdCounter) != nullptr) {
      newIdCounter++;
   }
   return newIdCounter;
}

/// @brief Gets a node from the scene graph based on its id.
/// @param id Node id for requested node
/// @return a reference to the node. TextureNodePtr(NULL) if not found.
TextureNodePtr TextureProject::getNode(int id) const {
   TextureNodePtr retval(nullptr);
   nodesmutex.lockForRead();
   if (nodes.contains(id)) {
      retval = nodes.value(id);
   }
   nodesmutex.unlock();
   return retval;
}

/// @brief Creates a new node and inserts it into the scene graph.
/// @param id Custom id for this node. If 0 a new id is generated.
/// @param generator The node's texture generator, must be defined.
/// @return a pointer to the newly created node.
TextureNodePtr TextureProject::newNode(int id, TextureGeneratorPtr generator) {
   nodesmutex.lockForRead();
   if (nodes.contains(id)) {
      nodesmutex.unlock();
      return nodes.value(id);
   }
   if (id == 0) {
      id = getNewId();
   }
   if (generator.isNull()) {
      generator = emptygenerator;
   }
   nodesmutex.unlock();
   nodesmutex.lockForWrite();
   TextureNodePtr newNode(new TextureNode(this, generator, id));
   nodes.insert(id, newNode);
   nodesmutex.unlock();

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

/// @brief Adds a TextureGenerator to the project's list.
/// @param gen New generator
/// @details No copying is done by this function.
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

/// @brief Removes a TextureGenerator from the project's list.
/// @param gen
/// @note Must be the same reference that was added.
void TextureProject::removeGenerator(const TextureGeneratorPtr& gen) {
   if (!gen.isNull() && generators.key(gen, "NULL") != "NULL") {
      generators.remove(gen->getName());
      emit generatorRemoved(gen);
   }
}

/// @brief Gets a TextureGenerator from the project's list.
/// @param name
/// @return reference to the singleton TextureGenerator object or NULL
/// @details Searches and returns a @c TextureGenerator based on its full name. @c nullptr is
/// returned if it hasn't been added to the project.
TextureGeneratorPtr TextureProject::getGenerator(const QString& name) const {
   if (!generators.contains(name)) {
      qDebug() << QString("No generator with name %1.").arg(name);
   }
   return generators.value(name, TextureGeneratorPtr(nullptr));
}

/// @brief Clones a node into the copy buffer. See @c pasteNode().
/// @param id Node id.
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

/// @brief Clones a node into the copy buffer and removes the original from the scene graph.
/// @param id Node id
void TextureProject::cutNode(int id) {
   copyNode(id);
   removeNode(id);
}

/// @brief Inserts a clone of the node that's in the copy buffer.
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
      node->setPos(QPointF(node->getPos().x() + this->nodes.size() * 15,
                           node->getPos().y() + this->nodes.size() * 15));
   }
}

/// @brief Completely removes a node from the scene graph.
/// @param id Node id
void TextureProject::removeNode(int id) {
   TextureNodePtr remNode = getNode(id);
   if (remNode.isNull()) {
      return;
   }
   remNode->release();
   nodesmutex.lockForWrite();
   nodes.remove(id);
   nodesmutex.unlock();
   // Removing an unconnected node emits no disconnection or image update signals.
   modified = true;
   emit nodeRemoved(id);
}

/// @brief Emits a signal that a node has been modified and the old cached image is invalid.
/// @param id Node id
void TextureProject::notifyImageUpdated(int id) {
   modified = true;
   emit imageUpdated(id);
}

/// @brief Emits a signal indicating that a node's texture cache has a new or updated image.
/// @param id Node id
/// @param size Image size
void TextureProject::notifyImageAvailable(int id, QSize size) { emit imageAvailable(id, size); }

/// @brief Emits a signal indicating that two nodes have been connected.
/// @param sourceId Source node id
/// @param receiverId Receiver node id
/// @param slot Source slot id
/// @details Also sets the project attribute modified since the last saved state to @c true.
void TextureProject::notifyNodesConnected(int sourceId, int receiverId, int slot) {
   modified = true;
   emit nodesConnected(sourceId, receiverId, slot);
}

/// @brief Emits a signal indicating that two nodes have been disconnected.
/// @param sourceId Source node id
/// @param receiverId Receiver node id
/// @param slot Source slot id
/// @details Also sets the project attribute modified since the last saved state to @c true.
void TextureProject::notifyNodesDisconnected(int sourceId, int receiverId, int slot) {
   modified = true;
   emit nodesDisconnected(sourceId, receiverId, slot);
}

/// @brief Finds if a node has itself as a source, directly or indirectly.
/// @return @c true if a loop was found
/// @note As a node having itself as a source, even indirectly, would lead to an infinite loop suchs
/// graphs are not allowed. This function checks all nodes and returns true if one or more has it
/// self a an indirect source.
bool TextureProject::findLoops() const {
   QMapIterator<int, TextureNodePtr> nodeiterator(nodes);
   while (nodeiterator.hasNext()) {
      if (nodeiterator.next().value()->findLoop()) {
         return true;
      }
   }
   return false;
}
