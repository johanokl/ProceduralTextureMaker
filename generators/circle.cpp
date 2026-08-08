
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "circle.h"
#include <QColor>
#include <cmath>

CircleTextureGenerator::CircleTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Colour";
   colorsetting.description = "Colour used to draw the circle or ring.";
   colorsetting.defaultvalue = QVariant(QColor(200, 100, 0, 255));
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting innerRadius;
   innerRadius.name = "Inner radius (%)";
   innerRadius.description = "Inner radius as a percentage of half the texture height.";
   innerRadius.defaultvalue = QVariant((double)0);
   innerRadius.min = QVariant(0);
   innerRadius.max = QVariant(200);
   innerRadius.id = "innerradius";
   configurables.append(innerRadius);

   TextureGeneratorSetting outerRadius;
   outerRadius.name = "Outer radius (%)";
   outerRadius.description = "Outer radius as a percentage of half the texture height.";
   outerRadius.defaultvalue = QVariant((double)80);
   outerRadius.min = QVariant(0);
   outerRadius.max = QVariant(200);
   outerRadius.id = "outerradius";
   configurables.append(outerRadius);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the centre horizontally as a percentage of the texture width.";
   offsetLeft.defaultvalue = QVariant((double)0);
   offsetLeft.min = QVariant(-100);
   offsetLeft.max = QVariant(100);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the centre vertically as a percentage of the texture height.";
   offsetTop.defaultvalue = QVariant((double)0);
   offsetTop.min = QVariant(-100);
   offsetTop.max = QVariant(100);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);
}
void CircleTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   double innerRadius = settings.value("innerradius").toDouble() * size.height() / 200.0;
   double outerRadius = settings.value("outerradius").toDouble() * size.height() / 200.0;
   int offsetLeft = settings.value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings.value("offsettop").toDouble() * size.height() / 100;

   bool blend = false;
   if (sourceimages.contains(QStringLiteral("Canvas"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Canvas"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
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
         if (((pow(abs(size.width() / 2 - x + offsetLeft), 2) +
               pow(abs(size.height() / 2 - y + offsetTop), 2)) >= (pow(innerRadius, 2))) &&
             ((pow(abs(size.width() / 2 - x + offsetLeft), 2) +
               pow(abs(size.height() / 2 - y + offsetTop), 2)) <= (pow(outerRadius, 2)))) {
            int thisPos = y * size.width() + x;
            destimage[thisPos].r = static_cast<quint8>(
                alpha * color.red() + (blend ? (srcAlpha * destimage[thisPos].r) : 0));
            destimage[thisPos].g = static_cast<quint8>(
                alpha * color.green() + (blend ? (srcAlpha * destimage[thisPos].g) : 0));
            destimage[thisPos].b = static_cast<quint8>(
                alpha * color.blue() + (blend ? (srcAlpha * destimage[thisPos].b) : 0));
            destimage[thisPos].a = static_cast<quint8>(
                color.alpha() + (blend ? (srcAlpha * destimage[thisPos].a) : 0));
         }
      }
   }
}
