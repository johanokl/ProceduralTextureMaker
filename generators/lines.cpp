/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QColor>
#include <math.h>
#include "core/textureimage.h"
#include "lines.h"

using namespace std;

LinesTextureGenerator::LinesTextureGenerator()
{
   TextureGeneratorSetting color;
   color.name = "Line color";
   color.defaultvalue = QVariant(QColor(255, 100, 50, 255));
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting lineheight;
   lineheight.name = "Line height";
   lineheight.defaultvalue = QVariant((int) 10);
   lineheight.min = QVariant((int) 0);
   lineheight.max = QVariant((int) 100);
   lineheight.order = 2;
   configurables.insert("lineheight", lineheight);

   TextureGeneratorSetting distance;
   distance.name = "Distance";
   distance.defaultvalue = QVariant((int) 10);
   distance.min = QVariant((int) 0);
   distance.max = QVariant((int) 100);
   distance.order = 3;
   configurables.insert("distance", distance);

   TextureGeneratorSetting offset;
   offset.name = "Offset";
   offset.defaultvalue = QVariant((int) 0);
   offset.min = QVariant((int) 0);
   offset.max = QVariant((int) 100);
   offset.order = 4;
   configurables.insert("offset", offset);
}


void LinesTextureGenerator::generate(QSize size,
                                     TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = configurables.value("color").defaultvalue.value<QColor>();
   int lineheight = configurables.value("lineheight").defaultvalue.toDouble() * size.height() / 100;
   int distance = configurables.value("distance").defaultvalue.toDouble() * size.height() / 100;
   int offset = configurables.value("offset").defaultvalue.toDouble() * size.height() / 100;
   if (settings != NULL) {
      if (settings->contains("color")) {
         color = settings->value("color").value<QColor>();
      }
      if (settings->contains("distance")) {
         distance = settings->value("distance").toDouble() * size.height() / 100;
      }
      if (settings->contains("lineheight")) {
         lineheight = settings->value("lineheight").toDouble() * size.height() / 100;
      }
      if (settings->contains("offset")) {
         offset = settings->value("offset").toDouble() * size.height() / 100;
      }
   }
   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   TexturePixel filler(color.red(), color.green(), color.blue(), 255);
   for (int y = 0; y < size.height(); y++) {
      if (((y + offset) % (lineheight + distance)) > distance) {
         for (int x = 0; x < size.width(); x++) {
            destimage[y * size.width() + x] = filler;
         }
      }
   }
}
