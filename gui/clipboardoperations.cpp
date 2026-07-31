// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "clipboardoperations.h"
#include "base/textureproject.h"
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

bool cutNodeToClipboard(TextureProject& project, const int id) {
   if (!copyNodeToClipboard(project, id)) {
      return false;
   }
   project.removeNode(id);
   return true;
}

int pasteNodesFromClipboard(TextureProject& project) {
   return project.pasteNodes(QApplication::clipboard()->text());
}
