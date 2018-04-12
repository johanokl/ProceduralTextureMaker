/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "lines.h"
#include <QColor>
#include <QtMath>
#include <cmath>


LinesTextureGenerator::LinesTextureGenerator()
{
   TextureGeneratorSetting color;
   color.name = "Line color";
   color.defaultvalue = QVariant(QColor(255, 100, 50, 255));
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting lineheight;
   lineheight.name = "Line width";
   lineheight.defaultvalue = QVariant((int) 10);
   lineheight.min = QVariant(0);
   lineheight.max = QVariant(100);
   lineheight.order = 2;
   configurables.insert("lineheight", lineheight);

   TextureGeneratorSetting spacing;
   spacing.name = "Spacing";
   spacing.defaultvalue = QVariant((int) 10);
   spacing.min = QVariant(0);
   spacing.max = QVariant(100);
   spacing.order = 3;
   configurables.insert("spacing", spacing);

   TextureGeneratorSetting offset;
   offset.name = "Offset";
   offset.defaultvalue = QVariant((int) 0);
   offset.min = QVariant(-100);
   offset.max = QVariant(0);
   offset.order = 4;
   configurables.insert("offset", offset);

   TextureGeneratorSetting angle;
   angle.name = "Angle";
   angle.defaultvalue = QVariant((double) 0);
   angle.min = QVariant(0);
   angle.max = QVariant(180);
   angle.order = 5;
   configurables.insert("angle", angle);
}


void LinesTextureGenerator::generate(QSize size,
                                     TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   int spacing = settings->value("spacing").toDouble() * size.height() / 100;
   int lineheight = settings->value("lineheight").toDouble() * size.height() / 100;
   int offset = settings->value("offset").toDouble() * size.height() / 100;
   double angle = settings->value("angle").toDouble();

   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   TexturePixel filler(color.red(), color.green(), color.blue(), 255);
   bool invert = false;
   if (angle > 90) {
      invert = true;
      angle = 180 - angle;
   }
   angle = (angle / 180.0) * ((double) M_PI);
   double x1 = 0;
   double y1 = -10000;
   double x1rot = (x1 * cos(angle)) - (y1 * sin(angle));
   double y1rot = (y1 * cos(angle)) + (x1 * sin(angle));
   x1 = x1rot;
   y1 = y1rot;
   double x2 = -x1;
   double y2 = -y1;
   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double x3 = x;
         double y3 = y;
         double x4 = -10000;
         double y4 = y;
         double x4rot = ((x4 - x3) * cos(angle)) - ((y4 - y3) * sin(angle)) + x3;
         double y4rot = ((y4 - y3) * cos(angle)) + ((x4 - x3) * sin(angle)) + y3;
         x4 = x4rot;
         y4 = y4rot;
         double intersection_x = ((x2 * y1 - x1 * y2) * (x4 - x) - (x4 * y - x * y4) * (x2 - x1)) /
               ((x2 - x1) * (y4 - y) - (x4 - x) * (y2 - y1));
         double intersection_y = ((x2 * y1 - x1 * y2) * (y4 - y) - (x4 * y - x * y4) * (y2 - y1)) /
               ((x2 - x1) * (y4 - y) - (x4 - x) * (y2 - y1));
         int distance = sqrt((x - intersection_x) * (x - intersection_x) + (y - intersection_y) * (y - intersection_y));
         if (((distance - offset) % (lineheight + spacing)) > spacing) {
            if (invert) {
               destimage[y * size.width() + size.width() - x - 1] = filler;
            } else {
               destimage[y * size.width() + x] = filler;
            }
         }
      }
   }
}
