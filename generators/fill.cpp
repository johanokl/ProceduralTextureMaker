/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QColor>
#include "core/textureimage.h"
#include "fill.h"

using namespace std;

FillTextureGenerator::FillTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(255, 255, 255, 255));
   colorsetting.name = "Color";
   colorsetting.description = "Color to fill the texture with.";
   configurables.insert("color", colorsetting);
}


void FillTextureGenerator::generate(QSize size, TexturePixel* destimage,
                           QMap<int, TextureImagePtr> sourceimages,
                           TextureNodeSettings* settings) const
{
   Q_UNUSED(sourceimages);

   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   const int numpixels = size.width() * size.height();
   TexturePixel filler(color.red(), color.green(), color.blue(), color.alpha());
   for (int i = 0; i < numpixels; i++) {
      destimage[i] = filler;
   }
}


