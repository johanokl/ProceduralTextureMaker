// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef RENDERSTATUSOVERLAY_H
#define RENDERSTATUSOVERLAY_H

#include "base/texturerenderstatus.h"
#include <QPainter>

/// @brief Draws the same status overlay on graph thumbnails and preview images.
/// @param painter Painter whose current image is retained below the overlay.
/// @param rect Image area to mark.
/// @param state Current thumbnail render state.
inline void paintRenderStatusOverlay(QPainter& painter, const QRectF& rect,
                                     TextureRenderState state) {
   if (state == TextureRenderState::Ready) {
      return;
   }
   if (state == TextureRenderState::Rendering || state == TextureRenderState::Stopped) {
      painter.fillRect(rect, QBrush(QColor(230, 126, 34, 180), Qt::DiagCrossPattern));
      return;
   }

   QColor color;
   QString label;
   switch (state) {
      case TextureRenderState::Failed:
         color = QColor("#b73535");
         label = QStringLiteral("Render failed");
         break;
      case TextureRenderState::Blocked:
         color = QColor("#93601b");
         label = QStringLiteral("Input failed");
         break;
      case TextureRenderState::Cancelled:
         color = QColor("#596170");
         label = QStringLiteral("Cancelled");
         break;
      default:
         return;
   }
   painter.save();
   QColor tint = color;
   tint.setAlpha(state == TextureRenderState::Failed ? 80 : 32);
   painter.fillRect(rect, tint);
   const qreal height = qMin(26.0, rect.height());
   const QRectF banner(rect.left(), rect.bottom() - height, rect.width(), height);
   painter.fillRect(banner, color);
   QFont font = painter.font();
   font.setPixelSize(12);
   painter.setFont(font);
   painter.setPen(Qt::white);
   painter.drawText(banner, Qt::AlignCenter, label);
   painter.restore();
}

#endif  // RENDERSTATUSOVERLAY_H
