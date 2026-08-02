
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturenode.h"
#include "generators/texturegenerator.h"
#include "global.h"
#include "texturerendermanager.h"
#include "textureimage.h"
#include "textureproject.h"
#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QMapIterator>
#include <QMetaType>
#include <QObject>
#include <QPointF>
#include <QSet>
#include <QSetIterator>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtCore/qcontainerfwd.h>
#include <QtCore/qtmetamacros.h>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <utility>

/// @brief Defines an ordering for `QSize` keys used by `QMap`.
bool operator<(const QSize& lhs, const QSize& rhs) {
   if (lhs.height() == rhs.height()) {
      return lhs.width() < rhs.width();
   }
   return lhs.height() < rhs.height();
}

TextureNode::TextureNode(TextureProject* project, const TextureGeneratorPtr& gen, int id) {
   qRegisterMetaType<TextureNodePtr>("TextureNodePtr");
   name = QString("Node %1").arg(id);
   this->project = project;
   this->id = id;
   this->gen = TextureGeneratorPtr(nullptr);
   sources.clear();
   receivers.clear();
   deleted = false;
   setGenerator(gen);
}

TextureNode::~TextureNode() {
   if (!deleted) {
      release();
   }
}

void TextureNode::release() {
   for (const QString& slot : getSourceSlots()) {
      setSourceSlot(slot, 0);
   }
   QSet<int> receiversCopy;
   {
      std::shared_lock lock(receiverMutex);
      receiversCopy = receivers;
   }
   QSetIterator<int> receiveriter(receiversCopy);
   while (receiveriter.hasNext()) {
      TextureNodePtr receiverNode = project->getNode(receiveriter.next());
      if (!receiverNode.isNull()) {
         receiverNode->removeSource(id);
      }
   }
   deleted = true;
}

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
      const TextureGeneratorPtr generator = getGenerator();
      const QString slotId =
          generator->resolveSourceSlot(sources.at(i).toElement().attribute("slot"));
      int sourceId = sources.at(i).toElement().attribute("source").toInt();
      if (idMappings.contains(sourceId)) {
         sourceId = idMappings[sourceId];
      }
      if (!slotId.isNull()) {
         setSourceSlot(slotId, sourceId);
      }
   }
   TextureNodeSettings loadedSettings = getSettings();
   QDomNodeList settingsNodes = xmlnode.namedItem("Settings").childNodes();
   for (int i = 0; i < settingsNodes.count(); i++) {
      QDomElement currNode = settingsNodes.at(i).toElement();
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
      loadedSettings.insert(settingId, settingVariant);
   }
   setSettings(loadedSettings);
}

