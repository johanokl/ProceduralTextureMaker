
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "lines.h"
#include <QColor>
#include <QtMath>
#include <cmath>

LinesTextureGenerator::LinesTextureGenerator() {
   TextureGeneratorSetting color;
   color.name = "Line colour";
   color.description = "Colour used to draw the lines.";
   color.defaultvalue = QVariant(QColor(255, 100, 50, 255));
   color.id = "color";
   configurables.append(color);

   TextureGeneratorSetting lineheight;
   lineheight.name = "Line thickness (%)";
   lineheight.description = "Thickness of each line as a percentage of the texture height.";
   lineheight.defaultvalue = QVariant((int)10);
   lineheight.min = QVariant(0);
   lineheight.max = QVariant(100);
   lineheight.id = "lineheight";
   configurables.append(lineheight);

   TextureGeneratorSetting spacing;
   spacing.name = "Gap size (%)";
   spacing.description = "Gap between adjacent lines as a percentage of the texture height.";
   spacing.defaultvalue = QVariant((int)10);
   spacing.min = QVariant(0);
   spacing.max = QVariant(100);
   spacing.id = "spacing";
   configurables.append(spacing);

   TextureGeneratorSetting offset;
   offset.name = "Pattern offset (%)";
   offset.description = "Shifts the repeating line pattern along its perpendicular axis.";
   offset.defaultvalue = QVariant((int)0);
   offset.min = QVariant(-100);
   offset.max = QVariant(0);
   offset.id = "offset";
   configurables.append(offset);

   TextureGeneratorSetting angle;
   angle.name = "Angle (°)";
   angle.description = "Rotation angle of the lines.";
   angle.defaultvalue = QVariant((double)0);
   angle.min = QVariant(0);
   angle.max = QVariant(180);
   angle.id = "angle";
   configurables.append(angle);
}
void LinesTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                     const QMap<QString, TextureImagePtr>& sourceimages,
                                     const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   int spacing = settings.value("spacing").toDouble() * size.height() / 100;
   int lineheight = settings.value("lineheight").toDouble() * size.height() / 100;
   int offset = settings.value("offset").toDouble() * size.height() / 100;
   double angle = settings.value("angle").toDouble();
   int period = lineheight + spacing;

   if (sourceimages.contains(QStringLiteral("Input"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Input"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   if (period <= 0) {
      return;
   }

   TexturePixel filler(color.red(), color.green(), color.blue(), 255);
   bool invert = false;
   if (angle > 90) {
      invert = true;
      angle = 180 - angle;
   }
   angle = (angle / 180.0) * ((double)M_PI);
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
         int distance = sqrt((x - intersection_x) * (x - intersection_x) +
                             (y - intersection_y) * (y - intersection_y));
         if (((distance - offset) % period) > spacing) {
            if (invert) {
               destimage[y * size.width() + size.width() - x - 1] = filler;
            } else {
               destimage[y * size.width() + x] = filler;
            }
         }
      }
   }
}
