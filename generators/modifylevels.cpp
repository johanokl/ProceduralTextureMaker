
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "modifylevels.h"

ModifyLevelsTextureGenerator::ModifyLevelsTextureGenerator() {
   TextureGeneratorSetting channel;
   channel.name = "Channels to adjust";
   QStringList channels;
   channels.append("All channels");
   channels.append("All colors, not alpha");
   channels.append("Only red");
   channels.append("Only green");
   channels.append("Only blue");
   channels.append("Only alpha");
   channel.description = "Selects which colour or alpha channels are modified.";
   channel.defaultindex = 1;
   channel.defaultvalue = QVariant(channels);
   channel.id = "channel";
   configurables.append(channel);

   TextureGeneratorSetting mode;
   mode.name = "Adjustment mode";
   mode.description = "Selects whether each channel is multiplied by or increased by the amount.";
   QStringList modes;
   modes.append("Multiply");
   modes.append("Add");
   mode.defaultvalue = QVariant(modes);
   mode.id = "mode";
   configurables.append(mode);

   TextureGeneratorSetting blendingAlpha;
   blendingAlpha.name = "Adjustment amount";
   blendingAlpha.description =
       "Acts as a percentage in Multiply mode and a channel-value offset in Add mode.";
   blendingAlpha.defaultvalue = QVariant((double)100);
   blendingAlpha.min = -500;
   blendingAlpha.max = 500;
   blendingAlpha.id = "level";
   configurables.append(blendingAlpha);
}
void ModifyLevelsTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                            const QMap<QString, TextureImagePtr>& sourceimages,
                                            const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   int numpixels = size.width() * size.height();
   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, numpixels * sizeof(TexturePixel));
      return;
   }
   memcpy(destimage, sourceimages.value(QStringLiteral("Input"))->getData(),
          numpixels * sizeof(TexturePixel));

   QString mode = settings.value("mode").toString();
   QString channel = settings.value("channel").toString();
   double levelFactor = settings.value("level").toDouble() / 100;
   int levelAbsolute = qMin(settings.value("level").toInt(), 255);

   bool r = false, g = false, b = false, a = false;
   if (channel == "All channels") {
      r = g = b = a = true;
   } else if (channel == "All colors, not alpha") {
      r = g = b = true;
   } else if (channel == "Only red") {
      r = true;
   } else if (channel == "Only green") {
      g = true;
   } else if (channel == "Only blue") {
      b = true;
   } else if (channel == "Only alpha") {
      a = true;
   }

   if (mode == "Add") {
      for (int i = 0; i < numpixels; i++) {
         if (r) {
            destimage[i].r = qMax(qMin(levelAbsolute + destimage[i].r, 255), 0);
         }
         if (g) {
            destimage[i].g = qMax(qMin(levelAbsolute + destimage[i].g, 255), 0);
         }
         if (b) {
            destimage[i].b = qMax(qMin(levelAbsolute + destimage[i].b, 255), 0);
         }
         if (a) {
            destimage[i].a = qMax(qMin(levelAbsolute + destimage[i].a, 255), 0);
         }
      }
   } else if (mode == "Multiply") {
      for (int i = 0; i < numpixels; i++) {
         if (r) {
            destimage[i].r = qMax(qMin((int)(levelFactor * destimage[i].r), 255), 0);
         }
         if (g) {
            destimage[i].g = qMax(qMin((int)(levelFactor * destimage[i].g), 255), 0);
         }
         if (b) {
            destimage[i].b = qMax(qMin((int)(levelFactor * destimage[i].b), 255), 0);
         }
         if (a) {
            destimage[i].a = qMax(qMin((int)(levelFactor * destimage[i].a), 255), 0);
         }
      }
   }
}
