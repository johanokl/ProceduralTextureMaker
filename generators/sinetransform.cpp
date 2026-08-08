
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "sinetransform.h"
#include <QtMath>
#include <cmath>

SineTransformTextureGenerator::SineTransformTextureGenerator() {
   TextureGeneratorSetting angle;
   angle.name = "Wave angle (°)";
   angle.description = "Sets the direction along which both sine waves distort the image.";
   angle.defaultvalue = QVariant((double)45);
   angle.min = QVariant(0);
   angle.max = QVariant(90);
   angle.id = "angle";
   configurables.append(angle);

   TextureGeneratorSetting frequencyone;
   frequencyone.name = "First frequency";
   frequencyone.description = "Controls how often the first wave repeats across the texture.";
   frequencyone.defaultvalue = QVariant((double)0.5);
   frequencyone.min = QVariant(0);
   frequencyone.max = QVariant(50);
   frequencyone.id = "frequencyone";
   configurables.append(frequencyone);

   TextureGeneratorSetting amplitudeone;
   amplitudeone.name = "First amplitude (%)";
   amplitudeone.description =
       "Maximum displacement caused by the first wave as a percentage of the texture width.";
   amplitudeone.defaultvalue = QVariant((double)20);
   amplitudeone.min = QVariant(0);
   amplitudeone.max = QVariant(100);
   amplitudeone.id = "amplitudeone";
   configurables.append(amplitudeone);

   TextureGeneratorSetting offsetone;
   offsetone.name = "First phase offset";
   offsetone.description = "Shifts the first wave along its cycle.";
   offsetone.defaultvalue = QVariant((double)0);
   offsetone.min = QVariant(-360);
   offsetone.max = QVariant(360);
   offsetone.id = "offsetone";
   configurables.append(offsetone);

   TextureGeneratorSetting frequencytwo;
   frequencytwo.name = "Second frequency";
   frequencytwo.description = "Controls how often the second wave repeats across the texture.";
   frequencytwo.defaultvalue = QVariant((double)10);
   frequencytwo.min = QVariant(0);
   frequencytwo.max = QVariant(50);
   frequencytwo.id = "frequencytwo";
   configurables.append(frequencytwo);

   TextureGeneratorSetting amplitudetwo;
   amplitudetwo.name = "Second amplitude (%)";
   amplitudetwo.description =
       "Maximum displacement caused by the second wave as a percentage of the texture width.";
   amplitudetwo.defaultvalue = QVariant((double)6);
   amplitudetwo.min = QVariant(0);
   amplitudetwo.max = QVariant(50);
   amplitudetwo.id = "amplitudetwo";
   configurables.append(amplitudetwo);

   TextureGeneratorSetting offsettwo;
   offsettwo.name = "Second phase offset";
   offsettwo.description = "Shifts the second wave along its cycle.";
   offsettwo.defaultvalue = QVariant((double)0);
   offsettwo.min = QVariant(-360);
   offsettwo.max = QVariant(360);
   offsettwo.id = "offsettwo";
   configurables.append(offsettwo);
}
void SineTransformTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                             const QMap<QString, TextureImagePtr>& sourceimages,
                                             const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Image"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   double angle = settings.value("angle").toDouble();
   double frequencyone = settings.value("frequencyone").toDouble() * 5 / size.width();
   double amplitudeone = settings.value("amplitudeone").toDouble() * size.width() / 100;
   double offsetone = settings.value("offsetone").toDouble() * 5 / size.width();
   double frequencytwo = settings.value("frequencytwo").toDouble() * 5 / size.width();
   double amplitudetwo = settings.value("amplitudetwo").toDouble() * size.width() / 100;
   double offsettwo = settings.value("offsettwo").toDouble() * 5 / size.width();

   TexturePixel* source = sourceimages.value(QStringLiteral("Image"))->getData();
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
         double distance = sqrt((x - intersection_x) * (x - intersection_x) +
                                (y - intersection_y) * (y - intersection_y));
         double srcDistance = sin(distance * frequencyone + offsetone) * amplitudeone +
                              sin(distance * frequencytwo + offsettwo) * amplitudetwo;
         x4 = x;
         y4 = y - srcDistance;
         int xpos = ((x4 - x) * cos(angle)) - ((y4 - y) * sin(angle)) + x;
         int ypos = ((y4 - y) * cos(angle)) + ((x4 - x) * sin(angle)) + y;
         xpos =
             xpos > size.height() ? xpos % size.height() : (xpos < 0 ? xpos + size.width() : xpos);
         ypos =
             ypos > size.height() ? ypos % size.height() : (ypos < 0 ? ypos + size.height() : ypos);
         xpos = qMax(qMin(xpos, size.width() - 1), 0);
         ypos = qMax(qMin(ypos, size.height() - 1), 0);
         destimage[y * size.width() + x] = source[ypos * size.width() + xpos];
      }
   }
}
