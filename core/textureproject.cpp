/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QApplication>
#include <QClipboard>
#include "textureproject.h"
#include "settingsmanager.h"
#include "generators/empty.h"
#include "texturerenderthread.h"

/**
 * @brief TextureProject::TextureProject
 */
TextureProject::TextureProject()
{
   settingsManager = NULL;
   nodes.clear();
   generators.clear();
   newIdCounter = 0;
   emptygenerator = TextureGeneratorPtr(new EmptyGenerator());
   modified = false;
   thumbnailSize = QSize(250, 250);
   startRenderThread(getThumbnailSize());
}

/**
 * @brief TextureProject::~TextureProject
 */
TextureProject::~TextureProject()
{
   this->clear();
   while (!renderThreads.isEmpty()) {
      QStringList values = renderThreads.firstKey().split("_");
      if (values.length() > 1) {
         stopRenderThread(QSize(values.at(0).toInt(), values.at(1).toInt()));
      }
   }
}

/**
 * @brief TextureProject::setSettingsManager
 * @param manager
 * Defines the object where the settings are stored.
 */
void TextureProject::setSettingsManager(SettingsManager* manager)
{
   if (settingsManager) {
      QObject::disconnect(manager, SIGNAL(settingsUpdated(void)), this, SLOT(settingsUpdated(void)));
   }
   settingsManager = manager;
   settingsUpdated();
   QObject::connect(manager, SIGNAL(settingsUpdated(void)), this, SLOT(settingsUpdated(void)));
}

/**
 * @brief TextureProject::settingsUpdated
 *
 * Called when the settings manager's updated.
 * If the thumbnail size has changed a new thread for the
 * new size is created and the old thread is stopped..
 */
void TextureProject::settingsUpdated()
{
   previewSize = settingsManager->getPreviewSize();
   if (settingsManager->getThumbnailSize() != getThumbnailSize()) {
      stopRenderThread(getThumbnailSize());
   }
   this->thumbnailSize = settingsManager->getThumbnailSize();
   startRenderThread(getThumbnailSize());
}

/**
 * @brief TextureProject::setName
 * @param newname
 *
 * Sets the project's public long name.
 */
void TextureProject::setName(QString newname)
{
   name = newname;
   emit nameUpdated(name);
}

/**
 * @brief TextureProject::startRenderThread
 * @param renderSize Image size that will be rendered by the new thread.
 * @param prio Scheduling priority. NormalPriority if not set.
 *
 * Starts a new thread with a render instance.
 * Only one thread per image size is allowed.
 * See stopRenderThread()
 */
void TextureProject::startRenderThread(QSize renderSize, QThread::Priority prio)
{
   QString key = QString("%1_%2").arg(renderSize.width()).arg(renderSize.height());
   if (!renderThreads.contains(key)) {
      QThread* renderThread = new QThread;
      TextureRenderThread* renderer = new TextureRenderThread(renderSize, nodes);
      renderer->moveToThread(renderThread);
      renderThread->start();
      renderThread->setPriority(prio);
      QObject::connect(this, SIGNAL(nodeAdded(TextureNodePtr)), renderer, SLOT(nodeAdded(TextureNodePtr)));
      QObject::connect(this, SIGNAL(nodeRemoved(int)), renderer, SLOT(nodeRemoved(int)));
      QObject::connect(this, SIGNAL(imageUpdated(int)), renderer, SLOT(imageUpdated()));
      QObject::connect(renderThread, SIGNAL (finished()), renderThread, SLOT (deleteLater()));
      QObject::connect(renderThread, SIGNAL (finished()), renderer, SLOT (deleteLater()));
      renderThreads.insert(key, renderer);
      emit imageUpdated(0);
   }
}

/**
 * @brief TextureProject::stopRenderThread
 * @param renderSize Image size
 *
 * Stops a thread started by startRenderThread().
 */
