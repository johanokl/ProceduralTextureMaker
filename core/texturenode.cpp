/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QMapIterator>
#include <QListIterator>
#include <QSetIterator>
#include <QDomDocument>
#include <QVariant>
#include <QColor>
#include "texturenode.h"
#include "generators/texturegenerator.h"
#include "textureproject.h"
#include "textureimage.h"

/**
 * @brief TextureNode::TextureNode
 * @param project The parent this node will belong to.
 * @param gen Reference to singleton generator.
 * @param id Node's unchangeable id.
 *
 * Private constructor. To be called from TextureProject only.
 */
TextureNode::TextureNode(TextureProject* project, TextureGeneratorPtr gen, int id)
{
   qRegisterMetaType<TextureNodePtr>("TextureNodePtr");
   name = QString("Node %1").arg(id);
   this->project = project;
   this->id = id;
   this->gen = gen;
   sources.clear();
   receivers.clear();
   deleted = false;
   for (int i = 0; i < 10; i++) {
      sources.insert(i, 0);
   }
}

/**
 * @brief TextureNode::~TextureNode
 * Removes all references to this node from other nodes before deletion.
 */
TextureNode::~TextureNode()
{
   if (!deleted) {
      release();
   }
}

/**
 * @brief TextureNode::release
 *
 * Disconnects all source and receiver connections.
 * Needs to be called before deletion.
 */
void TextureNode::release()
{
   for (int i = 0; i < getNumSourceSlots(); i++) {
      setSourceSlot(i, 0);
   }
   receivermutex.lockForRead();
   QSetIterator<int> receiveriter(receivers);
   while (receiveriter.hasNext()) {
      project->getNode(receiveriter.next())->removeSource(id);
   }
   receivermutex.unlock();
   deleted = true;
}

/**
 * @brief TextureNode::loadFromXML
 * @param xmlnode Serialized data.
 * @param idMappings Mappings between new and old ids so that references are kept.
 *
 * Set up the node's properties, including settings and connections, based on
 * the serialized data stored in XML format by the function saveAsXML().
 */
void TextureNode::loadFromXML(QDomNode xmlnode, QMap<int, int> idMappings)
{
   name = xmlnode.toElement().attribute("name");
   QDomElement pos = xmlnode.namedItem("pos").toElement();
   if (!pos.isNull()) {
      setPos(QPointF(pos.attribute("x").toDouble(),
                     pos.attribute("y").toDouble()));
   }
   QDomElement generatornode = xmlnode.namedItem("generator").toElement();
   if (!generatornode.isNull()) {
      setGenerator(generatornode.attribute("name"));
   }
   QDomNodeList sources = xmlnode.namedItem("Sources").childNodes();
   for (int i = 0; i < sources.count(); i++) {
      int slotId = sources.at(i).toElement().attribute("slot").toInt();
      int sourceId = sources.at(i).toElement().attribute("source").toInt();
      if (idMappings.contains(sourceId)) {
         sourceId = idMappings[sourceId];
      }
      setSourceSlot(slotId, sourceId);
   }
   QDomNodeList settings = xmlnode.namedItem("Settings").childNodes();
   for (int i = 0; i < settings.count(); i++) {
      QDomElement currNode = settings.at(i).toElement();
      QString settingId = currNode.attribute("id");
      QString settingType = currNode.attribute("type");
      QString settingValue = currNode.attribute("value");
      QVariant settingVariant;
      if (settingType == "int") {
         settingVariant = QVariant(settingValue.toInt());
      } else if (settingType == "double") {
         settingVariant = QVariant(settingValue.toDouble());
      } else if (settingType == "QColor") {
         settingVariant = QVariant(QColor(settingValue));
      } else if (settingType == "QString") {
         settingVariant = QVariant(QString(settingValue));
      }
      this->settings.insert(settingId, settingVariant);
   }
   emit settingsUpdated(id);
}

/**
 * @brief TextureNode::saveAsXML
 * @param targetdoc The XML document to add this node to.
 * @return A XML document with this node.
 *
 * Serializes this node in a XML format that can then be loaded
 * using the function loadFromXML. Node settings, properties and
 * connections are included in the serialized data.
 */
