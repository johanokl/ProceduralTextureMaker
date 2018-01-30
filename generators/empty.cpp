/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QMapIterator>
#include "empty.h"

void EmptyGenerator::generate(QSize size, TexturePixel* destimage,
                              QMap<int, TextureImagePtr> sourceimages,
                              TextureNodeSettings* settings) const
{
   Q_UNUSED(settings);
   Q_UNUSED(sourceimages);
   memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   return;
}
