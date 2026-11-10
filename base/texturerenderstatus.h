// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTURERENDERSTATUS_H
#define TEXTURERENDERSTATUS_H

#include <QString>

/// @brief Display state of one node's thumbnail render.
enum class TextureRenderState {
   /// @brief The current image is ready.
   Ready,
   /// @brief An image is queued or being generated.
   Rendering,
   /// @brief This node's generator failed.
   Failed,
   /// @brief A source node failed, so this node cannot render.
   Blocked,
   /// @brief Rendering is stopped and this image waits for a restart.
   Stopped,
   /// @brief Other work was cancelled after a render failure.
   Cancelled
};

/// @brief Current thumbnail state and a message for its tooltip.
struct TextureRenderStatus {
   /// @brief State shown by graph nodes and preview panels.
   TextureRenderState state = TextureRenderState::Ready;
   /// @brief Failure or cancellation details, or an empty string on success.
   QString message;
};

#endif  // TEXTURERENDERSTATUS_H
