
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "whirl.h"
#include <QtMath>
#include <cmath>

WhirlTextureGenerator::WhirlTextureGenerator() {
   TextureGeneratorSetting radius;
   radius.name = "Radius (%)";
   radius.description = "Radius of the affected area as a percentage of the texture width.";
   radius.defaultvalue = QVariant((double)50);
   radius.min = QVariant(0);
   radius.max = QVariant(200);
   radius.id = "radius";
   configurables.append(radius);

   TextureGeneratorSetting strength;
   strength.name = "Strength";
   strength.description = "Controls the amount and direction of twisting.";
   strength.defaultvalue = QVariant((double)40);
   strength.min = QVariant(-500);
   strength.max = QVariant(500);
   strength.id = "strength";
   configurables.append(strength);

   TextureGeneratorSetting offsetleft;
   offsetleft.name = "Horizontal centre offset (%)";
   offsetleft.description = "Moves the centre of the whirl horizontally.";
   offsetleft.defaultvalue = QVariant((double)0);
   offsetleft.min = QVariant(-100);
   offsetleft.max = QVariant(100);
   offsetleft.id = "offsetleft";
   configurables.append(offsetleft);

   TextureGeneratorSetting offsettop;
   offsettop.name = "Vertical centre offset (%)";
   offsettop.description = "Moves the centre of the whirl vertically.";
   offsettop.defaultvalue = QVariant((double)0);
   offsettop.min = QVariant(-100);
   offsettop.max = QVariant(100);
   offsettop.id = "offsettop";
   configurables.append(offsettop);
}
void WhirlTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                     const QMap<QString, TextureImagePtr>& sourceimages,
                                     const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   double radius = settings.value("radius").toDouble() * size.width() / 100;
   double strength = settings.value("strength").toDouble() / 80;
   double offsetleft = settings.value("offsetleft").toDouble() * size.width() / 100;
   double offsettop = settings.value("offsettop").toDouble() * size.height() / 100;

   TexturePixel* source = sourceimages.value(QStringLiteral("Input"))->getData();
   if (radius <= 0 || strength == 0) {
      memcpy(destimage, source, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   int centerx = size.width() / 2 + offsetleft;
   int centery = size.height() / 2 + offsettop;

   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double distortion = sqrt((x - centerx) * (x - centerx) + (y - centery) * (y - centery));
         if (distortion > radius) {
            distortion = 0;
         } else {
            distortion = (radius - distortion) * (radius - distortion) / radius;
         }
         double angle = 2 * M_PI * distortion / (radius / strength);
         int xpos = ((x - centerx) * cos(angle)) - ((y - centery) * sin(angle)) + centerx;
         int ypos = ((y - centery) * cos(angle)) + ((x - centerx) * sin(angle)) + centery;
         xpos = xpos > size.width() ? xpos % size.width() : (xpos < 0 ? xpos + size.width() : xpos);
         ypos =
             ypos > size.height() ? ypos % size.height() : (ypos < 0 ? ypos + size.height() : ypos);
         xpos = qMax(qMin(xpos, size.width() - 1), 0);
         ypos = qMax(qMin(ypos, size.height() - 1), 0);
         destimage[y * size.width() + x] = source[ypos * size.width() + xpos];
      }
   }
}
