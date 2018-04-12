/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "invert.h"

InvertTextureGenerator::InvertTextureGenerator()
{
   QStringList options;
   options.append("Yes");
   options.append("No");

   TextureGeneratorSetting channelRed;
   channelRed.name = "Invert red";
   channelRed.order = 0;
   channelRed.defaultindex = 0;
   channelRed.defaultvalue = QVariant(options);
   configurables.insert("channelRed", channelRed);

   TextureGeneratorSetting channelGreen;
   channelGreen.name = "Invert green";
   channelGreen.order = 1;
   channelGreen.defaultindex = 0;
   channelGreen.defaultvalue = QVariant(options);
   configurables.insert("channelGreen", channelGreen);

   TextureGeneratorSetting channelBlue;
   channelBlue.name = "Invert blue";
   channelBlue.order = 2;
   channelBlue.defaultindex = 0;
   channelBlue.defaultvalue = QVariant(options);
   configurables.insert("channelBlue", channelBlue);

   TextureGeneratorSetting channelAlpha;
   channelAlpha.name = "Invert alpha";
   channelAlpha.order = 3;
   channelAlpha.defaultindex = 1;
   channelAlpha.defaultvalue = QVariant(options);
   configurables.insert("channelAlpha", channelAlpha);
}

void InvertTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      QMap<int, TextureImagePtr> sourceimages,
                                      TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   auto channelRedStr = settings->value("channelRed").toString();
   auto channelGreenStr = settings->value("channelGreen").toString();
   auto channelBlueStr = settings->value("channelBlue").toString();
   auto channelAlphaStr = settings->value("channelAlpha").toString();
   int numPixels = size.width() * size.height();
   TexturePixel* source = nullptr;
   if (sourceimages.contains(0)) {
      source = sourceimages.value(0).data()->getData();
   }
   if (!source) {
      memset(destimage, 0, numPixels * sizeof(TexturePixel));
      return;
   }
   memcpy(destimage, source, numPixels * sizeof(TexturePixel));

   bool channelRedInvert = false;
   bool channelGreenInvert = false;
   bool channelBlueInvert = false;
   bool channelAlphaInvert = false;
   if (channelRedStr == "Yes") {
      channelRedInvert = true;
   }
   if (channelGreenStr == "Yes") {
      channelGreenInvert = true;
   }
   if (channelBlueStr == "Yes") {
      channelBlueInvert = true;
   }
   if (channelAlphaStr == "Yes") {
      channelAlphaInvert = true;
   }
   for (int thisPos = 0; thisPos < numPixels; thisPos++) {
      if (channelRedInvert) {
         destimage[thisPos].r = 255 - destimage[thisPos].r;
      }
      if (channelGreenInvert) {
         destimage[thisPos].g = 255 - destimage[thisPos].g;
      }
      if (channelBlueInvert) {
         destimage[thisPos].b = 255 - destimage[thisPos].b;
      }
      if (channelAlphaInvert) {
         destimage[thisPos].a = 255 - destimage[thisPos].a;
      }
   }
}
