
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREPROJECT_H
#define TEXTUREPROJECT_H

#include "texturenode.h"
#include <QDomDocument>
#include <QMap>
#include <QSize>
#include <QThread>
#include <shared_mutex>
class TextureRenderThread;
class TextureGenerator;
class SettingsManager;

/// @brief Manages all texture generators, nodes, and connections between them.
class TextureProject : public QObject {
   Q_OBJECT
   friend class TextureNode;

public:
   /// @brief Constructs an empty project and starts thumbnail rendering with default settings.
   TextureProject();

   /// @brief Stops render workers and releases all project nodes.
   ~TextureProject() override;

   /// @brief Serializes the project graph and settings to XML.
   /// @param includegenerators Whether the document should include generator metadata.
   /// @return An XML document containing the project graph.
   QDomDocument saveAsXML(bool includegenerators = false);

   /// @brief Loads nodes, settings, and connections from an XML document.
   /// @param xmlfile The XML document to load.
   void loadFromXML(const QDomDocument& xmlfile);

   /// @brief Gets the project's public display name.
   /// @return The project name.
   QString getName() const { return name; }

   /// @brief Sets the project's public display name.
   /// @param name The new project name.
   void setName(const QString& name);

   /// @brief Gets a node by ID under project synchronization.
   /// @param id The node ID.
   /// @return A shared node pointer, or null if the ID is not present.
   TextureNodePtr getNode(int id) const;

   /// @brief Checks the graph for direct or indirect cycles.
   /// @return @c true if at least one cycle is found.
   bool findLoops() const;

   /// @brief Disconnects and removes a node from the graph.
   /// @param id The ID of the node to remove.
   void removeNode(int id);

   /// @brief Creates and inserts a node into the graph.
   /// @param id The requested ID, or `0` to allocate a new ID.
   /// @param generator The initial generator, or null to use the empty generator.
   /// @return The newly created node, or the existing node if the requested ID is already present.
   TextureNodePtr newNode(int id = 0, TextureGeneratorPtr generator = TextureGeneratorPtr(nullptr));

   /// @brief Removes every node and resets automatic ID allocation.
   void clear();

   /// @brief Checks whether the project has changed since its last save or load.
   /// @return @c true if the project contains unsaved changes.
   bool isModified() const;

   /// @brief Gets the number of nodes in the graph.
   /// @return The number of nodes.
   int getNumNodes() const;

   /// @brief Gets a registered generator by its public name.
   /// @param name The generator name.
   /// @return A shared generator pointer, or null if no matching generator exists.
   TextureGeneratorPtr getGenerator(const QString& name) const;

   /// @brief Gets a snapshot of all registered generators keyed by public name.
   /// @return A copy of the generator registry.
   QMap<QString, TextureGeneratorPtr> getGenerators() const { return generators; }

   /// @brief Gets the configured graph thumbnail dimensions.
   /// @return The thumbnail dimensions.
   QSize getThumbnailSize() const { return thumbnailSize; }

   /// @brief Gets the configured export image dimensions.
   /// @return The configured preview and export dimensions.
   QSize getPreviewSize() const { return previewSize; }

   /// @brief Connects the project to an application settings manager.
   /// @param manager The settings manager, or null to detach the current manager.
   void setSettingsManager(SettingsManager* manager);

   /// @brief Gets the attached settings manager.
   /// @return A non-owning settings manager pointer, or null if none is attached.
   SettingsManager* getSettingsManager() const { return settingsManager; }

public slots:
   /// @brief Registers a texture generator unless its name is already in use.
   /// @param gen The generator to register.
   void addGenerator(const TextureGeneratorPtr& gen);

   /// @brief Removes a registered texture generator.
   /// @param gen The exact shared generator instance that was previously registered.
   void removeGenerator(const TextureGeneratorPtr& gen);

   /// @brief Marks the project modified and forwards a node-connection notification.
   /// @param sourceId The source node ID.
   /// @param receiverId The receiver node ID.
   /// @param slot The receiver slot index.
   void notifyNodesConnected(int sourceId, int receiverId, int slot);

   /// @brief Marks the project modified and forwards a node-disconnection notification.
   /// @param sourceId The source node ID.
   /// @param receiverId The receiver node ID.
   /// @param slot The receiver slot index.
   void notifyNodesDisconnected(int sourceId, int receiverId, int slot);

   /// @brief Marks the project modified and forwards an image-invalidation notification.
   /// @param id The updated node ID.
   void notifyImageUpdated(int id);

   /// @brief Forwards a notification that a node image is available in the cache.
   /// @param id The ID of the rendered node.
   /// @param size The rendered image dimensions.
   void notifyImageAvailable(int id, QSize size);

   /// @brief Serializes a node and writes it to the application clipboard.
   /// @param id The ID of the node to copy.
   void copyNode(int id);

   /// @brief Copies a node to the clipboard and removes it from the graph.
   /// @param id The ID of the node to cut.
   void cutNode(int id);

   /// @brief Inserts node data from the application clipboard into the graph.
   void pasteNode();

   /// @brief Applies updated image sizes from the attached settings manager.
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
   /// @brief Gets the fallback generator used by nodes without a configured generator.
   /// @return A shared empty-generator instance.
   TextureGeneratorPtr getEmptyGenerator() const { return emptygenerator; }

   /// @brief Starts a render worker for an image size if one is not already active.
   /// @param renderSize The image dimensions handled by the worker.
   /// @param priority The worker thread's scheduling priority.
   void startRenderThread(QSize renderSize, QThread::Priority priority = QThread::NormalPriority);

   /// @brief Stops and removes the render worker for an image size.
   /// @param renderSize The image dimensions identifying the worker.
   void stopRenderThread(QSize renderSize);

   /// @brief Allocates a node ID that does not collide with the current graph.
   /// @return A newly allocated node ID.
   int getNewId();

   /// @brief Returns a thread-safe snapshot of all nodes in the project.
   /// @return A copy whose shared pointers keep the snapshot nodes alive.
   QMap<int, TextureNodePtr> nodesSnapshot() const;

   QString name;
   int newIdCounter;
   TextureGeneratorPtr emptygenerator;
   QMap<QString, TextureRenderThread*> renderThreads;
   QMap<int, TextureNodePtr> nodes;
   QMap<QString, TextureGeneratorPtr> generators;
   mutable std::shared_mutex nodesMutex;

   QSize thumbnailSize;
   QSize previewSize;
   SettingsManager* settingsManager;
   bool modified;
};

#endif  // TEXTUREPROJECT_H
