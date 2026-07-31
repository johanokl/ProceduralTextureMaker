// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef CLIPBOARDOPERATIONS_H
#define CLIPBOARDOPERATIONS_H

class TextureProject;

/// @brief Serializes a project node and places it on the application clipboard.
/// @param project Project containing the node.
/// @param id Identifier of the node to copy.
/// @return True when the node was serialized and stored on the clipboard.
bool copyNodeToClipboard(TextureProject& project, int id);

/// @brief Copies a project node to the clipboard and removes it from the project.
/// @param project Project containing the node.
/// @param id Identifier of the node to cut.
/// @return True when the node was copied and removed.
bool cutNodeToClipboard(TextureProject& project, int id);

/// @brief Adds nodes serialized on the application clipboard to a project.
/// @param project Project that receives the pasted nodes.
/// @return Number of nodes added to the project.
int pasteNodesFromClipboard(TextureProject& project);

#endif  // CLIPBOARDOPERATIONS_H
