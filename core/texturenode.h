/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTURENODE_H
#define TEXTURENODE_H

#include <QSet>
#include <QMap>
#include <QPoint>
#include <QDomNode>
#include <QReadWriteLock>
#include "global.h"
#include "textureimage.h"
#include "generators/texturegenerator.h"

class TextureProject;
class TextureNode;

/**
 * @brief TextureNodePtr
 * Thread-safe smart pointer for TextureNode objects.
 */
typedef QSharedPointer<TextureNode> TextureNodePtr;

/**
 * @brief The TextureNode class
 *
 * A node in the graph with its settings, attributes
 * and links to its source and receiver nodes.
 */
class TextureNode : public QObject
{
   Q_OBJECT
   friend class TextureProject;

public:
   virtual ~TextureNode();
   void release();
   int getId() const { return id; }
   QString getName() const { return name; }
   void setName(QString name);
   bool setGenerator(QString name);
   bool setGenerator(TextureGeneratorPtr gen);
   TextureGeneratorPtr getGenerator() const { return gen; }
   QString getGeneratorName() const;
   int getNumSourceSlots() const;
   int getNumReceivers() const { return receivers.size(); }
   QSetIterator<int> getReceivers() const { return QSetIterator<int>(receivers); }
   bool slotAvailable(int slot) const;
   bool setSourceSlot(int slot, int value);
   bool findLoop() const;
   QPointF getPos() const { return pos; }
   void setPos(QPointF pos);
   TextureImagePtr getImage(QSize size);
   void setUpdated();
   bool isTextureInCache(QSize size) const;
   int waitingFor(QSize size) const;
   const TextureNodeSettings getSettings() const { return settings; }
   void setSettings(TextureNodeSettings settings);
   const QMap<int, int> getSources() const { return sources; }

signals:
   void positionUpdated(int id);
   void imageUpdated(int id);
   void slotsUpdated(int id);
   void imageAvailable(int id, QSize size);
   void settingsUpdated(int id);
   void generatorUpdated(int id);
   void nodesConnected(int sourceId, int receiverId, int slot);
   void nodesDisconnected(int sourceId, int receiverId, int slot);

private:
   TextureNode(TextureProject* project, TextureGeneratorPtr generator, int id);
   void loadFromXML(QDomNode xmlnode, const QMap<int, int> idMapping = QMap<int, int>());
   QDomElement saveAsXML(QDomDocument targetdoc);
   bool findLoop(QList<int> visited) const;
   void removeSource(int id);

   int id;
   QString name;
   QPointF pos;

   QMap<int, int> sources;
   QSet<int> receivers;
   TextureNodeSettings settings;

   TextureGeneratorPtr gen;
   TextureProject* project;

   // Contains all the generated images
   QMap<QSize, TextureImagePtr> texturecache;
   // Set to true after releasing all connections, before delete
   bool deleted;
   QMap<QSize, bool> validImage;

   // Mutexes to make it thread-safe.
   mutable QReadWriteLock sourcemutex;
   mutable QReadWriteLock receivermutex;
   mutable QReadWriteLock imagemutex;
   mutable QReadWriteLock settingsmutex;
};

#endif // TEXTURENODE_H