QDomElement TextureNode::saveAsXML(QDomDocument targetdoc) {
   const TextureNodeSettings settingsCopy = getSettings();
   const QMap<QString, int> sourcesCopy = getSources();
   const TextureGeneratorPtr generator = getGenerator();
   QDomElement retXmlNode = targetdoc.createElement("Node");
   retXmlNode.setAttribute("id", id);
   retXmlNode.setAttribute("name", name);

   QLocale localeXML(QLocale::C);

   QDomElement posnode = targetdoc.createElement("pos");
   posnode.setAttribute("x", localeXML.toString(pos.x()));
   posnode.setAttribute("y", localeXML.toString(pos.y()));
   retXmlNode.appendChild(posnode);

   QDomElement generatornode = targetdoc.createElement("generator");
   generatornode.setAttribute("name", generator->getName());
   retXmlNode.appendChild(generatornode);

   if (!settingsCopy.empty()) {
      QDomElement settingsnode = targetdoc.createElement("Settings");
      retXmlNode.appendChild(settingsnode);
      QMapIterator<QString, QVariant> settingsiterator(settingsCopy);
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
   if (!sourcesCopy.empty()) {
      QDomElement sourcesnode = targetdoc.createElement("Sources");
      retXmlNode.appendChild(sourcesnode);
      for (const QString& slot : generator->getSourceSlots()) {
         const int sourceId = sourcesCopy.value(slot);
         if (sourceId > 0) {
            QDomElement sourcenode = targetdoc.createElement("source");
            sourcenode.setAttribute("slot", slot);
            sourcenode.setAttribute("source", sourceId);
            sourcesnode.appendChild(sourcenode);
         }
      }
   }
   return retXmlNode;
}

void TextureNode::setName(const QString& newname) {
   if (name == newname) {
      return;
   }
   name = newname;
   emit nameUpdated(id);
}

void TextureNode::setSettings(const TextureNodeSettings& settings) {
   {
      std::unique_lock lock(settingsMutex);
      if (this->settings == settings) {
         return;
      }
      this->settings = settings;
      invalidateImageCache();
   }
   emit settingsUpdated(id);
   propagateImageUpdate();
}

TextureNodeSettings TextureNode::getSettings() const {
   std::shared_lock lock(settingsMutex);
   return settings;
}

QMap<QString, int> TextureNode::getSources() const {
   std::shared_lock lock(sourceMutex);
   return sources;
}

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

void TextureNode::removeSource(int id) {
   const QMap<QString, int> sourcesCopy = getSources();
   QMapIterator<QString, int> sourceiter(sourcesCopy);
   while (sourceiter.findNext(id)) {
      setSourceSlot(sourceiter.key(), 0);
   }
}

bool TextureNode::slotAvailable(const QString& slot) const {
   if (!getSourceSlots().contains(slot)) {
      return false;
   }
   std::shared_lock lock(sourceMutex);
   return sources.value(slot) == 0;
}

QString TextureNode::getFirstAvailableSourceSlot() const {
   const QStringList sourceSlots = getSourceSlots();
   std::shared_lock lock(sourceMutex);
   for (const QString& slot : sourceSlots) {
      if (sources.value(slot) == 0) {
         return slot;
      }
   }
   return QString();
}

bool TextureNode::setSourceSlot(const QString& slot, int sourceId) {
   if (!getSourceSlots().contains(slot) || sourceId == id) {
      return false;
   }
   std::unique_lock sourceLock(sourceMutex);
   if (sources.value(slot) == sourceId) {
      return true;
   }

   const int oldSourceId = sources.value(slot);
   bool removedOldReceiver = false;
   TextureNodePtr oldNode;
   // Shall we remove an unused source?
   if (oldSourceId != 0 && sources.keys(oldSourceId).length() <= 1) {
      oldNode = project->getNode(oldSourceId);
      if (!oldNode.isNull()) {
         std::unique_lock receiverLock(oldNode->receiverMutex);
         oldNode->receivers.remove(id);
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
            std::unique_lock receiverLock(oldNode->receiverMutex);
            oldNode->receivers.insert(id);
         }
         return false;
      }
      // Does adding it cause loops?
      {
         std::unique_lock receiverLock(sourceNode->receiverMutex);
         addedReceiver = !sourceNode->receivers.contains(id);
         sourceNode->receivers.insert(id);
      }
      if (project->findLoops()) {
         if (addedReceiver) {
            std::unique_lock receiverLock(sourceNode->receiverMutex);
            sourceNode->receivers.remove(id);
         }
         if (removedOldReceiver) {
            std::unique_lock receiverLock(oldNode->receiverMutex);
            oldNode->receivers.insert(id);
         }
         return false;
      }
   }

   sources[slot] = sourceId;
   invalidateImageCache();
   sourceLock.unlock();

   if (oldSourceId != 0 && oldSourceId != sourceId) {
      emit nodesDisconnected(oldSourceId, id, slot);
   }
   if (sourceId != 0) {
      emit nodesConnected(sourceId, id, slot);
   }
   propagateImageUpdate();
   emit slotsUpdated(id);
   return true;
}

int TextureNode::getNumReceivers() const {
   std::shared_lock lock(receiverMutex);
   return receivers.size();
}

QSetIterator<int> TextureNode::getReceivers() const {
   QSet<int> receiversCopy;
   {
      std::shared_lock lock(receiverMutex);
      receiversCopy = receivers;
   }
   return QSetIterator<int>(receiversCopy);
}

void TextureNode::setUpdated() {
   invalidateImageCache();
   propagateImageUpdate();
}

void TextureNode::invalidateImageCache() {
   {
      std::unique_lock lock(imageMutex);
      ++imageRevision;
      texturecache.clear();
   }
}

void TextureNode::propagateImageUpdate() {
   QSet<int> receiversCopy;
   {
      std::shared_lock lock(receiverMutex);
      receiversCopy = receivers;
   }
   QSetIterator<int> receiveriter(receiversCopy);
   while (receiveriter.hasNext()) {
      TextureNodePtr receiverNode = project->getNode(receiveriter.next());
      if (!receiverNode.isNull()) {
         receiverNode->setUpdated();
      }
   }
   emit imageUpdated(id);
}

TextureImagePtr TextureNode::renderImage(QSize size) {
   for (;;) {
      std::uint64_t renderRevision = 0;
      {
         std::shared_lock lock(imageMutex);
         TextureImagePtr cachedImage = texturecache.value(size);
         if (!cachedImage.isNull()) {
            return cachedImage;
         }
         renderRevision = imageRevision;
      }

      const QMap<QString, int> sourcesCopy = getSources();
      TextureGeneratorPtr generator;
      TextureNodeSettings settingsCopy;
      {
         std::shared_lock lock(settingsMutex);
         generator = gen;
         settingsCopy = settings;
      }

      QMap<QString, TextureImagePtr> sourceImages;
      for (const QString& slot : generator->getSourceSlots()) {
         const int slotSource = sourcesCopy.value(slot);
         if (slotSource != 0) {
            TextureNodePtr sourceNode = project->getNode(slotSource);
            if (!sourceNode.isNull()) {
               sourceImages.insert(slot, sourceNode->renderImage(size));
            }
         }
      }

      QMapIterator<QString, TextureGeneratorSetting> settingsIterator(generator->getSettings());
      while (settingsIterator.hasNext()) {
         settingsIterator.next();
         if (!settingsCopy.contains(settingsIterator.key())) {
            settingsCopy.insert(settingsIterator.key(), settingsIterator.value().defaultvalue);
         }
      }

      TextureImagePtr renderedImage = TextureImage::create(size);
      generator->generate(size, renderedImage->data(), sourceImages, &settingsCopy);

      bool imagePublished = false;
      {
         std::unique_lock lock(imageMutex);
         if (renderRevision != imageRevision) {
            continue;
         }
         TextureImagePtr cachedImage = texturecache.value(size);
         if (!cachedImage.isNull()) {
            return cachedImage;
         }
         texturecache.insert(size, renderedImage);
         imagePublished = true;
      }
      if (imagePublished) {
         emit imageAvailable(id, size);
      }
      return renderedImage;
   }
}

