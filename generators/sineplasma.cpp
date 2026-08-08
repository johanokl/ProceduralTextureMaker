
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "sineplasma.h"
#include <QtMath>
#include <QColor>
#include <cmath>

SinePlasmaTextureGenerator::SinePlasmaTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(255, 0, 0));
   colorsetting.name = "Colour";
   colorsetting.description = "Colour mixed into the generated plasma pattern.";
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting xoffset;
   xoffset.defaultvalue = QVariant((double)10);
   xoffset.name = "Horizontal offset (%)";
   xoffset.description = "Shifts the plasma horizontally across the texture.";
   xoffset.min = QVariant(-100);
   xoffset.max = QVariant(100);
   xoffset.id = "xoffset";
   configurables.append(xoffset);

   TextureGeneratorSetting yoffset;
   yoffset.defaultvalue = QVariant((double)10);
   yoffset.name = "Vertical offset (%)";
   yoffset.description = "Shifts the plasma vertically across the texture.";
   yoffset.min = QVariant(-100);
   yoffset.max = QVariant(100);
   yoffset.id = "yoffset";
   configurables.append(yoffset);

   TextureGeneratorSetting xfrequency;
   xfrequency.defaultvalue = QVariant((double)10);
   xfrequency.name = "Horizontal frequency";
   xfrequency.description = "Controls how often the pattern repeats along the horizontal axis.";
   xfrequency.min = QVariant(0);
   xfrequency.max = QVariant(100);
   xfrequency.group = "frequencies";
   xfrequency.id = "xfrequency";
   configurables.append(xfrequency);

   TextureGeneratorSetting yfrequency;
   yfrequency.defaultvalue = QVariant((double)10);
   yfrequency.name = "Vertical frequency";
   yfrequency.description = "Controls how often the pattern repeats along the vertical axis.";
   yfrequency.min = QVariant(0);
   yfrequency.max = QVariant(100);
   yfrequency.group = "frequencies";
   yfrequency.id = "yfrequency";
   configurables.append(yfrequency);
}
void SinePlasmaTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                          const QMap<QString, TextureImagePtr>& sourceimages,
                                          const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   double xoffset = settings.value("xoffset").toInt() * size.width() / 100;
   double yoffset = settings.value("yoffset").toInt() * size.height() / 100;
   double xfrequency = settings.value("xfrequency").toDouble() * 5 / size.width();
   double yfrequency = settings.value("yfrequency").toDouble() * 5 / size.height();

   if (sourceimages.contains(QStringLiteral("Background"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Background"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double value = 0.5 + 0.25 * qSin((x - xoffset) * xfrequency) +
                        0.25 * qSin((y - yoffset) * yfrequency);
         double negVal = 1 - value;
         int pixelPos = y * size.width() + x;
         destimage[pixelPos].r = qMin(
             qMax((int)(value * (double)color.red() + negVal * (double)destimage[pixelPos].r), 0),
             255);
         destimage[pixelPos].g = qMin(
             qMax((int)(value * (double)color.green() + negVal * (double)destimage[pixelPos].g), 0),
             255);
         destimage[pixelPos].b = qMin(
             qMax((int)(value * (double)color.blue() + negVal * (double)destimage[pixelPos].b), 0),
             255);
         destimage[pixelPos].a = qMin(
             qMax((int)(value * (double)color.alpha() + negVal * (double)destimage[pixelPos].a),
                  (int)destimage[pixelPos].a),
             255);
      }
   }
}
