/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTUREPROJECT_H
#define TEXTUREPROJECT_H

#include <QMap>
#include <QDomDocument>
#include <QSize>
#include <QReadWriteLock>
#include <QThread>
#include "texturenode.h"

class TextureRenderThread;
class TextureGenerator;
class SettingsManager;

/**
 * @brief The TextureProject class
 *
 * Manages all the texture generators, TextureNode instances and
 * the connections between them.
 */
class TextureProject : public QObject
{
   Q_OBJECT
   friend class TextureNode;

public:
   TextureProject();
   virtual ~TextureProject();
   QDomDocument saveAsXML(bool includegenerators = false);
   void loadFromXML(QDomDocument xmlfile);
   QString getName() { return name; }
   void setName(QString);
   TextureNodePtr getNode(int id) const;
   bool findLoops();
   void removeNode(int id);
   TextureNodePtr newNode(int id = 0, TextureGeneratorPtr generator = NULL);
   void clear();
   bool isModified();
   int getNumNodes();
   TextureGeneratorPtr getGenerator(QString name);
   QMap<QString, TextureGeneratorPtr> getGenerators() { return generators; }
   QSize getThumbnailSize() { return thumbnailSize; }
   QSize getPreviewSize() { return previewSize; }
   void setSettingsManager(SettingsManager* manager);
   SettingsManager* getSettingsManager() { return settingsManager; }

public slots:
   void addGenerator(TextureGeneratorPtr gen);
   void removeGenerator(TextureGeneratorPtr gen);
   void notifyNodesConnected(int sourceId, int receiverId, int slot);
   void notifyNodesDisconnected(int sourceId, int receiverId, int slot);
   void notifyImageUpdated(int id);
   void notifyImageAvailable(int id, QSize size);
   void copyNode(int id);
   void cutNode(int id);
   void pasteNode();
   void settingsUpdated();

signals:
   void nodeAdded(TextureNodePtr);
   void nodeRemoved(int);
   void nodesConnected(int, int, int);
   void nodesDisconnected(int, int, int);
   void imageUpdated(int);
   void imageAvailable(int id, QSize size);
   void nameUpdated(QString);
   void generatorAdded(TextureGeneratorPtr);
   void generatorRemoved(TextureGeneratorPtr);

private:
   TextureGeneratorPtr getEmptyGenerator() { return emptygenerator; }
   void startRenderThread(QSize renderSize, QThread::Priority = QThread::NormalPriority);
   void stopRenderThread(QSize renderSize);
   int getNewId();

   QString name;
   int newIdCounter;
   TextureGeneratorPtr emptygenerator;
   QMap<QString, TextureRenderThread*> renderThreads;
   QMap<int, TextureNodePtr> nodes;
   QMap<QString, TextureGeneratorPtr> generators;
   bool modified;
   mutable QReadWriteLock nodesmutex;

   QSize thumbnailSize;
   QSize previewSize;
   SettingsManager* settingsManager;
};

#endif // TEXTUREPROJECT_H
