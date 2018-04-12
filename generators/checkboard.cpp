/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "checkboard.h"
#include <QColor>
#include <QPainter>

CheckboardTextureGenerator::CheckboardTextureGenerator()
{
   TextureGeneratorSetting color;
   color.name = "Color";
   color.defaultvalue = QVariant(QColor(0, 0, 0, 255));
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting brickwidth;
   brickwidth.name = "Brick width";
   brickwidth.defaultvalue = QVariant((int) 10);
   brickwidth.min = QVariant(1);
   brickwidth.max = QVariant(300);
   brickwidth.order = 2;
   configurables.insert("brickwidth", brickwidth);

   TextureGeneratorSetting brickheight;
   brickheight.name = "Brick height";
   brickheight.defaultvalue = QVariant((int) 10);
   brickheight.min = QVariant(1);
   brickheight.max = QVariant(300);
   brickheight.order = 3;
   configurables.insert("brickheight", brickheight);

   TextureGeneratorSetting offsetx;
   offsetx.name = "Offset left";
   offsetx.defaultvalue = QVariant((int) 0);
   offsetx.min = QVariant(-100);
   offsetx.max = QVariant(100);
   offsetx.order = 4;
   configurables.insert("offsetx", offsetx);

   TextureGeneratorSetting offsety;
   offsety.name = "Offset top";
   offsety.defaultvalue = QVariant((int) 0);
   offsety.min = QVariant(-100);
   offsety.max = QVariant(100);
   offsety.order = 5;
   configurables.insert("offsety", offsety);
}


void CheckboardTextureGenerator::generate(QSize size,
                                          TexturePixel* destimage,
                                          QMap<int, TextureImagePtr> sourceimages,
                                          TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   int brickheight = settings->value("brickheight").toDouble() * size.height() / 300;
   int brickwidth = settings->value("brickwidth").toDouble() * size.width() / 300;
   int offsetx = settings->value("offsetx").toDouble() * size.width() / 100;
   int offsety = settings->value("offsety").toDouble() * size.height() / 100;

   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));
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
   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
