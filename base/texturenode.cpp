
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturenode.h"
#include "textureproject.h"
#include <QColor>
#include <QLocale>
#include <QMetaType>

/// @brief Orders QSize keys for QMap.
bool operator<(const QSize& lhs, const QSize& rhs) {
   if (lhs.height() == rhs.height()) {
      return lhs.width() < rhs.width();
   }
   return lhs.height() < rhs.height();
}

/// @brief Private constructor. To be called from TextureProject only.
/// @param project The parent this node will belong to.
/// @param gen Reference to singleton generator.
/// @param id Node's unchangeable id.
TextureNode::TextureNode(TextureProject* project, const TextureGeneratorPtr& gen, int id) {
   qRegisterMetaType<TextureNodePtr>("TextureNodePtr");
   name = QString("Node %1").arg(id);
   this->project = project;
   this->id = id;
   this->gen = TextureGeneratorPtr(nullptr);
   sources.clear();
   receivers.clear();
   deleted = false;
   for (int i = 0; i < 10; i++) {
      sources.insert(i, 0);
   }
   setGenerator(gen);
}

/// @brief Destructor.
/// @note Removes all references to this node from other nodes before deletion.
TextureNode::~TextureNode() {
   if (!deleted) {
      release();
   }
}

/// @brief Disconnects all source and receiver connections.
/// @note Needs to be called before deletion.
void TextureNode::release() {
   for (int i = 0; i < getNumSourceSlots(); i++) {
      setSourceSlot(i, 0);
   }
   receivermutex.lockForRead();
   QSet<int> receiversCopy = receivers;
   receivermutex.unlock();
   QSetIterator<int> receiveriter(receiversCopy);
   while (receiveriter.hasNext()) {
      TextureNodePtr receiverNode = project->getNode(receiveriter.next());
      if (!receiverNode.isNull()) {
         receiverNode->removeSource(id);
      }
   }
   deleted = true;
}

/// @brief Load scene from XML data.
/// @param xmlnode Serialized data.
/// @param idMappings Mappings between new and old ids so that references are kept.
/// @details Set up the node's properties, including settings and connections, based on the
/// serialized data stored in XML format by the function saveAsXML().
void TextureNode::loadFromXML(const QDomNode& xmlnode, QMap<int, int> idMappings) {
   name = xmlnode.toElement().attribute("name");
   QDomElement pos = xmlnode.namedItem("pos").toElement();
   if (!pos.isNull()) {
      setPos(QPointF(pos.attribute("x").toDouble(), pos.attribute("y").toDouble()));
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
      } else if (settingType == "bool") {
         settingVariant = QVariant((bool)(settingValue == "true" ? true : false));
      } else if (settingType == "QColor") {
         settingVariant = QVariant(QColor(settingValue));
      } else if (settingType == "QString") {
         settingVariant = QVariant(QString(settingValue));
      }
      this->settings.insert(settingId, settingVariant);
   }
   emit settingsUpdated(id);
}