void TextureProject::stopRenderThread(QSize renderSize)
{
   QString key = QString("%1_%2").arg(renderSize.width()).arg(renderSize.height());
   if (renderThreads.contains(key)) {
      TextureRenderThread* renderThread = renderThreads.take(key);
      QObject::disconnect(this, SIGNAL(nodeAdded(TextureNodePtr)), renderThread, SLOT(nodeAdded(TextureNodePtr)));
      QObject::disconnect(this, SIGNAL(nodeRemoved(int)), renderThread, SLOT(nodeRemoved(int)));
      QObject::disconnect(this, SIGNAL(imageUpdated(int)), renderThread, SLOT(imageUpdated()));
      renderThread->abort();
      renderThread->thread()->quit();
      nodesmutex.lockForRead();
      QMapIterator<int, TextureNodePtr> nodeiterator(nodes);
      while (nodeiterator.hasNext()) {
         // Remove all images to prevent memory leaks
         nodeiterator.next().value()->setUpdated();
      }
      nodesmutex.unlock();
   }
}

/**
 * @brief TextureProject::loadFromXML
 * @param xmlfile XML DOM object.
 *
 * Loads a whole project including node connections and settings from an XML document.
 */
void TextureProject::loadFromXML(QDomDocument xmlfile)
{
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
      if (getNode(nodeId) != NULL) {
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
}

/**
 * @brief TextureProject::saveAsXML
 * @return XML document with the whole scene including the nodes.
 */
QDomDocument TextureProject::saveAsXML(bool includegenerators)
{
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

/**
 * @brief TextureProject::isModified
 * @return true if the project has been modified since last save or load.
 *
 * Used for indicating to the user that a scene has been modified and needs to be saved
 * before closing to prevent changes getting lost.
 */
bool TextureProject::isModified()
{
   return modified;
}

/**
 * @brief TextureProject::clear
 * Clears the scene graph and removes all nodes.
 */
void TextureProject::clear()
{
   while (nodes.count()) {
      removeNode(nodes.first()->getId());
   }
   newIdCounter = 0;
}

/**
 * @brief TextureProject::getNumNodes
 * @return the number of nodes in the node graph.
 */
int TextureProject::getNumNodes()
{
   return nodes.count();
}

/**
 * @brief TextureProject::getNewId
 * @return a valid node id
 * Generates a unique node id without collisions in the node graph.
 */
int TextureProject::getNewId()
{
   newIdCounter++;
   while (getNode(newIdCounter) != NULL) {
      newIdCounter++;
   }
   return newIdCounter;
}

/**
 * @brief TextureProject::getNode
 * @param id Node id for requested node
 * @return a reference to the node. TextureNodePtr(NULL) if not found.
 */
TextureNodePtr TextureProject::getNode(int id) const
{
   TextureNodePtr retval(NULL);
   nodesmutex.lockForRead();
   if (nodes.contains(id)) {
      retval = nodes.value(id);
   }
   nodesmutex.unlock();
   return retval;
}

/**
 * @brief TextureProject::newNode
 * @param id Custom id for this node. If 0 a new id is generated.
 * @param generator The node's texture generator, must be defined.
 * @return Creates a new node and inserts it into the scene graph.
 */
TextureNodePtr TextureProject::newNode(int id, TextureGeneratorPtr generator)
{
   nodesmutex.lockForRead();
   if (nodes.contains(id)) {
      nodesmutex.unlock();
      return nodes.value(id);
   }
   if (id == 0) {
      id = getNewId();
   }
   if (generator == NULL) {
      generator = emptygenerator;
   }
   nodesmutex.unlock();
   nodesmutex.lockForWrite();
   TextureNodePtr newNode(new TextureNode(this, generator, id));
   nodes.insert(id, newNode);
   nodesmutex.unlock();

   QObject::connect(newNode.data(), SIGNAL(nodesConnected(int, int, int)), this, SLOT(notifyNodesConnected(int, int, int)));
   QObject::connect(newNode.data(), SIGNAL(nodesDisconnected(int, int, int)), this, SLOT(notifyNodesDisconnected(int, int, int)));
   QObject::connect(newNode.data(), SIGNAL(imageUpdated(int)), this, SLOT(notifyImageUpdated(int)));
   QObject::connect(newNode.data(), SIGNAL(imageAvailable(int, QSize)), this, SLOT(notifyImageAvailable(int, QSize)));

   emit nodeAdded(newNode);
   newNode->setUpdated();
   return newNode;
}

/**
 * @brief TextureProject::addGenerator
 * @param gen New generator
 *
 * Adds a TextureGenerator to the project's list.
 * No copying is done by this function.
 */
void TextureProject::addGenerator(TextureGeneratorPtr gen)
{
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

/**
 * @brief TextureProject::removeGenerator
 * @param gen
 *
 * Removes a TextureGenerator from the project's list.
 * Must be the same reference that was added.
 */
void TextureProject::removeGenerator(TextureGeneratorPtr gen)
{
   if (!gen.isNull() && generators.values().contains(gen)) {
      generators.remove(gen->getName());
      emit generatorRemoved(gen);
   }
}

/**
 * @brief TextureProject::getGenerator
 * @param name
 * @return reference to the singleton TextureGenerator object or NULL
 *
 * Searches and returns a TextureGenerator based on its full name.
 * NULL is returned if it hasn't been added to the project.
 */
TextureGeneratorPtr TextureProject::getGenerator(QString name)
{
   if (name == "Blur") {
      name = "Box blur";
   }
   return generators.value(name, NULL);
}

/**
 * @brief TextureProject::copyNode
 * @param id Node id.
 *
 * Clones a node into the copy buffer. See pasteNode().
 */
void TextureProject::copyNode(int id)
{
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

/**
 * @brief TextureProject::cutNode
 * @param id Node id
 *
 * Clones a node into the copy buffer and removes
 * the original from the scene graph.
 */
void TextureProject::cutNode(int id)
{
   copyNode(id);
   removeNode(id);
}

/**
 * @brief TextureProject::pasteNode
 * Inserts a clone of the node that's in the copy buffer.
 */
void TextureProject::pasteNode()
{
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
      TextureNodePtr node = newNode();
      node->loadFromXML(nodes.at(i));
      node->setPos(QPointF(node->getPos().x() + this->nodes.size() * 15,
                           node->getPos().y() + this->nodes.size() * 15));
   }

}

/**
 * @brief TextureProject::removeNode
 * @param id Node id
 * Completely removes a node from the scene graph.
 */
void TextureProject::removeNode(int id)
{
   TextureNodePtr remNode = getNode(id);
   if (remNode.isNull()) {
      return;
   }
   remNode->release();
   nodesmutex.lockForWrite();
   nodes.remove(id);
   nodesmutex.unlock();
   emit nodeRemoved(id);
}

/**
 * @brief TextureProject::notifyImageUpdated
 * @param id Node id
 *
 * Emits a signal indicating that a node has been modified and the
 * old cached image isn't valid.
 */
void TextureProject::notifyImageUpdated(int id)
{
   modified = true;
   emit imageUpdated(id);
}

/**
 * @brief TextureProject::notifyImageAvailable
 * @param id Node id
 * @param size Image size
 *
 * Emits a signal indicating that a node's texture cache
 * has a new or updated image.
 */
void TextureProject::notifyImageAvailable(int id, QSize size)
{
   emit imageAvailable(id, size);
}

/**
 * @brief TextureProject::notifyNodesConnected
 * @param sourceId Source node id
 * @param receiverId Receiver node id
 * @param slot Source slot id
 *
 * Emits a signal indicating that two nodes have been disconnected.
 * Also sets the project attribute modified since the last saved state to true.
 */
void TextureProject::notifyNodesConnected(int sourceId, int receiverId, int slot)
{
   modified = true;
   emit nodesConnected(sourceId, receiverId, slot);
}

/**
 * @brief TextureProject::notifyNodesDisconnected
 * @param sourceId Source node id
 * @param receiverId Receiver node id
 * @param slot Source slot id
 *
 * Emits a signal indicating that two nodes have been connected.
 * Also sets the project attribute modified since the last saved state to true.
 */
void TextureProject::notifyNodesDisconnected(int sourceId, int receiverId, int slot)
{
   modified = true;
   emit nodesDisconnected(sourceId, receiverId, slot);
}

/**
 * @brief TextureProject::findLoops
 * @return true if a loop was found
 *
 * As a node having itself as a source, even indirectly,
 * would lead to an infinite loop suchs graphs are not allowed.
 * This function checks all nodes and returns true if one or more
 * has it self a an indirect source.
 */
bool TextureProject::findLoops()
{
   QMapIterator<int, TextureNodePtr> nodeiterator(nodes);
   while (nodeiterator.hasNext()) {
      if (nodeiterator.next().value()->findLoop()) {
         return true;
      }
   }
   return false;
}
