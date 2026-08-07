// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREBACKGROUND_H
#define TEXTUREBACKGROUND_H

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPixmap>

namespace TextureBackground {

/// @brief Composites a texture over a base color and optional patterned overlay.
inline QPixmap composite(const QPixmap& texture, const QColor& backgroundColor,
                         const QColor& overlayColor, const Qt::BrushStyle overlayStyle) {
   if (texture.isNull()) {
      return {};
   }

   QPixmap result(texture.size());
   result.fill(backgroundColor);
   {
      QPainter painter(&result);
      if (overlayStyle != Qt::NoBrush) {
         painter.fillRect(result.rect(), QBrush(overlayColor, overlayStyle));
      }
      painter.drawPixmap(result.rect(), texture, texture.rect());
   }
   result.setDevicePixelRatio(texture.devicePixelRatio());
   return result;
}

}  // namespace TextureBackground

#endif  // TEXTUREBACKGROUND_H
