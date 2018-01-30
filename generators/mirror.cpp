/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include <QVector3D>
#include "mirror.h"
#include "core/textureimage.h"

using namespace std;

MirrorTextureGenerator::MirrorTextureGenerator()
{
   QStringList directions;
   directions.append("Flip horizentally");
   directions.append("Flip vertically");
   directions.append("Mirror horizentally");
   directions.append("Mirror vertically");
   TextureGeneratorSetting direction;
   direction.name = "Direction";
   direction.description = "Which way?";
   direction.defaultvalue = QVariant(directions);
   configurables.insert("direction", direction);
}


void MirrorTextureGenerator::generate(QSize size,
                                      TexturePixel* destimage,
                                      QMap<int, TextureImagePtr> sourceimages,
                                      TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   QString direction = configurables["direction"].defaultvalue.toStringList().takeFirst();
   if (settings->contains("direction") && !settings->value("direction").toString().isEmpty()) {
      direction = settings->value("direction").toString();
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 255, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(0)->getData();

   if (direction == "Flip horizentally" || direction == "Flip vertically") {
      for (int y = 0; y < size.height(); y++) {
         bool horizontal = (direction == "Flip horizentally") ? true : false;
         int destrowstart = y * size.width();
         if (horizontal) {
            for (int x = 0; x < size.width(); x++) {
               int sourcepos = destrowstart + size.width() - x;
               destimage[destrowstart + x] = sourceImage[sourcepos];
            }
         } else {
            int sourcerowstart = (size.height() - y) * size.width();
            for (int x = 0; x < size.width(); x++) {
               destimage[destrowstart + x] = sourceImage[sourcerowstart + x];
            }
         }
      }
   } else if (size.width() % 2 == 0) {
      if (direction == "Mirror horizentally") {
         for (int y = 0; y < size.height(); y++) {
            int destrowstart = y * size.width();
            for (int x = 0; x < size.width(); x+=2) {
               TexturePixel color;
               color.r = (int) (sourceImage[destrowstart + x].r + sourceImage[destrowstart + x + 1].r) / 2;
               color.g = (int) (sourceImage[destrowstart + x].g + sourceImage[destrowstart + x + 1].g) / 2;
               color.b = (int) (sourceImage[destrowstart + x].b + sourceImage[destrowstart + x + 1].b) / 2;
               color.a = (int) (sourceImage[destrowstart + x].a + sourceImage[destrowstart + x + 1].a) / 2;
               destimage[destrowstart + x / 2] = color;
               destimage[destrowstart + size.width() - x / 2] = color;
            }
         }
      } else if (direction == "Mirror vertically") {
         int height = size.height();
         int width = size.width();
         for (int x = 0; x < size.width(); x++) {
            for (int y = 0; y < size.height(); y+=2) {
               TexturePixel color;
               color.r = (int) (sourceImage[y * width + x].r + sourceImage[(y + 1) * width + x].r) / 2;
               color.g = (int) (sourceImage[y * width + x].g + sourceImage[(y + 1) * width + x].g) / 2;
               color.b = (int) (sourceImage[y * width + x].b + sourceImage[(y + 1) * width + x].b) / 2;
               color.a = (int) (sourceImage[y * width + x].a + sourceImage[(y + 1) * width + x].a) / 2;
               destimage[y * width / 2 + x] = color;
               destimage[(height - y / 2) * width + x] = color;
            }
         }
      }
   }
}
