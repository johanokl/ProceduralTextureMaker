/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "modifyalpha.h"

using namespace std;

ModifyAlphaTextureGenerator::ModifyAlphaTextureGenerator()
{
   QStringList modes;
   modes.append("Multiply");
   modes.append("Add");
   modes.append("Subtract");
   TextureGeneratorSetting mode;
   mode.name = "mode";
   mode.description = "";
   mode.defaultvalue = QVariant(modes);
   configurables.insert("mode", mode);

   TextureGeneratorSetting blendingAlpha;
   blendingAlpha.defaultvalue = QVariant((double) 100);
   blendingAlpha.name = "Alpha level (%)";
   blendingAlpha.min = 0;
   blendingAlpha.max = 300;
   blendingAlpha.description = "Alpha value of the blending (0-100)";
   configurables.insert("level", blendingAlpha);
}


void ModifyAlphaTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                           QMap<int, TextureImagePtr> sourceimages,
                                           TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   int numpixels = size.width() * size.height();
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, numpixels * sizeof(TexturePixel));
      return;
   }
   memcpy(destimage, sourceimages.value(0)->getData(), numpixels * sizeof(TexturePixel));

   QString mode = settings->value("mode").toString();
   double levelFactor = settings->value("level").toDouble()  / 100;
   int levelAbsolute = qMin(settings->value("level").toInt(), 255);

   for (int i = 0; i < numpixels; i++) {
      if (mode == "Add") {
         destimage[i].a = qMax(qMin(levelAbsolute + destimage[i].a, 255), 0);
      } else if (mode == "Subtract") {
         destimage[i].a = qMax((int) destimage[i].a - levelAbsolute, 0);
      } else if (mode == "Multiply") {
         destimage[i].a = qMin((int) (levelFactor * destimage[i].a), 255);
      }
   }
}


