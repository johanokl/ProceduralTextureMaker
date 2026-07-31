// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTURERENDERMANAGER_H
#define TEXTURERENDERMANAGER_H

#include "generators/texturegenerator.h"
#include "global.h"
#include "textureimage.h"
#include <QMap>
#include <QSize>
#include <QString>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/// @brief A copy of the state needed to render one texture node.
struct TextureNodeSnapshot {
   /// @brief ID of the node represented by this snapshot.
   int nodeId = 0;
   /// @brief Node revision captured with the render state.
   std::uint64_t revision = 0;
   /// @brief Generator used to render the node.
   TextureGeneratorPtr generator;
   /// @brief Generator setting values captured for the render.
   TextureNodeSettings settings;
   /// @brief Source node ID assigned to each generator input slot.
   QMap<int, int> sources;
   /// @brief Cached image for the current render size, if available.
   TextureImagePtr cachedImage;
};

/// @brief A copy of the graph state used for one render.
struct TextureGraphSnapshot {
   /// @brief Width and height of the images produced by this graph render.
   QSize size;
   /// @brief Node states used by this graph render.
   std::vector<TextureNodeSnapshot> nodes;
};

/// @brief A rendered texture image with the node state used to produce it.
struct TextureRenderResult {
   /// @brief ID of the rendered node.
   int nodeId = 0;
   /// @brief Node revision used to produce the image.
   std::uint64_t revision = 0;
   /// @brief Width and height of the rendered image.
   QSize size;
   /// @brief Rendered image.
   TextureImagePtr image;
};

/// @brief Details of a texture render error.
struct TextureRenderFailure {
   /// @brief ID of the node that failed, or zero for a graph-level error.
   int nodeId = 0;
   /// @brief Image dimensions being rendered when the error occurred.
   QSize size;
   /// @brief Message describing the error.
   QString message;
};

/// @brief Keeps the newest graph render and runs unblocked nodes on an owned worker pool.
/// @details A new render replaces older queued work. A node becomes runnable after all its source
/// nodes finish, so independent graph branches can render at the same time. Destruction cancels
/// queued work, wakes the workers, and joins them.
class TextureRenderManager final {
public:
   /// @brief Function called when a node image is ready.
   using ResultHandler = std::function<void(TextureRenderResult)>;

   /// @brief Function called when a render error occurs.
   using FailureHandler = std::function<void(TextureRenderFailure)>;

   /// @brief Starts the render manager's bounded worker pool.
   /// @param resultHandler Receives successfully generated images from worker threads.
   /// @param failureHandler Receives render errors.
   TextureRenderManager(ResultHandler resultHandler, FailureHandler failureHandler,
                        std::size_t workerCount = 0);

   /// @brief Cancels queued work and joins all worker threads.
   ~TextureRenderManager();

   /// @brief Disables copying because the render manager owns worker threads.
   TextureRenderManager(const TextureRenderManager&) = delete;

   /// @brief Disables copy assignment because the render manager owns worker threads.
   TextureRenderManager& operator=(const TextureRenderManager&) = delete;

   /// @brief Disables moving because workers refer to this render manager.
   TextureRenderManager(TextureRenderManager&&) = delete;

   /// @brief Disables move assignment because workers refer to this render manager.
   TextureRenderManager& operator=(TextureRenderManager&&) = delete;

   /// @brief Starts a graph render and replaces any older queued render.
   /// @param snapshot The fixed graph state to render.
   void render(TextureGraphSnapshot snapshot);

   /// @brief Cancels queued work and asks active work to stop between nodes.
   void cancel();

private:
   /// @brief Tracks one node and the source nodes that still need to finish.
   struct TextureNodeRenderState {
      /// @brief Render state copied from the node.
      TextureNodeSnapshot snapshot;
      /// @brief Number of unfinished source nodes.
      int remainingDependencies = 0;
      /// @brief IDs of nodes that use this node as a source.
      std::vector<int> receivers;
   };

   /// @brief Tracks the shared state of one graph render.
   struct TextureGraphRenderState {
      /// @brief Number that identifies this graph render.
      std::uint64_t sequence = 0;
      /// @brief Width and height of the images produced by this graph render.
      QSize size;
      /// @brief Per-node render state stored by node ID.
      std::map<int, TextureNodeRenderState> nodes;
      /// @brief Images available to downstream nodes, stored by node ID.
      QMap<int, TextureImagePtr> renderedImages;
      /// @brief Number of nodes that have not finished rendering.
      std::size_t unfinishedNodes = 0;
      /// @brief Whether rendering stopped because one node failed.
      bool failed = false;
   };

   /// @brief Contains a node task that a worker can run.
   struct TextureNodeRenderTask {
      /// @brief Shared graph render state for this task.
      std::shared_ptr<TextureGraphRenderState> renderState;
      /// @brief ID of the node to render.
      int nodeId = 0;
   };

   /// @brief Builds dependency state for a graph render.
   /// @param snapshot The graph snapshot to prepare for rendering.
   /// @return Shared state used by active node render tasks.
   static std::shared_ptr<TextureGraphRenderState> createGraphRenderState(
       TextureGraphSnapshot snapshot);

   /// @brief Waits for runnable tasks and catches exceptions before they leave the worker thread.
   void runWorker();

   /// @brief Renders one node after all its source nodes finish.
   /// @param task The graph render and node ID to process.
   void renderNode(const TextureNodeRenderTask& task);

   /// @brief Stores an available image and queues newly unblocked receiver nodes.
   /// @param task The completed node render task.
   /// @param image The generated or cached image.
   /// @param publish Whether to send the image to the result handler.
   void completeNode(const TextureNodeRenderTask& task, const TextureImagePtr& image, bool publish);

   /// @brief Stops the current graph render and reports its first failure.
   /// @param task The failed node render task.
   /// @param message A message describing the error.
   void failRender(const TextureNodeRenderTask& task, QString message);

   /// @brief Checks whether a render was cancelled or replaced by a newer render.
   /// @param sequence The render sequence to compare with the newest render.
   /// @return @c true if work on the render should stop.
   [[nodiscard]] bool isObsolete(std::uint64_t sequence) const;

   /// @brief Callback used to return completed images.
   ResultHandler resultHandler;
   /// @brief Callback used to report render errors.
   FailureHandler failureHandler;
   /// @brief Protects the task queue and shared render state.
   mutable std::mutex mutex;
   /// @brief Signals that a task can run or shutdown has started.
   std::condition_variable taskAvailable;
   /// @brief Node tasks whose source dependencies have finished.
   std::deque<TextureNodeRenderTask> runnableTasks;
   /// @brief Newest graph render, or null when no render is active.
   std::shared_ptr<TextureGraphRenderState> currentRender;
   /// @brief Sequence number used to reject older renders.
   std::uint64_t latestRenderSequence = 0;
   /// @brief Whether the render manager is shutting down.
   bool stopping = false;
   /// @brief Worker threads owned by the render manager.
   std::vector<std::thread> workers;
};

#endif  // TEXTURERENDERMANAGER_H
