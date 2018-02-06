/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "greyscale.h"

void GreyscaleTextureGenerator::generate(QSize size,
                                         TexturePixel* destimage,
                                         QMap<int, TextureImagePtr> sourceimages,
                                         TextureNodeSettings* settings) const
{
   Q_UNUSED(settings);

   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(0).data()->getData();

   for (int j = 0; j < size.height(); j++) {
      for (int i = 0; i < size.width(); i++) {
         int pixelpos = j * size.width() + i;
         TexturePixel sourcePixel = sourceImage[pixelpos];
         unsigned char color = sourcePixel.intensity() * 255;
         destimage[pixelpos].r = color;
         destimage[pixelpos].g = color;
         destimage[pixelpos].b = color;
         destimage[pixelpos].a = sourcePixel.a;
      }
   }
}
