/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QtMath>
#include <QColor>
#include "sineplasma.h"
#include "core/textureimage.h"


using namespace std;

SinePlasmaTextureGenerator::SinePlasmaTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(255, 0, 0));
   colorsetting.name = "Color";
   colorsetting.description = "Color of the circle";
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting xoffset;
   xoffset.defaultvalue = QVariant((double) 10);
   xoffset.name = "Offset X axis";
   xoffset.description = "Offset along the x axis";
   xoffset.min = QVariant((double) -100);
   xoffset.max = QVariant((double) 100);
   xoffset.order = 2;
   configurables.insert("xoffset", xoffset);

   TextureGeneratorSetting yoffset;
   yoffset.defaultvalue = QVariant((double) 10);
   yoffset.name = "Offset Y axis";
   yoffset.description = "Offset along the y axis";
   yoffset.min = QVariant((double) -100);
   yoffset.max = QVariant((double) 100);
   yoffset.order = 3;
   configurables.insert("yoffset", yoffset);

   TextureGeneratorSetting xfrequency;
   xfrequency.defaultvalue = QVariant((double) 10);
   xfrequency.name = "Frequency X axis";
   xfrequency.description = "How often it repeats along the y axis";
   xfrequency.min = QVariant((double) 0);
   xfrequency.max = QVariant((double) 100);
   xfrequency.order = 4;
   configurables.insert("xfrequency", xfrequency);

   TextureGeneratorSetting yfrequency;
   yfrequency.defaultvalue = QVariant((double) 10);
   yfrequency.name = "Frequency Y axis";
   yfrequency.description = "How often it repeats along the y axis";
   yfrequency.min = QVariant((double) 0);
   yfrequency.max = QVariant((double) 100);
   yfrequency.order = 5;
   configurables.insert("yfrequency", yfrequency);
}


void SinePlasmaTextureGenerator::generate(QSize size,
                                          TexturePixel* destimage,
                                          QMap<int, TextureImagePtr> sourceimages,
                                          TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = configurables.value("color").defaultvalue.value<QColor>();
   double xoffset = configurables.value("xoffset").defaultvalue.toDouble() * size.width() / 100;
   double yoffset = configurables.value("yoffset").defaultvalue.toDouble() * size.height() / 100;
   double xfrequency = configurables.value("xfrequency").defaultvalue.toDouble() * 5 / size.width();
   double yfrequency = configurables.value("yfrequency").defaultvalue.toDouble() * 5 / size.height();
   if (settings != NULL) {
      if (settings->contains("color")) {
         color = settings->value("color").value<QColor>();
      }
      if (settings->contains("xoffset")) {
         xoffset = settings->value("xoffset").toInt() * size.width() / 100;
      }
      if (settings->contains("yoffset")) {
         yoffset = settings->value("yoffset").toInt() * size.height() / 100;
      }
      if (settings->contains("xfrequency")) {
         xfrequency = settings->value("xfrequency").toDouble() * 5 / size.width();
      }
      if (settings->contains("yfrequency")) {
         yfrequency = settings->value("yfrequency").toDouble() * 5 / size.height();
      }
   }
   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double value = 0.5 + 0.25 * qSin((x - xoffset) * xfrequency) + 0.25 * qSin((y - yoffset) * yfrequency);
         double negVal = 1 - value;
         int pixelPos = y * size.width() + x;
         destimage[pixelPos].r = qMin(qMax((int) (value * (double) color.red() +
                                                  negVal * (double) destimage[pixelPos].r), 0), 255);
         destimage[pixelPos].g = qMin(qMax((int) (value * (double) color.green() +
                                                  negVal * (double) destimage[pixelPos].g), 0), 255);
         destimage[pixelPos].b = qMin(qMax((int) (value * (double) color.blue() +
                                                  negVal * (double) destimage[pixelPos].b), 0), 255);
         destimage[pixelPos].a = qMin(qMax((int) (value * (double) color.alpha() +
                                                  negVal * (double) destimage[pixelPos].a), (int) destimage[pixelPos].a), 255);
      }
   }
}
