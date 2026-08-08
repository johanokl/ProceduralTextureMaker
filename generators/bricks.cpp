
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "bricks.h"
#include <QColor>
#include <QPainter>

BricksTextureGenerator::BricksTextureGenerator() {
   TextureGeneratorSetting color;
   color.name = "Colour";
   color.description = "Colour used to draw the mortar lines.";
   color.defaultvalue = QVariant(QColor(200, 200, 200, 255));
   color.id = "color";
   configurables.append(color);

   TextureGeneratorSetting linewidth;
   linewidth.name = "Line width";
   linewidth.description = "Width of the mortar lines, relative to a 300-pixel texture height.";
   linewidth.defaultvalue = QVariant((int)10);
   linewidth.min = QVariant(0);
   linewidth.max = QVariant(100);
   linewidth.id = "linewidth";
   configurables.append(linewidth);

   TextureGeneratorSetting brickwidth;
   brickwidth.name = "Brick width";
   brickwidth.description = "Width of each brick, relative to a 300-pixel texture width.";
   brickwidth.defaultvalue = QVariant((int)120);
   brickwidth.min = QVariant(0);
   brickwidth.max = QVariant(300);
   brickwidth.group = "size";
   brickwidth.id = "brickwidth";
   configurables.append(brickwidth);

   TextureGeneratorSetting brickheight;
   brickheight.name = "Brick height";
   brickheight.description = "Height of each brick, relative to a 300-pixel texture height.";
   brickheight.defaultvalue = QVariant((int)45);
   brickheight.min = QVariant(0);
   brickheight.max = QVariant(300);
   brickheight.group = "size";
   brickheight.id = "brickheight";
   configurables.append(brickheight);

   TextureGeneratorSetting offsetx;
   offsetx.name = "Horizontal offset (%)";
   offsetx.description = "Moves the brickwork horizontally as a percentage of the texture width.";
   offsetx.defaultvalue = QVariant((int)0);
   offsetx.min = QVariant(-100);
   offsetx.max = QVariant(100);
   offsetx.id = "offsetx";
   configurables.append(offsetx);

   TextureGeneratorSetting offsety;
   offsety.name = "Vertical offset (%)";
   offsety.description = "Moves the brickwork vertically as a percentage of the texture height.";
   offsety.defaultvalue = QVariant((int)0);
   offsety.min = QVariant(-100);
   offsety.max = QVariant(100);
   offsety.id = "offsety";
   configurables.append(offsety);
}
void BricksTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   int linewidth = settings.value("linewidth").toDouble() * size.height() / 300;
   int brickheight =
       qMax(1, static_cast<int>(settings.value("brickheight").toDouble() * size.height() / 300));
   int brickwidth =
       qMax(1, static_cast<int>(settings.value("brickwidth").toDouble() * size.width() / 300));
   int offsetx = settings.value("offsetx").toDouble() * size.width() / 100;
   int offsety = settings.value("offsety").toDouble() * size.height() / 100;

   if (sourceimages.contains(QStringLiteral("Background"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Background"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   bool invert = false;
   QVector<QLine> lines;

   int currY = offsety - size.height() - brickheight / 2;
   while (currY < (size.height() + linewidth / 2)) {
      if (currY > (-linewidth / 2) && currY < (size.height() + linewidth)) {
         lines.append(QLine(0, currY, size.width(), currY));
      }
      invert = !invert;
      int currX = offsetx - size.width() - brickwidth / 2;
      if (invert) {
         currX += brickwidth / 2;
      }
      while (currX < (size.width() + linewidth / 2)) {
         if (currX > (-linewidth / 2) && currX < (size.width() + linewidth)) {
            lines.append(QLine(currX, currY, currX, currY + brickheight + linewidth));
         }
         currX += brickwidth + linewidth;
      }
      currY += brickheight + linewidth;
   }
   QImage tempimage = makeTextureImageView(size, destimage);
   QPainter painter(&tempimage);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setBrush(QBrush(color, Qt::BrushStyle::SolidPattern));
   painter.setPen(QPen(color, linewidth, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
   painter.drawLines(lines);
}