QDomElement TextureNode::saveAsXML(QDomDocument targetdoc)
{
   QDomElement retXmlNode = targetdoc.createElement("Node");
   retXmlNode.setAttribute("id", id);
   retXmlNode.setAttribute("name", name);

   QDomElement posnode = targetdoc.createElement("pos");
   posnode.setAttribute("x", pos.x());
   posnode.setAttribute("y", pos.y());
   retXmlNode.appendChild(posnode);

   QDomElement generatornode = targetdoc.createElement("generator");
   generatornode.setAttribute("name", gen->getName());
   retXmlNode.appendChild(generatornode);

   if (!settings.empty()) {
      QDomElement settingsnode = targetdoc.createElement("Settings");
      retXmlNode.appendChild(settingsnode);
      QMapIterator<QString, QVariant> settingsiterator(settings);
      while (settingsiterator.hasNext()) {
         settingsiterator.next();
         QDomElement settingnode = targetdoc.createElement("setting");
         QString key = settingsiterator.key();
         QVariant value = settingsiterator.value();
         settingnode.setAttribute("id", key);
         settingnode.setAttribute("type", value.typeName());
         if (value.type() == QVariant::Type::Color) {
            settingnode.setAttribute("value", value.toString());
         } else {
            settingnode.setAttribute("value", value.toString());
         }
         settingsnode.appendChild(settingnode);
      }
   }
   if (!sources.empty()) {
      QDomElement sourcesnode = targetdoc.createElement("Sources");
      retXmlNode.appendChild(sourcesnode);
      QMapIterator<int, int> sourcesiterator(sources);
      while (sourcesiterator.hasNext()) {
         sourcesiterator.next();
         if (sourcesiterator.value() > 0) {
            QDomElement sourcenode = targetdoc.createElement("source");
            sourcenode.setAttribute("slot", sourcesiterator.key());
            sourcenode.setAttribute("source", sourcesiterator.value());
            sourcesnode.appendChild(sourcenode);
         }
      }
   }
   return retXmlNode;
}

/**
 * @brief TextureNode::setName
 * @param newname
 * Sets the node's public name displayed in the view.
 */
void TextureNode::setName(QString newname)
{
   name = newname;
}

/**
 * @brief TextureNode::setSettings
 * @param settings Content and keys used depend on the node's TextureGeneratorSettings.
 *
 * Replaces the node's settings with the new settings object. No merging is done.
 */
void TextureNode::setSettings(TextureNodeSettings settings)
{
   settingsmutex.lockForWrite();
   this->settings = settings;
   settingsmutex.unlock();
   emit settingsUpdated(id);
   setUpdated();
}

/**
 * @brief TextureNode::setPos
 * @param pos Scene position (relative)
 *
 * Set the node's position and notify the scene views.
 */
void TextureNode::setPos(QPointF pos)
{
   bool posChanged = false;
   if (this->pos != pos) {
      posChanged = true;
   }
   this->pos = pos;
   if (posChanged) {
      emit positionUpdated(id);
   }
}

/**
 * @brief TextureNode::removeSource
 * @param id Node id
 *
 * Disconnect the other node from this node's source slots.
 */
void TextureNode::removeSource(int id)
{
   QMapIterator<int, int> sourceiter(sources);
   while (sourceiter.findNext(id)) {
      setSourceSlot(sourceiter.key(), 0);
   }
}

/**
 * @brief TextureNode::slotAvailable
 * @param slot Slot id, or -1 for any slot.
 * @return true if available
 *
 * Check if a source slot is empty.
 */
bool TextureNode::slotAvailable(int slot) const
{
   if (slot >= 0 &&
       slot < gen->getNumSourceSlots() &&
       sources[slot] == 0) {
      return true;
   }
   if (slot == -1) {
      for (int i = 0; i < gen->getNumSourceSlots(); i++) {
         if (sources[i] == 0) {
            return true;
         }
      }
   }
   return false;
}

/**
 * @brief TextureNode::setSourceSlot
 * @param slot The slot number
 * @param sourceId The source node's id. Use -1 for disconnection.
 * @return true if the nodes could be connected.
 *
 * Checks if the slot is empty and valid, and the connection won't cause a
 * loop in the graphs. If it passes the test the source slot is set and the
 * node update signal is emitted.
 */
