
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTURENODE_H
#define TEXTURENODE_H

#include "generators/texturegenerator.h"
#include "global.h"
#include "textureimage.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointF>
#include <QSet>
#include <QSetIterator>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <cstdint>
#include <shared_mutex>

class TextureProject;
class TextureNode;
struct TextureNodeSnapshot;

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

   /// @brief Gets the ordered source-slot names exposed by the current generator.
   /// @return The source-slot names.
   QStringList getSourceSlots() const;

   /// @brief Gets the number of nodes receiving this node's image.
   /// @return The number of receiver nodes.
   int getNumReceivers() const;

   /// @brief Returns an iterator over a synchronized copy of the receiver set.
   /// @return An iterator whose copied receiver set is independent of the node.
   QSetIterator<int> getReceivers() const;

   /// @brief Checks whether a source slot is empty.
   /// @param slot The canonical slot name.
   /// @return @c true if the given slot is available.
   bool slotAvailable(const QString& slot) const;

   /// @brief Returns the first empty source slot in generator-declared order.
   /// @return The canonical slot name, or a null string if every slot is occupied.
   QString getFirstAvailableSourceSlot() const;

   /// @brief Connects a source node to a slot or disconnects the current source.
   /// @param slot The canonical slot name.
   /// @param sourceId The source node ID, or `0` to disconnect the slot.
   /// @return @c true if the connection change was accepted.
   /// @details The change is rejected if the slot is invalid, the source does not exist, or the
   /// proposed edge would introduce a cycle in the graph.
   bool setSourceSlot(const QString& slot, int sourceId);

   /// @brief Checks whether this node participates in a direct or indirect graph cycle.
   /// @return @c true if a cycle is found.
   bool findLoop() const;

   /// @brief Gets the node's position in scene coordinates.
   /// @return The current scene position.
   QPointF getPos() const { return pos; }

   /// @brief Sets the node's position and notifies scene views when it changes.
   /// @param pos The new scene position.
   void setPos(QPointF pos);

   /// @brief Synchronously retrieves or renders an image at the given size.
   /// @param size The width and height of the image to render.
   /// @return A thread-safe shared pointer to the current image.
   /// @details Cached images are returned immediately. Missing source images are rendered first.
   /// If the node changes during rendering, obsolete output is discarded and rendering restarts
   /// using the new revision.
   /// @note This potentially expensive operation is intended for explicit synchronous workflows,
   /// such as image export. Interactive previews use the background render manager.
   [[nodiscard]] TextureImagePtr renderImage(QSize size);

   /// @brief Returns the cached image for the given size without rendering.
   /// @param size The width and height of the cached image.
   /// @return The cached image, or a null pointer if no image is available.
   [[nodiscard]] TextureImagePtr cachedImage(QSize size) const;

   /// @brief Invalidates this node and all downstream image caches.
   void setUpdated();

   /// @brief Returns a thread-safe snapshot of the current settings.
   /// @return A copy of all setting keys and values used by the node.
   TextureNodeSettings getSettings() const;

   /// @brief Replaces the node's settings without merging.
   /// @param settings The new setting keys and values for the current generator.
   void setSettings(const TextureNodeSettings& settings);

   /// @brief Returns a thread-safe snapshot of the source-slot mapping.
   /// @return A copy of the source node ID assigned to each slot.
   QMap<QString, int> getSources() const;

signals:
   /// @brief Emitted when the node display name changes.
   void nameUpdated(int id);

   /// @brief Emitted when the node position changes.
   void positionUpdated(int id);

   /// @brief Emitted when the node image cache becomes outdated.
   void imageUpdated(int id);

   /// @brief Emitted when the available source slots change.
   void slotsUpdated(int id);

   /// @brief Emitted when a rendered image is added to the cache.
   void imageAvailable(int id, QSize size);

   /// @brief Emitted when the generator settings change.
   void settingsUpdated(int id);

   /// @brief Emitted when the node generator changes.
   void generatorUpdated(int id);

   /// @brief Emitted after a source is connected to this node.
   void nodesConnected(int sourceId, int receiverId, QString slot);

   /// @brief Emitted after a source is disconnected from this node.
   void nodesDisconnected(int sourceId, int receiverId, QString slot);

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

   /// @brief Copies the node state needed for background rendering.
   /// @param size The width and height of the image to render.
   /// @return A synchronized copy of the node's render state and any matching cached image.
   TextureNodeSnapshot createTextureNodeSnapshot(QSize size) const;

   /// @brief Publishes a rendered image if its captured revision is still current.
   /// @param size The rendered image dimensions.
   /// @param revision The node revision captured before rendering.
   /// @param image The completed image.
   /// @return @c true if the image was added to the cache.
   bool publishRenderedImage(QSize size, std::uint64_t revision, const TextureImagePtr& image);

   /// @brief Removes the cached image for the specified dimensions.
   /// @param size The dimensions to remove from the cache.
   void discardCachedImage(QSize size);

   /// @brief Stable ID assigned by the project.
   int id;
   /// @brief Name shown to the user.
   QString name;
   /// @brief Position in graph scene coordinates.
   QPointF pos;
   /// @brief Source node ID assigned to each generator input slot.
   QMap<QString, int> sources;
   /// @brief IDs of nodes that use this node as a source.
   QSet<int> receivers;
   /// @brief Current generator setting values.
   TextureNodeSettings settings;
   /// @brief Generator used to produce this node's image.
   TextureGeneratorPtr gen;
   /// @brief Project that owns this node.
   TextureProject* project;
   /// @brief Generated images stored by image dimensions.
   QMap<QSize, TextureImagePtr> texturecache;
   /// @brief Version increased whenever the rendered output becomes outdated.
   std::uint64_t imageRevision = 0;
   /// @brief Whether all graph connections have been released before deletion.
   bool deleted;
   /// @brief Protects the source slot map.
   mutable std::shared_mutex sourceMutex;
   /// @brief Protects the receiver set.
   mutable std::shared_mutex receiverMutex;
   /// @brief Protects cached images and the image revision.
   mutable std::shared_mutex imageMutex;
   /// @brief Protects the generator and its setting values.
   mutable std::shared_mutex settingsMutex;
};

#endif  // TEXTURENODE_H
