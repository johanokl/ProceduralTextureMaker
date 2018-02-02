/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QColor>
#include <math.h>
#include <QDebug>
#include "core/textureimage.h"
#include "sinetransform.h"

using namespace std;

SineTransformTextureGenerator::SineTransformTextureGenerator()
{
   TextureGeneratorSetting angle;
   angle.name = "Angle";
   angle.defaultvalue = QVariant((double) 0);
   angle.min = QVariant((double) 0);
   angle.max = QVariant((double) 90);
   angle.order = 1;
   configurables.insert("angle", angle);

   TextureGeneratorSetting frequencyone;
   frequencyone.name = "Frequency";
   frequencyone.defaultvalue = QVariant((double) 3);
   frequencyone.min = QVariant((double) 0);
   frequencyone.max = QVariant((double) 10);
   frequencyone.order = 2;
   configurables.insert("frequencyone", frequencyone);

   TextureGeneratorSetting amplitudeone;
   amplitudeone.name = "Amplitude";
   amplitudeone.defaultvalue = QVariant((double) 10);
   amplitudeone.min = QVariant((double) -100);
   amplitudeone.max = QVariant((double) 100);
   amplitudeone.order = 3;
   configurables.insert("amplitudeone", amplitudeone);

   TextureGeneratorSetting offsetone;
   offsetone.name = "Offset";
   offsetone.defaultvalue = QVariant((double) -10);
   offsetone.min = QVariant((double) -360);
   offsetone.max = QVariant((double) 360);
   offsetone.order = 4;
   configurables.insert("offsetone", offsetone);
}


void SineTransformTextureGenerator::generate(QSize size,
                                     TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   double angle = settings->value("angle").toDouble();
   double frequencyone = settings->value("frequencyone").toDouble() * 5 / size.width();
   double amplitudeone = settings->value("amplitudeone").toDouble() * size.width() / 100;
   double offsetone = settings->value("offsetone").toDouble() * 5 / size.width();

   TexturePixel* source = sourceimages.value(0)->getData();
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
         double x4 = -10000;
         double y4 = y;
         double x4rot = ((x4 - x) * cos(angle)) - ((y4 - y) * sin(angle)) + x;
         double y4rot = ((y4 - y) * cos(angle)) + ((x4 - x) * sin(angle)) + y;
         x4 = x4rot;
         y4 = y4rot;
         double intersection_x = ((x2 * y1 - x1 * y2) * (x4 - x) - (x4 * y - x * y4) * (x2 - x1)) /
               ((x2 - x1) * (y4 - y) - (x4 - x) * (y2 - y1));
         double intersection_y = ((x2 * y1 - x1 * y2) * (y4 - y) - (x4 * y - x * y4) * (y2 - y1)) /
               ((x2 - x1) * (y4 - y) - (x4 - x) * (y2 - y1));
         double distance = sqrt((x - intersection_x) * (x - intersection_x) + (y - intersection_y) * (y - intersection_y));
         double srcDistance = sin(distance * frequencyone + offsetone) * amplitudeone;
         x4 = x;
         y4 = y - srcDistance;
         int xpos = ((x4 - x) * cos(angle)) - ((y4 - y) * sin(angle)) + x;
         int ypos = ((y4 - y) * cos(angle)) + ((x4 - x) * sin(angle)) + y;
         xpos = xpos > size.height() ? xpos % size.height() :
                                       (xpos < 0 ? xpos + size.width() : xpos);
         ypos = ypos > size.height() ? ypos % size.height() :
                                       (ypos < 0 ? ypos + size.height() : ypos);
         xpos = qMax(qMin(xpos, size.width() - 1), 0);
         ypos = qMax(qMin(ypos, size.height() - 1), 0);
         destimage[y * size.width() + x] = source[ypos * size.width() + xpos];
      }
   }
}
