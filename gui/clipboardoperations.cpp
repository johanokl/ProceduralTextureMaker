// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "clipboardoperations.h"
#include "base/textureproject.h"
#include <QApplication>
#include <QClipboard>
#include <QString>

/// @brief Copies a serialized project node to the application clipboard.
/// @param project Project containing the node.
/// @param id Identifier of the node to copy.
/// @return True when the clipboard contains the serialized node.
bool copyNodeToClipboard(TextureProject& project, const int id) {
   const QString payload = project.serializeNode(id);
   if (payload.isEmpty()) {
      return false;
   }
   QApplication::clipboard()->setText(payload);
   return QApplication::clipboard()->text() == payload;
}

/// @brief Copies a project node to the clipboard before removing it.
/// @param project Project containing the node.
/// @param id Identifier of the node to cut.
/// @return True when both copying and removal succeeded.
bool cutNodeToClipboard(TextureProject& project, const int id) {
   if (!copyNodeToClipboard(project, id)) {
      return false;
   }
   project.removeNode(id);
   return true;
}

/// @brief Pastes serialized nodes from the application clipboard into a project.
/// @param project Project that receives the pasted nodes.
/// @return Number of nodes added to the project.
int pasteNodesFromClipboard(TextureProject& project) {
   return project.pasteNodes(QApplication::clipboard()->text());
}
