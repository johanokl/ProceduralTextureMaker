
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREPROJECT_H
#define TEXTUREPROJECT_H

#include "base/texturegenerator.h"
#include "texturenode.h"
#include <QDomDocument>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSize>
#include <QString>
#include <memory>
#include <shared_mutex>

class TextureRenderManager;
class ProjectFileService;
class TextureGenerator;
class SettingsManager;
class EditManager;
struct TextureRenderFailure;
struct TextureGraphSnapshot;
struct TextureRenderResult;

/// @brief Manages all texture generators, nodes, and connections between them.
class TextureProject : public QObject {
   Q_OBJECT
   friend class TextureNode;
   friend class ProjectFileService;
   friend class EditManager;

public:
   /// @brief Constructs an empty project and starts thumbnail rendering with default settings.
   explicit TextureProject(bool automaticThumbnailRendering = true);

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
   /// @param id The node ID, or `0` to allocate a new ID.
   /// @param generator The initial generator, or null to use the empty generator.
   /// @return The newly created node, or the existing node if the same ID is already present.
   TextureNodePtr newNode(int id = 0, TextureGeneratorPtr generator = TextureGeneratorPtr(nullptr));

   /// @brief Removes every node and resets automatic ID allocation.
   void clear();

   /// @brief Checks whether the project has changed since its last save or load.
   /// @return @c true if the project contains unsaved changes.
   bool isModified() const;

   /// @brief Gets the number of nodes in the graph.
   /// @return The number of nodes.
   int getNumNodes() const;

   /// @brief Returns all node IDs in ascending order.
   QList<int> getNodeIds() const;

   /// @brief Returns nodes that are not used as a source by another node.
   QList<int> getSinkNodeIds() const;

   /// @brief Gets a registered generator by its public name.
   /// @param name The generator name.
   /// @return A shared generator pointer, or null if no matching generator exists.
   TextureGeneratorPtr getGenerator(const QString& name) const;

   /// Returns the registered generators keyed by name.
   QMap<QString, TextureGeneratorPtr> getGenerators() const { return generators; }

   /// @brief Gets the configured graph thumbnail dimensions.
   /// @return The thumbnail dimensions.
   QSize getThumbnailSize() const { return thumbnailSize; }

   /// @brief Gets the configured preview and export image dimensions.
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

   /// @brief Atomically replaces a registered definition and migrates nodes using it.
   /// @details Compatible settings and connections with unchanged slot names are preserved.
   /// @return True when the registered generator was replaced.
   bool replaceGenerator(const TextureGeneratorPtr& oldGenerator,
                         const TextureGeneratorPtr& newGenerator);

   /// @brief Marks the project modified and forwards a node-connection notification.
   /// @param sourceId The source node ID.
   /// @param receiverId The receiver node ID.
   /// @param slot The receiver slot name.
   void notifyNodesConnected(int sourceId, int receiverId, QString slot);

   /// @brief Marks the project modified and forwards a node-disconnection notification.
   /// @param sourceId The source node ID.
   /// @param receiverId The receiver node ID.
   /// @param slot The receiver slot name.
   void notifyNodesDisconnected(int sourceId, int receiverId, QString slot);

   /// @brief Marks the project modified and forwards an image-invalidation notification.
   /// @param id The updated node ID.
   void notifyImageUpdated(int id);

   /// @brief Forwards a notification that a node image is available in the cache.
   /// @param id The ID of the rendered node.
   /// @param size The rendered image dimensions.
   void notifyImageAvailable(int id, QSize size);

   /// @brief Serializes one node into the application's clipboard XML format.
   /// @param id The node ID to serialize.
   /// @return The XML payload, or an empty string when the node does not exist.
   QString serializeNode(int id);

   /// @brief Pastes texture nodes from an application clipboard XML payload.
   /// @param xml The XML payload to parse.
   /// @return The number of nodes added to the project.
   int pasteNodes(const QString& xml);

   /// @brief Applies updated image sizes from the attached settings manager.
   void settingsUpdated();

signals:
   /// @brief Emitted after a node is added to the project.
   void nodeAdded(TextureNodePtr);

   /// @brief Emitted after a node is removed from the project.
   void nodeRemoved(int);

   /// @brief Emitted after two nodes are connected.
   void nodesConnected(int, int, QString);

   /// @brief Emitted after two nodes are disconnected.
   void nodesDisconnected(int, int, QString);

   /// @brief Emitted when a node image cache becomes outdated.
   void imageUpdated(int);

   /// @brief Emitted when a rendered node image becomes available.
   void imageAvailable(int, QSize);

   /// @brief Reports a background render failure on the project owner thread.
   /// @param id The failed node ID, or `0` for a graph-level failure.
   /// @param size The image dimensions that failed to render.
   /// @param message A message describing the error.
   void renderFailed(int id, QSize size, QString message);

   /// @brief Emitted when the project name changes.
   void nameUpdated(QString);

   /// @brief Emitted after a texture generator is registered.
   void generatorAdded(TextureGeneratorPtr);

   /// @brief Emitted after a texture generator is removed.
   void generatorRemoved(TextureGeneratorPtr);

   /// @brief Emitted when two generators use the same public name.
   void generatorNameCollision(TextureGeneratorPtr, TextureGeneratorPtr);

private:
   /// @brief Gets the fallback generator used by nodes without a configured generator.
   /// @return A shared empty-generator instance.
   TextureGeneratorPtr getEmptyGenerator() const { return emptygenerator; }

   /// @brief Allocates a node ID that does not collide with the current graph.
   /// @return A newly allocated node ID.
   int getNewId();

   /// @brief Returns a thread-safe snapshot of all nodes in the project.
   /// @return A copy whose shared pointers keep the snapshot nodes alive.
   QMap<int, TextureNodePtr> nodesSnapshot() const;

   /// @brief Copies the render state needed for all project nodes.
   /// @param renderSize The width and height of the images to render.
   /// @return A graph snapshot that does not retain live nodes or the project.
   TextureGraphSnapshot createTextureGraphSnapshot(QSize renderSize) const;

   /// @brief Starts a thumbnail render using the latest graph state.
   void scheduleThumbnailRender();

   /// @brief Adds a completed image to the node cache on the project thread.
   /// @param result The completed image and its captured node revision.
   void publishRenderResult(TextureRenderResult result);

   /// @brief Reports a render failure on the project thread.
   /// @param failure The failed node, image dimensions, and error message.
   void publishRenderFailure(TextureRenderFailure failure);

   /// @brief Marks the current project state as successfully persisted.
   void markSaved();

   /// @brief Name shown to the user.
   QString name;
   /// @brief Next value considered when assigning a node ID.
   int newIdCounter;
   /// @brief Fallback generator used by nodes without another generator.
   TextureGeneratorPtr emptygenerator;
   /// @brief Background render manager owned by the project.
   std::unique_ptr<TextureRenderManager> renderManager;
   /// @brief Project nodes stored by ID.
   QMap<int, TextureNodePtr> nodes;
   /// @brief Registered texture generators stored by public name.
   QMap<QString, TextureGeneratorPtr> generators;
   /// @brief Protects the project node map.
   mutable std::shared_mutex nodesMutex;
   /// @brief Width and height of node thumbnail images.
   QSize thumbnailSize;
   /// @brief Width and height of preview and export images.
   QSize previewSize;
   /// @brief Non-owning pointer to the application settings manager.
   SettingsManager* settingsManager;
   /// @brief Whether the project has unsaved changes.
   bool modified;
   /// @brief Whether graph changes automatically schedule thumbnail rendering.
   bool automaticThumbnailRendering;
};

#endif  // TEXTUREPROJECT_H
