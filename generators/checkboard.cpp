
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "checkboard.h"
#include <QColor>
#include <QPainter>

CheckboardTextureGenerator::CheckboardTextureGenerator() {
   TextureGeneratorSetting color;
   color.name = "Colour";
   color.description = "Colour used for alternating squares.";
   color.defaultvalue = QVariant(QColor(0, 0, 0, 255));
   color.id = "color";
   configurables.append(color);

   TextureGeneratorSetting brickwidth;
   brickwidth.name = "Square width";
   brickwidth.description = "Width of each square, relative to a 300-pixel texture width.";
   brickwidth.defaultvalue = QVariant((int)10);
   brickwidth.min = QVariant(1);
   brickwidth.max = QVariant(300);
   brickwidth.id = "brickwidth";
   configurables.append(brickwidth);

   TextureGeneratorSetting brickheight;
   brickheight.name = "Square height";
   brickheight.description = "Height of each square, relative to a 300-pixel texture height.";
   brickheight.defaultvalue = QVariant((int)10);
   brickheight.min = QVariant(1);
   brickheight.max = QVariant(300);
   brickheight.id = "brickheight";
   configurables.append(brickheight);

   TextureGeneratorSetting offsetx;
   offsetx.name = "Horizontal offset (%)";
   offsetx.description = "Moves the pattern horizontally as a percentage of the texture width.";
   offsetx.defaultvalue = QVariant((int)0);
   offsetx.min = QVariant(-100);
   offsetx.max = QVariant(100);
   offsetx.id = "offsetx";
   configurables.append(offsetx);

   TextureGeneratorSetting offsety;
   offsety.name = "Vertical offset (%)";
   offsety.description = "Moves the pattern vertically as a percentage of the texture height.";
   offsety.defaultvalue = QVariant((int)0);
   offsety.min = QVariant(-100);
   offsety.max = QVariant(100);
   offsety.id = "offsety";
   configurables.append(offsety);
}
void CheckboardTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                          const QMap<QString, TextureImagePtr>& sourceimages,
                                          const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   int brickheight =
       qMax(1, static_cast<int>(settings.value("brickheight").toDouble() * size.height() / 300));
   int brickwidth =
       qMax(1, static_cast<int>(settings.value("brickwidth").toDouble() * size.width() / 300));
   int offsetx = settings.value("offsetx").toDouble() * size.width() / 100;
   int offsety = settings.value("offsety").toDouble() * size.height() / 100;

   if (sourceimages.contains(QStringLiteral("Input"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Input"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   QImage tempimage = makeTextureImageView(size, destimage);
   QPainter painter(&tempimage);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setBrush(QBrush(color, Qt::BrushStyle::SolidPattern));
   painter.setPen(Qt::NoPen);

   bool invert = false;
   int currY = offsety - size.height() - brickheight;
   while (currY < size.height()) {
      invert = !invert;
      int currX = offsetx - size.width() - brickwidth;
      if (invert) {
         currX += brickwidth;
      }
      while (currX < size.width()) {
         painter.drawRect(currX, currY, brickwidth, brickheight);
         currX += brickwidth * 2;
      }
      currY += brickheight;
   }
}
