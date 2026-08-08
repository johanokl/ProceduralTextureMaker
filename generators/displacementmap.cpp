
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include <cmath>
#include <QtMath>
#include "displacementmap.h"

using namespace std;

DisplacementMapTextureGenerator::DisplacementMapTextureGenerator() {
   TextureGeneratorSetting angle;
   angle.name = "Direction (°)";
   angle.description = "Direction in which the displacement map moves source pixels.";
   angle.defaultvalue = QVariant((double)45);
   angle.min = QVariant(-360);
   angle.max = QVariant(360);
   angle.id = "angle";
   configurables.append(angle);

   TextureGeneratorSetting strength;
   strength.name = "Map strength";
   strength.description = "Controls how far luminance values displace source pixels.";
   strength.defaultvalue = QVariant((double)0.5);
   strength.min = QVariant(0);
   strength.max = QVariant(3);
   strength.id = "strength";
   configurables.append(strength);

   TextureGeneratorSetting offset;
   offset.name = "Distance offset (%)";
   offset.description = "Offsets the map-derived distance by a percentage of the texture width.";
   offset.defaultvalue = QVariant((double)0);
   offset.min = QVariant(-360);
   offset.max = QVariant(360);
   offset.id = "offset";
   configurables.append(offset);
}
void DisplacementMapTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                               const QMap<QString, TextureImagePtr>& sourceimages,
                                               const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Source image"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(QStringLiteral("Source image"))->getData();
   if (!sourceimages.contains(QStringLiteral("Map"))) {
      memcpy(destimage, sourceImage, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceMap = sourceimages.value(QStringLiteral("Map"))->getData();

   double strength = settings.value("strength").toDouble() * size.width() / 500;
   double offset = settings.value("offset").toDouble() * size.width() / 100;
   double angle = settings.value("angle").toDouble();
   angle = (angle / 180.0) * ((double)M_PI);

   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double srcDistance = sourceMap[y * size.width() + x].intensityWithAlpha();
         srcDistance *= strength;
         srcDistance -= offset;
         int xpos = x + srcDistance * sin(angle);
         int ypos = y - srcDistance * cos(angle);
         xpos =
             xpos > size.height() ? xpos % size.height() : (xpos < 0 ? xpos + size.width() : xpos);
         ypos =
             ypos > size.height() ? ypos % size.height() : (ypos < 0 ? ypos + size.height() : ypos);
         xpos = qMax(qMin(xpos, size.width() - 1), 0);
         ypos = qMax(qMin(ypos, size.height() - 1), 0);
         destimage[y * size.width() + x] = sourceImage[ypos * size.width() + xpos];
      }
   }
}
