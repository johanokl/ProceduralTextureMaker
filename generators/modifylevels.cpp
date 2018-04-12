/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "modifylevels.h"

ModifyLevelsTextureGenerator::ModifyLevelsTextureGenerator()
{
   TextureGeneratorSetting channel;
   channel.name = "Channels";
   QStringList channels;
   channels.append("All channels");
   channels.append("All colors, not alpha");
   channels.append("Only red");
   channels.append("Only green");
   channels.append("Only blue");
   channels.append("Only alpha");
   channel.description = "";
   channel.order = 0;
   channel.defaultindex = 1;
   channel.defaultvalue = QVariant(channels);
   configurables.insert("channel", channel);

   TextureGeneratorSetting mode;
   mode.name = "Mode";
   QStringList modes;
   modes.append("Multiply");
   modes.append("Add");
   mode.defaultvalue = QVariant(modes);
   mode.order = 1;
   configurables.insert("mode", mode);

   TextureGeneratorSetting blendingAlpha;
   blendingAlpha.name = "Amount";
   blendingAlpha.defaultvalue = QVariant((double) 100);
   blendingAlpha.min = -500;
   blendingAlpha.max = 500;
   blendingAlpha.order = 2;
   configurables.insert("level", blendingAlpha);
}


void ModifyLevelsTextureGenerator::generate(QSize size, TexturePixel* destimage,
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
   QString channel = settings->value("channel").toString();
   double levelFactor = settings->value("level").toDouble()  / 100;
   int levelAbsolute = qMin(settings->value("level").toInt(), 255);

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
            destimage[i].r = qMax(qMin((int) (levelFactor * destimage[i].r), 255), 0);
         }
         if (g) {
            destimage[i].g = qMax(qMin((int) (levelFactor * destimage[i].g), 255), 0);
         }
         if (b) {
            destimage[i].b = qMax(qMin((int) (levelFactor * destimage[i].b), 255), 0);
         }
         if (a) {
            destimage[i].a = qMax(qMin((int) (levelFactor * destimage[i].a), 255), 0);
         }
      }
   }
}


