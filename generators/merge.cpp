/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "merge.h"

void MergeTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   Q_UNUSED(settings);
   if (!destimage || !size.isValid()) {
      return;
   }
   int numPixels = size.width() * size.height();
   memset(destimage, 0, numPixels * sizeof(TexturePixel));
   QMapIterator<int, TextureImagePtr> sourceIterator(sourceimages);
   while (sourceIterator.hasNext()) {
      sourceIterator.next();
      TexturePixel* newSource = sourceIterator.value().data()->getData();
      for (int i = 0; i < numPixels; i++) {
         destimage[i] += newSource[i];
      }
   }
}
