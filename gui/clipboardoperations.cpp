// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "clipboardoperations.h"
#include "base/textureproject.h"
#include "base/editmanager.h"
#include <QApplication>
#include <QClipboard>
#include <QString>

bool copyNodeToClipboard(TextureProject& project, const int id) {
   const QString payload = project.serializeNode(id);
   if (payload.isEmpty()) {
      return false;
   }
   QApplication::clipboard()->setText(payload);
   return QApplication::clipboard()->text() == payload;
}

bool cutNodeToClipboard(TextureProject& project, EditManager& editManager, const int id) {
   if (!copyNodeToClipboard(project, id)) {
      return false;
   }
   return editManager.removeNode(id);
}

int pasteNodesFromClipboard(EditManager& editManager) {
   return editManager.pasteNodes(QApplication::clipboard()->text());
}
