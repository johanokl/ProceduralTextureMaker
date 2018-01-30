/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include "core/textureimage.h"
#include "boxblur.h"

using namespace std;

BoxBlurTextureGenerator::BoxBlurTextureGenerator()
{
   TextureGeneratorSetting neighbourssetting;
   neighbourssetting.defaultvalue = QVariant((int) 5);
   neighbourssetting.name = "Neighbours";
   neighbourssetting.description = "Number of neighbours";
   neighbourssetting.min = QVariant((int) 1);
   neighbourssetting.max = QVariant((int) 50);
   configurables.insert("numneighbours", neighbourssetting);
}


void BoxBlurTextureGenerator::generate(QSize size,
                             TexturePixel* destimage,
                             QMap<int, TextureImagePtr> sourceimages,
                             TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   int numNeightbours = configurables["numneighbours"].defaultvalue.toInt();
   if (settings != NULL && settings->contains("numneighbours")) {
      numNeightbours = settings->value("numneighbours").toInt();
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(0).data()->getData();

   for (int j = 0; j < size.height(); j++) {
      for (int i = 0; i < size.width(); i++) {
         int startX = i - numNeightbours;
         if (startX < 0) {
            startX = 0;
         }
         int endX = i + numNeightbours;
         if (endX > size.width()) {
            endX = size.width();
         }
         int startY = j - numNeightbours;
         if (startY < 0) {
            startY = 0;
         }
         int endY = j + numNeightbours;
         if (endY > size.height()) {
            endY = size.height();
         }
         int totalPixels = 0;
         int red = 0;
         int green = 0;
         int blue = 0;
         int alpha = 0;
         for (int ypos = startY; ypos < endY; ypos++) {
            int height = ypos * size.width();
            for (int xpos = startX; xpos < endX; xpos++) {
               totalPixels++;
               TexturePixel sourcePixel = sourceImage[height + xpos];
               red += sourcePixel.r;
               green += sourcePixel.g;
               blue += sourcePixel.b;
               alpha += sourcePixel.a;
            }
         }
         int pixelpos = j * size.width() + i;
         destimage[pixelpos].r = (unsigned char) (red / totalPixels);
         destimage[pixelpos].g = (unsigned char) (green / totalPixels);
         destimage[pixelpos].b = (unsigned char) (blue / totalPixels);
         destimage[pixelpos].a = (unsigned char) (alpha / totalPixels);
      }
   }
}