bool TextureNode::setGenerator(TextureGeneratorPtr newgenerator) {
   if (newgenerator.isNull()) {
      newgenerator = project->getEmptyGenerator();
   }
   const TextureGeneratorPtr oldGenerator = getGenerator();
   if (oldGenerator == newgenerator) {
      return true;
   }

   const QStringList oldSlots =
       oldGenerator.isNull() ? QStringList() : oldGenerator->getSourceSlots();
   const QStringList newSlots = newgenerator->getSourceSlots();
   QList<int> oldSourceIds;
   oldSourceIds.reserve(oldSlots.size());
   const QMap<QString, int> oldSources = getSources();
   for (const QString& slot : oldSlots) {
      oldSourceIds.append(oldSources.value(slot));
      if (oldSources.value(slot) != 0) {
         setSourceSlot(slot, 0);
      }
   }

   const TextureGeneratorSettings generatorSettings = newgenerator->getSettings();
   TextureNodeSettings newSettings;
   for (auto it = generatorSettings.constBegin(); it != generatorSettings.constEnd(); ++it) {
      newSettings.insert(it.key(), it.value().defaultvalue);
      if (it.value().defaultvalue.typeId() == QMetaType::QStringList) {
         const QStringList values = it.value().defaultvalue.toStringList();
         if (it.value().defaultindex >= 0 && it.value().defaultindex < values.size()) {
            newSettings.insert(it.key(), values.at(it.value().defaultindex));
         }
      }
   }

   {
      std::unique_lock lock(settingsMutex);
      gen = newgenerator;
      settings = std::move(newSettings);
      invalidateImageCache();
   }
   {
      std::unique_lock lock(sourceMutex);
      sources.clear();
      for (const QString& slot : newSlots) {
         sources.insert(slot, 0);
      }
   }
   const int preservedSlots = qMin(oldSourceIds.size(), newSlots.size());
   for (int i = 0; i < preservedSlots; ++i) {
      if (oldSourceIds.at(i) != 0) {
         setSourceSlot(newSlots.at(i), oldSourceIds.at(i));
      }
   }
   emit settingsUpdated(id);
   emit generatorUpdated(id);
   propagateImageUpdate();
   return true;
}

TextureGeneratorPtr TextureNode::getGenerator() const {
   std::shared_lock lock(settingsMutex);
   return gen;
}

bool TextureNode::setGenerator(const QString& name) {
   return setGenerator(project->getGenerator(name));
}

QString TextureNode::getGeneratorName() const {
   const TextureGeneratorPtr generator = getGenerator();
   return generator.isNull() ? QString() : generator->getName();
}

QStringList TextureNode::getSourceSlots() const {
   const TextureGeneratorPtr generator = getGenerator();
   return generator.isNull() ? QStringList() : generator->getSourceSlots();
}

TextureImagePtr TextureNode::cachedImage(QSize size) const {
   std::shared_lock lock(imageMutex);
   return texturecache.value(size);
}

TextureNodeSnapshot TextureNode::createTextureNodeSnapshot(QSize size) const {
   std::shared_lock settingsLock(settingsMutex);
   std::shared_lock sourceLock(sourceMutex);
   std::shared_lock imageLock(imageMutex);
   return TextureNodeSnapshot{id, imageRevision, gen, settings, sources, texturecache.value(size)};
}

bool TextureNode::publishRenderedImage(QSize size, std::uint64_t revision,
                                       const TextureImagePtr& image) {
   {
      std::unique_lock lock(imageMutex);
      if (image.isNull() || revision != imageRevision || texturecache.contains(size)) {
         return false;
      }
      texturecache.insert(size, image);
   }
   emit imageAvailable(id, size);
   return true;
}

void TextureNode::discardCachedImage(QSize size) {
   std::unique_lock lock(imageMutex);
   texturecache.remove(size);
}

bool TextureNode::findLoop() const {
   QList<int> empty;
   return findLoop(empty);
}

bool TextureNode::findLoop(QList<int> visited) const {
   if (visited.contains(id)) {
      return true;
   }
   visited.push_back(id);
   QSet<int> receiversCopy;
   {
      std::shared_lock lock(receiverMutex);
      receiversCopy = receivers;
   }
   QSetIterator<int> receiveriter(receiversCopy);
   bool hasLoop = false;
   while (receiveriter.hasNext()) {
      const TextureNodePtr receiverNode = project->getNode(receiveriter.next());
      if (!receiverNode.isNull() && receiverNode->findLoop(visited)) {
         hasLoop = true;
         break;
      }
   }
   return hasLoop;
}