bool TextureNode::setSourceSlot(int slot, int sourceId)
{
   sourcemutex.lockForWrite();
   if ((slot < -1) || slot >= getNumSourceSlots() || sourceId == id) {
      sourcemutex.unlock();
      return false;
   }
   if (sources[slot] == sourceId) {
      sourcemutex.unlock();
      return true;
   }
   if (slot == -1) {
      for (int i = 0; i < gen->getNumSourceSlots(); i++) {
         if (sources[i] == 0) {
            slot = i;
            break;
         }
      }
      if (slot == -1) {
         sourcemutex.unlock();
         return false;
      }
   }
   // Shall we remove an unused source?
   if (sources[slot] != 0 && sources.values().count(sources[slot]) <= 1) {
      TextureNodePtr oldNode = project->getNode(sources[slot]);
      if (!oldNode.isNull()) {
         oldNode->receivers.remove(id);
      }
      emit nodesDisconnected(sources[slot], id, slot);
   }
   // Can we add a new source node?
   if (sourceId != 0) {
      // Does the node exist?
      TextureNodePtr sourceNode = project->getNode(sourceId);
      if (sourceNode.isNull()) {
         sourcemutex.unlock();
         return false;
      }
      // Does adding it cause loops?
      sourceNode->receivers.insert(id);
      if (project->findLoops()) {
         INFO_MSG("Found loop in graph. Rolling back to safe state.");
         if (!(sources.values().contains(sourceId))) {
            sourceNode->receivers.remove(id);
         }
         sourcemutex.unlock();
         return false;
      }
   }
   // Add it and send signals
   sources[slot] = sourceId;
   if (sourceId != 0) {
      emit nodesConnected(sources[slot], id, slot);
   }
   if (slot < gen->getNumSourceSlots()) {
      setUpdated();
   }
   sourcemutex.unlock();
   emit slotsUpdated(id);
   return true;
}

/**
 * @brief TextureNode::setUpdated
 * Setings this node to updated. Clears the texture cache and notifies
 * the reciever nodes and the generator threads that this node's old image
 * no longer is valid.
 */
void TextureNode::setUpdated()
{
   imagemutex.lockForWrite();
   texturecache.clear();
   validImage.clear();
   imagemutex.unlock();
   QSetIterator<int> receiveriter(receivers);
   receivermutex.lockForRead();
   while (receiveriter.hasNext()) {
      project->getNode(receiveriter.next())->setUpdated();
   }
   receivermutex.unlock();
   emit imageUpdated(id);
}

/**
 * @brief TextureNode::getImage
 * @param size The requested image size.
 * @return a thread safe pointer to the new TextureImage
 *
 * Retrieves the generated image for this node. If the image for the requested
 * size has already been calculated, and the image or its sources' settings
 * since then haven't since been changed, the image is returned immediately
 * from the cache.
 * If the node has source nodes whose images haven't been calculated those
 * nodes are calculated first. Thus the waiting time for this call can be long.
 * This call is thread safe and contains several mutexes for various node properties.
 */
TextureImagePtr TextureNode::getImage(QSize size)
{
   // First check if the image is in the texture cache and
   // and can be returned immediately.
   imagemutex.lockForRead();
   TextureImagePtr retImage = texturecache.value(size);
   if (!retImage.isNull()) {
      imagemutex.unlock();
      return retImage;
   }
   imagemutex.unlock();
   imagemutex.lockForWrite();
   // Used to check if the node's settings have been updated by another
   // thread while we were in this function. Prevents storing outdated
   // images in the texture image cache.
   validImage.insert(size, true);
   imagemutex.unlock();

   // All the node's source
   QMap<int, TextureImagePtr> sourceImages;
   for (int i = 0; i < getNumSourceSlots(); i++) {
      sourcemutex.lockForRead();
      int slotSource = sources.value(i);
      sourcemutex.unlock();
      if (slotSource != 0) {
         TextureNodePtr srcNode = project->getNode(sources.value(i));
         if (!srcNode.isNull()) {
            sourceImages.insert(i, srcNode->getImage(size));
         }
      }
   }
   // Smart pointer to memory area to store the new image
   TexturePixel* destImage = new TexturePixel[size.width() * size.height()];
   retImage = TextureImagePtr(new TextureImage(size, destImage));
   // Copy the settings to make it thread safe.
   settingsmutex.lockForRead();
   TextureNodeSettings settingsCopy(settings);
   settingsmutex.unlock();
   QMapIterator<QString, TextureGeneratorSetting> settingsIterator(gen->getSettings());
   while (settingsIterator.hasNext()) {
      settingsIterator.next();
      if (!settingsCopy.contains(settingsIterator.key())) {
         settingsCopy.insert(settingsIterator.key(), settingsIterator.value().defaultvalue);
      }
   }

   // Call the generator singleton
   gen->generate(size, destImage, sourceImages, &settingsCopy);

   imagemutex.lockForRead();
   if (validImage.value(size)) {
      imagemutex.unlock();
      imagemutex.lockForWrite();
      texturecache.insert(size, retImage);
      emit imageAvailable(id, size);
   }
   imagemutex.unlock();
   return retImage;
}

