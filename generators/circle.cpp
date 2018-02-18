/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include "circle.h"

using namespace std;

CircleTextureGenerator::CircleTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Color";
   colorsetting.description = "Color of the circle";
   colorsetting.defaultvalue = QVariant(QColor(200, 100, 0, 200));
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting innerRadius;
   innerRadius.name = "Inner radius";
   innerRadius.description = "Inner radius of the circle in percent of width";
   innerRadius.defaultvalue = QVariant((double) 0);
   innerRadius.min = QVariant((double) 0);
   innerRadius.max = QVariant((double) 200);
   innerRadius.order = 2;
   configurables.insert("innerradius", innerRadius);

   TextureGeneratorSetting outerRadius;
   outerRadius.name = "Outer radius";
   outerRadius.description = "Outer radius of the circle in percent of width";
   outerRadius.defaultvalue = QVariant((double) 100);
   outerRadius.min = QVariant((double) 0);
   outerRadius.max = QVariant((double) 200);
   outerRadius.order = 3;
   configurables.insert("outerradius", outerRadius);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset left";
   offsetLeft.description = "Inner radius of the circle in percent of width";
   offsetLeft.defaultvalue = QVariant((double) 0);
   offsetLeft.min = QVariant((double) -100);
   offsetLeft.max = QVariant((double) 100);
   offsetLeft.order = 4;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset top";
   offsetTop.description = "Outer radius of the circle in percent of width";
   offsetTop.defaultvalue = QVariant((double) 0);
   offsetTop.min = QVariant((double) -100);
   offsetTop.max = QVariant((double) 100);
   offsetTop.order = 5;
   configurables.insert("offsettop", offsetTop);
}


void CircleTextureGenerator::generate(QSize size,
                                      TexturePixel* destimage,
                                      QMap<int, TextureImagePtr> sourceimages,
                                      TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   double innerRadius = settings->value("innerradius").toDouble() * size.height() / 200.0;
   double outerRadius = settings->value("outerradius").toDouble() * size.height() / 200.0;
   int offsetLeft = settings->value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings->value("offsettop").toDouble() * size.height() / 100;

   bool blend = false;
   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
      blend = true;
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   if (color.alpha() == 255) {
      blend = false;
   }
   double alpha = color.alphaF();
   double srcAlpha = 1 - alpha;
   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         if (((pow(abs(size.width() / 2 - x + offsetLeft), 2)
               + pow(abs(size.height() / 2 - y + offsetTop), 2))
              >= (pow(innerRadius, 2))) &&
             ((pow(abs(size.width() / 2 - x + offsetLeft), 2)
               + pow(abs(size.height() / 2 - y + offsetTop), 2))
              <= (pow(outerRadius, 2)))) {
            int thisPos = y * size.width() + x;
            destimage[thisPos].r = (int) (alpha * color.red() + (blend ? (srcAlpha * destimage[thisPos].r) : 0));
            destimage[thisPos].g = (int) (alpha * color.green() + (blend ? (srcAlpha * destimage[thisPos].g) : 0));
            destimage[thisPos].b = (int) (alpha * color.blue() + (blend ? (srcAlpha * destimage[thisPos].b) : 0));
            destimage[thisPos].a = (int) (color.alpha() + (blend ? (srcAlpha * destimage[thisPos].a) : 0));
         }
      }
   }
}
