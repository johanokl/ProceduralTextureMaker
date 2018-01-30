/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QColor>
#include <math.h>
#include "core/textureimage.h"
#include "whirl.h"

using namespace std;

WhirlTextureGenerator::WhirlTextureGenerator()
{
   TextureGeneratorSetting radius;
   radius.name = "Radius";
   radius.defaultvalue = QVariant((double) 50);
   radius.min = QVariant((double) 0);
   radius.max = QVariant((double) 200);
   radius.order = 1;
   configurables.insert("radius", radius);

   TextureGeneratorSetting strength;
   strength.name = "Strength";
   strength.defaultvalue = QVariant((double) 40);
   strength.min = QVariant((double) 0);
   strength.max = QVariant((double) 500);
   strength.order = 2;
   configurables.insert("strength", strength);

   TextureGeneratorSetting offsetleft;
   offsetleft.name = "Offset left";
   offsetleft.defaultvalue = QVariant((double) 0);
   offsetleft.min = QVariant((double) -100);
   offsetleft.max = QVariant((double) 100);
   offsetleft.order = 3;
   configurables.insert("offsetleft", offsetleft);

   TextureGeneratorSetting offsettop;
   offsettop.name = "Offset top:";
   offsettop.defaultvalue = QVariant((double) 0);
   offsettop.min = QVariant((double) -100);
   offsettop.max = QVariant((double) 100);
   offsettop.order = 4;
   configurables.insert("offsettop", offsettop);
}


void WhirlTextureGenerator::generate(QSize size,
                                     TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   double radius = configurables.value("radius").defaultvalue.toDouble() * size.width() / 100;
   double strength = configurables.value("strength").defaultvalue.toDouble() / 100;
   double offsetleft = configurables.value("offsetleft").defaultvalue.toDouble() * size.width() / 100;
   double offsettop = configurables.value("offsettop").defaultvalue.toDouble() * size.height() / 100;
   if (settings != NULL) {
      if (settings->contains("radius")) {
         radius = settings->value("radius").toDouble() * size.width() / 100;
      }
      if (settings->contains("strength")) {
         strength = settings->value("strength").toDouble() / 100;
      }
      if (settings->contains("offsetleft")) {
         offsetleft = settings->value("offsetleft").toDouble() * size.width() / 100;
      }
      if (settings->contains("offsettop")) {
         offsettop = settings->value("offsettop").toDouble() * size.height() / 100;
      }
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   TexturePixel* source = sourceimages.value(0)->getData();
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
         int xpos = ((x - centerx) * cos(angle)) - ((y - centerx) * sin(angle)) + centerx;
         int ypos = ((y - centery) * cos(angle)) + ((x - centery) * sin(angle)) + centery;
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