/// @brief Save as XML
/// @param targetdoc The XML document to add this node to.
/// @return A XML document with this node.
/// @details Serializes this node in a XML format that can then be loaded using the function
/// @c loadFromXML. Node settings, properties and connections are included in the serialized data.
QDomElement TextureNode::saveAsXML(QDomDocument targetdoc) {
   QDomElement retXmlNode = targetdoc.createElement("Node");
   retXmlNode.setAttribute("id", id);
   retXmlNode.setAttribute("name", name);

   QLocale localeXML(QLocale::C);

   QDomElement posnode = targetdoc.createElement("pos");
   posnode.setAttribute("x", localeXML.toString(pos.x()));
   posnode.setAttribute("y", localeXML.toString(pos.y()));
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
         settingnode.setAttribute("value", value.toString());
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

/// @brief Sets the node's public name displayed in the view.
/// @param newname
void TextureNode::setName(const QString& newname) { name = newname; }

/// @brief Replaces the node's settings with the new settings object. No merging is done.
/// @param settings Content and keys used depend on the node's TextureGeneratorSettings.
void TextureNode::setSettings(const TextureNodeSettings& settings) {
   settingsmutex.lockForWrite();
   this->settings = settings;
   settingsmutex.unlock();
   emit settingsUpdated(id);
   setUpdated();
}

/// @brief Set the node's position and notify the scene views.
/// @param pos Scene position (relative)
void TextureNode::setPos(QPointF pos) {
   bool posChanged = false;
   if (this->pos != pos) {
      posChanged = true;
   }
   this->pos = pos;
   if (posChanged) {
      emit positionUpdated(id);
   }
}

/// @brief Disconnect the other node from this node's source slots.
/// @param id Node id
void TextureNode::removeSource(int id) {
   QMapIterator<int, int> sourceiter(sources);
   while (sourceiter.findNext(id)) {
      setSourceSlot(sourceiter.key(), 0);
   }
}

/// @brief Check if a source slot is empty.
/// @param slot Slot id, or -1 for any slot.
/// @return @c true if available
bool TextureNode::slotAvailable(int slot) const {
   if (slot >= 0 && slot < gen->getNumSourceSlots() && sources[slot] == 0) {
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

/// @brief Sets a source node to a slot.
/// @param slot The slot number
/// @param sourceId The source node's id. Use -1 for disconnection.
/// @return @c true if the nodes could be connected.
/// @details Sets a source node to a slot. If the slot is -1, the first empty slot is used. If the
/// source node is already connected to this node, nothing is done. Ensures that the new connection
/// won't cause a loop in the graphs. If it passes the test the source slot is set and the node
/// update signal is emitted.
bool TextureNode::setSourceSlot(int slot, int sourceId) {
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
   int oldSourceId = sources[slot];
   bool removedOldReceiver = false;
   TextureNodePtr oldNode;
   // Shall we remove an unused source?
   if (oldSourceId != 0 && sources.keys(oldSourceId).length() <= 1) {
      oldNode = project->getNode(oldSourceId);
      if (!oldNode.isNull()) {
         oldNode->receivermutex.lockForWrite();
         oldNode->receivers.remove(id);
         oldNode->receivermutex.unlock();
         removedOldReceiver = true;
      }
   }
   // Can we add a new source node?
   bool addedReceiver = false;
   if (sourceId != 0) {
      // Does the node exist?
      TextureNodePtr sourceNode = project->getNode(sourceId);
      if (sourceNode.isNull()) {
         if (removedOldReceiver) {
            oldNode->receivermutex.lockForWrite();
            oldNode->receivers.insert(id);
            oldNode->receivermutex.unlock();
         }
         sourcemutex.unlock();
         return false;
      }
      // Does adding it cause loops?
      sourceNode->receivermutex.lockForWrite();
      addedReceiver = !sourceNode->receivers.contains(id);
      sourceNode->receivers.insert(id);
      sourceNode->receivermutex.unlock();
      if (project->findLoops()) {
         if (addedReceiver) {
            sourceNode->receivermutex.lockForWrite();
            sourceNode->receivers.remove(id);
            sourceNode->receivermutex.unlock();
         }
         if (removedOldReceiver) {
            oldNode->receivermutex.lockForWrite();
            oldNode->receivers.insert(id);
            oldNode->receivermutex.unlock();
         }
         sourcemutex.unlock();
         return false;
      }
   }
   // Add it and send signals
   sources[slot] = sourceId;
   if (oldSourceId != 0 && oldSourceId != sourceId) {
      emit nodesDisconnected(oldSourceId, id, slot);
   }
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

/// @brief Get the number of nodes receiving this node's image.
/// @return Number of nodes receiving this node's image.
int TextureNode::getNumReceivers() const {
   receivermutex.lockForRead();
   int count = receivers.size();
   receivermutex.unlock();
   return count;
}

/// @brief Get an iterator over a copy of the receiver set.
/// @return Iterator over a copy of the receiver set.
/// @details Returns an iterator over a copy of the set of nodes that receive this node's image.
QSetIterator<int> TextureNode::getReceivers() const {
   receivermutex.lockForRead();
   QSet<int> receiversCopy = receivers;
   receivermutex.unlock();
   return QSetIterator<int>(receiversCopy);
}

/// @brief Sets this node to updated.
/// @details Sets this node to updated. Clears the texture cache and notifies the receiver nodes and
/// the generator threads that this node's old image no longer is valid.
void TextureNode::setUpdated() {
   imagemutex.lockForWrite();
   texturecache.clear();
   validImage.clear();
   imagemutex.unlock();
   receivermutex.lockForRead();
   QSet<int> receiversCopy = receivers;
   receivermutex.unlock();
   QSetIterator<int> receiveriter(receiversCopy);
   while (receiveriter.hasNext()) {
      TextureNodePtr receiverNode = project->getNode(receiveriter.next());
      if (!receiverNode.isNull()) {
         receiverNode->setUpdated();
      }
   }
   emit imageUpdated(id);
}

/// @brief Retrieves the generated image for this node.
/// @param size The requested image size.
/// @return a thread safe pointer to the new TextureImage
/// @details Retrieves the generated image for this node. If the image for the requested size has
/// already been calculated, and the image or its sources' settings since then haven't since been
/// changed, the image is returned immediately from the cache. If the node has source nodes whose
/// images haven't been calculated those nodes are calculated first. Thus the waiting time for this
/// call can be long. This call is thread safe and contains several mutexes for various node
/// properties.
TextureImagePtr TextureNode::getImage(QSize size) {
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
         TextureNodePtr srcNode = project->getNode(slotSource);
         if (!srcNode.isNull()) {
            sourceImages.insert(i, srcNode->getImage(size));
         }
      }
   }
   // Smart pointer to memory area to store the new image
   retImage = TextureImage::create(size);
   auto* destImage = retImage->getData();
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

/// @brief Sets the texture generator for this node.
/// @param newgenerator The new texture generator to set.
/// @return @c true if successful.
/// @details Sets the texture generator for this node. The texture generator does not need to have
/// been added to the project. The node keeps a reference to the texture generator singleton
/// instance, no data besides the default settings is copied.
bool TextureNode::setGenerator(TextureGeneratorPtr newgenerator) {
   if (newgenerator.isNull()) {
      newgenerator = project->getEmptyGenerator();
   }
   if (gen != newgenerator) {
      if (!gen.isNull() && gen->getNumSourceSlots() > newgenerator->getNumSourceSlots()) {
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
            if (it.value().defaultvalue.typeId() == QMetaType::QStringList) {
               newSettings.insert(
                   it.key(), it.value().defaultvalue.toStringList().at(it.value().defaultindex));
            }
         }
      }
      setSettings(newSettings);
      gen = newgenerator;
      emit generatorUpdated(id);
      setUpdated();
   }
   return true;
}

/// @brief Sets the texture generator for this node by searching for a generator with that name.
/// @param name The generator class's public name.
/// @return @c true if successful.
/// @details Wrapper function for setting the texture generator for this node that takes the name
/// and searches the project for it. The generator needs to have been added to the project before
/// this function is called.
bool TextureNode::setGenerator(const QString& name) {
   return setGenerator(project->getGenerator(name));
}

/// @brief Gets the node's generator's long name.
/// @return the node's generator's long name.
QString TextureNode::getGeneratorName() const { return gen->getName(); }

/// @brief Retrieves the number of sources for this node's generator.
/// @return number [0,∞]
int TextureNode::getNumSourceSlots() const { return gen->getNumSourceSlots(); }

/// @brief Is the node's image for the requested size already in the cache?
/// @param size The size to look for.
/// @return @c true if in cache.
/// @details Checks whether the local cache contains an image with the selected size.
bool TextureNode::isTextureInCache(QSize size) const {
   imagemutex.lockForRead();
   bool retval = false;
   if (texturecache.contains(size)) {
      retval = true;
   }
   imagemutex.unlock();
   return retval;
}

/// @brief Number of nodes that need to be generated before this node's image can be generated.
/// @param size
/// @return number of nodes
/// @details Calculates the number of node images that need to be generated
/// in the selected size before this one has all its source slots
/// marked as ready. If the number is zero then this node image can
/// be generated.
int TextureNode::waitingFor(QSize size) const {
   if (isTextureInCache(size)) {
      return 0;
   }
   sourcemutex.lockForRead();
   int numwaitingFor = 0;
   QListIterator<int> sourceiterator(sources.values());
   while (sourceiterator.hasNext()) {
      int currNode = sourceiterator.next();
      if (currNode != 0) {
         numwaitingFor += project->getNode(currNode)->waitingFor(size);
      }
   }
   sourcemutex.unlock();
   return numwaitingFor;
}

/// @brief Finds if this node has itself as a source, directly or indirectly.
/// @return @c true if the node has itself a source
/// @note As a node having itself as a source, even indirectly,
/// would lead to an infinite loop suchs graphs are not allowed.
bool TextureNode::findLoop() const {
   QList<int> empty;
   return findLoop(empty);
}

/// @brief Finds if this node has itself as a source, directly or indirectly.
/// @param visited Contains the visited nodes
/// @return @c true if the node has itself a source
bool TextureNode::findLoop(QList<int> visited) const {
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
