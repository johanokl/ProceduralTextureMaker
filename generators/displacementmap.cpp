/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QtMath>
#include "displacementmap.h"

using namespace std;

DisplacementMapTextureGenerator::DisplacementMapTextureGenerator()
{
   TextureGeneratorSetting angle;
   angle.name = "Angle";
   angle.defaultvalue = QVariant((double) 45);
   angle.min = QVariant((double) -360);
   angle.max = QVariant((double) 360);
   angle.order = 1;
   configurables.insert("angle", angle);

   TextureGeneratorSetting strength;
   strength.name = "Strength";
   strength.defaultvalue = QVariant((double) 0.5);
   strength.min = QVariant((double) 0);
   strength.max = QVariant((double) 3);
   strength.order = 3;
   configurables.insert("strength", strength);

   TextureGeneratorSetting offset;
   offset.name = "Offset";
   offset.defaultvalue = QVariant((double) 0);
   offset.min = QVariant((double) -360);
   offset.max = QVariant((double) 360);
   offset.order = 4;
   configurables.insert("offset", offset);

}


void DisplacementMapTextureGenerator::generate(QSize size,
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
   TexturePixel* sourceImage = sourceimages.value(0)->getData();
   if (!sourceimages.contains(1)) {
      memcpy(destimage, sourceImage, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceMap = sourceimages.value(1)->getData();

   double strength = settings->value("strength").toDouble() * size.width() / 500;
   double offset = settings->value("offset").toDouble() * size.width() / 100;
   double angle = settings->value("angle").toDouble();
   angle = (angle / 180.0) * ((double) M_PI);

   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double srcDistance = sourceMap[y * size.width() + x].intensityWithAlpha();
         srcDistance *= strength;
         srcDistance -= offset;
         int xpos = x + srcDistance * sin(angle);
         int ypos = y - srcDistance * cos(angle);
         xpos = xpos > size.height() ? xpos % size.height() :
                                       (xpos < 0 ? xpos + size.width() : xpos);
         ypos = ypos > size.height() ? ypos % size.height() :
                                       (ypos < 0 ? ypos + size.height() : ypos);
         xpos = qMax(qMin(xpos, size.width() - 1), 0);
         ypos = qMax(qMin(ypos, size.height() - 1), 0);
         destimage[y * size.width() + x] = sourceImage[ypos * size.width() + xpos];
      }
   }
}
