
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "invert.h"

InvertTextureGenerator::InvertTextureGenerator() {
   QStringList options;
   options.append("Yes");
   options.append("No");

   TextureGeneratorSetting channelRed;
   channelRed.name = "Invert red";
   channelRed.description = "Selects whether the red channel is inverted.";
   channelRed.defaultindex = 0;
   channelRed.defaultvalue = QVariant(options);
   channelRed.id = "channelRed";
   configurables.append(channelRed);

   TextureGeneratorSetting channelGreen;
   channelGreen.name = "Invert green";
   channelGreen.description = "Selects whether the green channel is inverted.";
   channelGreen.defaultindex = 0;
   channelGreen.defaultvalue = QVariant(options);
   channelGreen.id = "channelGreen";
   configurables.append(channelGreen);

   TextureGeneratorSetting channelBlue;
   channelBlue.name = "Invert blue";
   channelBlue.description = "Selects whether the blue channel is inverted.";
   channelBlue.defaultindex = 0;
   channelBlue.defaultvalue = QVariant(options);
   channelBlue.id = "channelBlue";
   configurables.append(channelBlue);

   TextureGeneratorSetting channelAlpha;
   channelAlpha.name = "Invert alpha";
   channelAlpha.description = "Selects whether the alpha channel is inverted.";
   channelAlpha.defaultindex = 1;
   channelAlpha.defaultvalue = QVariant(options);
   channelAlpha.id = "channelAlpha";
   configurables.append(channelAlpha);
}
void InvertTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   auto channelRedStr = settings.value("channelRed").toString();
   auto channelGreenStr = settings.value("channelGreen").toString();
   auto channelBlueStr = settings.value("channelBlue").toString();
   auto channelAlphaStr = settings.value("channelAlpha").toString();
   int numPixels = size.width() * size.height();
   TexturePixel* source = nullptr;
   if (sourceimages.contains(QStringLiteral("Image"))) {
      source = sourceimages.value(QStringLiteral("Image")).data()->getData();
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