/**
 * @brief TextureNode::setGenerator
 * @param newgenerator
 * @return true if successful.
 *
 * Sets the texture generator for this node. The texture generator does not
 * need to have been added to the project. The node keeps a reference to the
 * texture generator singleton instance, no data besides the default settings
 * is copied.
 */
bool TextureNode::setGenerator(TextureGeneratorPtr newgenerator)
{
   if (newgenerator == NULL) {
      newgenerator = project->getEmptyGenerator();
   }
   if (gen != newgenerator) {
      if (gen->getNumSourceSlots() > newgenerator->getNumSourceSlots()) {
         for (int i = newgenerator->getNumSourceSlots(); i < gen->getNumSourceSlots(); i++) {
            setSourceSlot(i, 0);
         }
      }
      TextureGeneratorSettings genSettings = newgenerator->getSettings();
      TextureNodeSettings newSettings;
      if (genSettings.count() > 0) {
         QMap<QString, TextureGeneratorSetting>::const_iterator it;
         for (it = genSettings.constBegin(); it != genSettings.constEnd(); ++it) {
            newSettings.insert(it.key(), it.value().defaultvalue);
         }
      }
      setSettings(newSettings);
      gen = newgenerator;
      emit generatorUpdated(id);
      setUpdated();
   }
   gen = newgenerator;
   return true;
}

/**
 * @brief TextureNode::setGenerator
 * @param name The generator class's public name.
 * @return true if successful.
 *
 * Wrapper function for setting the texture generator for this node that takes
 * the name and searches the project for it.
 * The generator needs to have been added to the project before this function
 * is called.
 */
bool TextureNode::setGenerator(QString name)
{
   return setGenerator(project->getGenerator(name));
}

/**
 * @brief TextureNode::getGeneratorName
 * @return the node's generator's long name.
 */
QString TextureNode::getGeneratorName() const
{
   return gen->getName();
}

/**
 * @brief TextureNode::getNumSourceSlots
 * @return number [0,∞]
 *
 * Retrieves the number of sources for this node's generator.
 */
int TextureNode::getNumSourceSlots() const
{
   return gen->getNumSourceSlots();
}

/**
 * @brief TextureNode::isTextureInCache
 * @param size The size to look for.
 * @return true if in cache.
 *
 * Checks whether the local cache contains an image
 * with the selected size.
 */
bool TextureNode::isTextureInCache(QSize size) const
{
   imagemutex.lockForRead();
   bool retval = false;
   if (texturecache.contains(size)) {
      retval = true;
   }
   imagemutex.unlock();
   return retval;
}

/**
 * @brief TextureNode::waitingFor
 * @param size
 * @return number of nodes
 *
 * Calculates the number of node images that need to be generated
 * in the selected size before this one has all its source slots
 * marked as ready. If the number is zero then this node image can
 * be generated.
 */
int TextureNode::waitingFor(QSize size) const
{
   if (isTextureInCache(size)) {
      return 0;
   }
   sourcemutex.lockForRead();
   int numwaitingFor = 0;
   QSetIterator<int> sourceiterator(sources.values().toSet());
   while (sourceiterator.hasNext()) {
      int currNode = sourceiterator.next();
      if (currNode != 0) {
         numwaitingFor += project->getNode(currNode)->waitingFor(size);
      }
   }
   sourcemutex.unlock();
   return numwaitingFor;
}

/**
 * @brief TextureNode::findLoop
 * @return true if the node has itself a source
 *
 * As a node having itself as a source, even indirectly,
 * would lead to an infinite loop suchs graphs are not allowed.
 */
bool TextureNode::findLoop() const
{
   QList<int> empty;
   return findLoop(empty);
}

/**
 * @brief TextureNode::findLoop
 * @param visited Contains the visited nodes
 * @return true if the node has itself a source
 */
bool TextureNode::findLoop(QList<int> visited) const
{
   if (visited.contains(id)) {
      return true;
   }
   receivermutex.lockForRead();
   visited.push_back(id);
   QSetIterator<int> receiveriter(receivers);
   bool hasLoop = false;
   while (receiveriter.hasNext()) {
      if (project->getNode(receiveriter.next())->findLoop(visited)) {
         hasLoop = true;
         break;
      }
   }
   receivermutex.unlock();
   return hasLoop;
}
