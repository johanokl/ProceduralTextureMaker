
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTURENODE_H
#define TEXTURENODE_H

#include "generators/texturegenerator.h"
#include "global.h"
#include "textureimage.h"
#include <QDomNode>
#include <QMap>
#include <QPoint>
#include <QSet>
#include <cstdint>
#include <shared_mutex>
class TextureProject;
class TextureNode;

/// @brief Shared-pointer type for `TextureNode` objects.
using TextureNodePtr = QSharedPointer<TextureNode>;

/// @brief Represents a node in the texture graph.
/// @details Stores the node's settings, attributes, and links to its source and receiver nodes.
class TextureNode : public QObject {
   Q_OBJECT
   friend class TextureProject;

public:
   /// @brief Destroys the node and releases its graph connections if necessary.
   ~TextureNode() override;

   /// @brief Disconnects all source and receiver connections.
   /// @note Call before removing the node from its project.
   void release();

   /// @brief Gets the node's stable project ID.
   /// @return The node ID.
   int getId() const { return id; }

   /// @brief Gets the node's public display name.
   /// @return The node name.
   QString getName() const { return name; }

   /// @brief Sets the node's public display name.
   /// @param name The new node name.
   void setName(const QString& name);

   /// @brief Selects the generator registered under the supplied name.
   /// @param name The public generator name.
   /// @return @c true if the generator was applied.
   bool setGenerator(const QString& name);

   /// @brief Sets the texture generator used by this node.
   /// @param gen A shared generator instance, or null to select the empty generator.
   /// @return @c true if the generator was applied.
   /// @details The generator may be external to the project registry. The node retains the shared
   /// generator instance and resets its settings to the generator defaults.
   bool setGenerator(TextureGeneratorPtr gen);

   /// @brief Returns the current texture generator under synchronization.
   /// @return A shared pointer that keeps the generator alive for the caller.
   TextureGeneratorPtr getGenerator() const;

   /// @brief Gets the current generator's public name.
   /// @return The generator name, or an empty string when no generator is set.
   QString getGeneratorName() const;

   /// @brief Gets the number of source slots exposed by the current generator.
   /// @return The number of source slots.
   int getNumSourceSlots() const;

   /// @brief Gets the number of nodes receiving this node's image.
   /// @return The number of receiver nodes.
   int getNumReceivers() const;

   /// @brief Returns an iterator over a synchronized copy of the receiver set.
   /// @return An iterator whose copied receiver set is independent of the node.
   QSetIterator<int> getReceivers() const;

   /// @brief Checks whether a source slot is empty.
   /// @param slot The slot index, or `-1` to check for any available slot.
   /// @return @c true if the requested slot, or at least one slot, is available.
   bool slotAvailable(int slot) const;

   /// @brief Connects a source node to a slot or disconnects the current source.
   /// @param slot The slot index, or `-1` to select the first available slot.
   /// @param sourceId The source node ID, or `0` to disconnect the slot.
   /// @return @c true if the requested connection state was accepted.
   /// @details The change is rejected if the slot is invalid, the source does not exist, or the
   /// proposed edge would introduce a cycle in the graph.
   bool setSourceSlot(int slot, int sourceId);

   /// @brief Checks whether this node participates in a direct or indirect graph cycle.
   /// @return @c true if a cycle is found.
   bool findLoop() const;

   /// @brief Gets the node's position in scene coordinates.
   /// @return The current scene position.
   QPointF getPos() const { return pos; }

   /// @brief Sets the node's position and notifies scene views when it changes.
   /// @param pos The new scene position.
   void setPos(QPointF pos);

   /// @brief Retrieves or renders the image with the requested dimensions.
   /// @param size The requested image dimensions.
   /// @return A thread-safe shared pointer to the current image.
   /// @details Cached images are returned immediately. Missing source images are rendered first.
   /// If the node changes during rendering, obsolete output is discarded and rendering restarts
   /// using the new revision.
   TextureImagePtr getImage(QSize size);

   /// @brief Invalidates this node and all downstream image caches.
   void setUpdated();

   /// @brief Checks whether an image is cached for the requested size.
   /// @param size The dimensions of the cache entry to locate.
   /// @return @c true if the cache contains the requested image.
   bool isTextureInCache(QSize size) const;

   /// @brief Counts the prerequisite node images needed before rendering this node.
   /// @param size The requested image dimensions.
   /// @return The number of prerequisite node images.
   int waitingFor(QSize size) const;

   /// @brief Returns a thread-safe snapshot of the current settings.
   /// @return A copy of all setting keys and values used by the node.
   TextureNodeSettings getSettings() const;

   /// @brief Replaces the node's settings without merging.
   /// @param settings The new setting keys and values for the current generator.
   void setSettings(const TextureNodeSettings& settings);

   /// @brief Returns a thread-safe snapshot of the source-slot mapping.
   /// @return A copy of the source node ID assigned to each slot.
   QMap<int, int> getSources() const;

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
   /// @brief Constructs a node owned by a project.
   /// @param project The project that owns and resolves this node's connections.
   /// @param generator The initial shared generator instance.
   /// @param id The stable project ID.
   TextureNode(TextureProject* project, const TextureGeneratorPtr& generator, int id);

   /// @brief Loads node properties, settings, and connections from XML.
   /// @param xmlnode The serialized node element.
   /// @param idMapping A mapping from serialized IDs to IDs in the destination project.
   void loadFromXML(const QDomNode& xmlnode, const QMap<int, int> idMapping = QMap<int, int>());

   /// @brief Serializes the node's properties, settings, and connections.
   /// @param targetdoc The XML document that will own the returned element.
   /// @return The serialized node element.
   QDomElement saveAsXML(QDomDocument targetdoc);

   /// @brief Searches downstream receiver nodes for a cycle.
   /// @param visited The node IDs on the current traversal path.
   /// @return @c true if the traversal reaches a previously visited node.
   bool findLoop(QList<int> visited) const;

   /// @brief Invalidates cached images and advances the render revision.
   /// @details Call this function while the lock protecting a related state mutation remains held,
   /// so renderers cannot observe new state with an old revision.
   void invalidateImageCache();

   /// @brief Invalidates downstream nodes and notifies observers after local state is consistent.
   /// @details Call this function without holding this node's state or cache locks.
   void propagateImageUpdate();

   /// @brief Disconnects every source slot that references a node ID.
   /// @param id The source node ID to disconnect.
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
   std::uint64_t imageRevision = 0;
   // Set to true after releasing all connections, before delete
   bool deleted;

   // Mutexes to make it thread-safe.
   mutable std::shared_mutex sourceMutex;
   mutable std::shared_mutex receiverMutex;
   mutable std::shared_mutex imageMutex;
   mutable std::shared_mutex settingsMutex;
};

#endif  // TEXTURENODE_H
