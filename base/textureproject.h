/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTUREPROJECT_H
#define TEXTUREPROJECT_H

#include "texturenode.h"
#include <QDomDocument>
#include <QMap>
#include <QReadWriteLock>
#include <QSize>
#include <QThread>

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
   ~TextureProject() override;
   QDomDocument saveAsXML(bool includegenerators = false);
   void loadFromXML(const QDomDocument& xmlfile);
   QString getName() const { return name; }
   void setName(const QString&);
   TextureNodePtr getNode(int id) const;
   bool findLoops() const;
   void removeNode(int id);
   TextureNodePtr newNode(int id = 0, TextureGeneratorPtr generator = TextureGeneratorPtr(nullptr));
   void clear();
   bool isModified() const;
   int getNumNodes() const;
   TextureGeneratorPtr getGenerator(const QString& name) const;
   QMap<QString, TextureGeneratorPtr> getGenerators() const { return generators; }
   QSize getThumbnailSize() const { return thumbnailSize; }
   QSize getPreviewSize() const { return previewSize; }
   void setSettingsManager(SettingsManager* manager);
   SettingsManager* getSettingsManager() const { return settingsManager; }

public slots:
   void addGenerator(const TextureGeneratorPtr& gen);
   void removeGenerator(const TextureGeneratorPtr& gen);
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
   void imageAvailable(int, QSize);
   void nameUpdated(QString);
   void generatorAdded(TextureGeneratorPtr);
   void generatorRemoved(TextureGeneratorPtr);
   void generatorNameCollision(TextureGeneratorPtr, TextureGeneratorPtr);

private:
   TextureGeneratorPtr getEmptyGenerator() const { return emptygenerator; }
   void startRenderThread(QSize renderSize, QThread::Priority = QThread::NormalPriority);
   void stopRenderThread(QSize renderSize);
   int getNewId();

   QString name;
   int newIdCounter;
   TextureGeneratorPtr emptygenerator;
   QMap<QString, TextureRenderThread*> renderThreads;
   QMap<int, TextureNodePtr> nodes;
   QMap<QString, TextureGeneratorPtr> generators;
   mutable QReadWriteLock nodesmutex;

   QSize thumbnailSize;
   QSize previewSize;
   SettingsManager* settingsManager;
   bool modified;
};

#endif // TEXTUREPROJECT_H
