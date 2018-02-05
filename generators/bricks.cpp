/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QColor>
#include <QPainter>
#include <QSize>
#include <math.h>
#include "core/textureimage.h"
#include "bricks.h"

using namespace std;

BricksTextureGenerator::BricksTextureGenerator()
{
   TextureGeneratorSetting color;
   color.name = "Color";
   color.defaultvalue = QVariant(QColor(200, 200, 200, 255));
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting linewidth;
   linewidth.name = "Line width";
   linewidth.defaultvalue = QVariant((int) 10);
   linewidth.min = QVariant((int) 0);
   linewidth.max = QVariant((int) 100);
   linewidth.order = 2;
   configurables.insert("linewidth", linewidth);

   TextureGeneratorSetting brickwidth;
   brickwidth.name = "Brick width";
   brickwidth.defaultvalue = QVariant((int) 40);
   brickwidth.min = QVariant((int) 0);
   brickwidth.max = QVariant((int) 100);
   brickwidth.order = 3;
   configurables.insert("brickwidth", brickwidth);

   TextureGeneratorSetting brickheight;
   brickheight.name = "Brick height";
   brickheight.defaultvalue = QVariant((int) 15);
   brickheight.min = QVariant((int) 0);
   brickheight.max = QVariant((int) 100);
   brickheight.order = 4;
   configurables.insert("brickheight", brickheight);

   TextureGeneratorSetting offsetx;
   offsetx.name = "Offset left";
   offsetx.defaultvalue = QVariant((int) 0);
   offsetx.min = QVariant((int) -100);
   offsetx.max = QVariant((int) 100);
   offsetx.order = 5;
   configurables.insert("offsetx", offsetx);

   TextureGeneratorSetting offsety;
   offsety.name = "Offset top";
   offsety.defaultvalue = QVariant((int) 0);
   offsety.min = QVariant((int) -100);
   offsety.max = QVariant((int) 100);
   offsety.order = 6;
   configurables.insert("offsety", offsety);
}


void BricksTextureGenerator::generate(QSize size,
                                      TexturePixel* destimage,
                                      QMap<int, TextureImagePtr> sourceimages,
                                      TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   int linewidth = settings->value("linewidth").toDouble() * size.height() / 300;
   int brickheight = settings->value("brickheight").toDouble() * size.height() / 100;
   int brickwidth = settings->value("brickwidth").toDouble() * size.width() / 100;
   int offsetx = settings->value("offsetx").toDouble() * size.width() / 100;
   int offsety = settings->value("offsety").toDouble() * size.height() / 100;

   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
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
   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));
   QPainter painter(&tempimage);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setBrush(QBrush(color, Qt::BrushStyle::SolidPattern));
   painter.setPen(QPen(color, linewidth, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
   painter.drawLines(lines);
   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
