/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "blending.h"
#include "cutout.h"
#include "fill.h"
#include "glow.h"
#include "merge.h"
#include "modifylevels.h"
#include "modifylevels.h"
#include "setchannels.h"
#include "stackblur.h"
#include "transform.h"
#include <QColor>
#include <QMap>
#include <cmath>

GlowTextureGenerator::GlowTextureGenerator()
{
   TextureGeneratorSetting color;
   color.defaultvalue = QVariant(QColor(255, 255, 0, 255));
   color.name = "Color";
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting size;
   size.name = "Size";
   size.defaultvalue = QVariant((double) 4);
   size.min = QVariant(0);
   size.max = QVariant(20);
   size.order = 2;
   configurables.insert("size", size);

   TextureGeneratorSetting firstblur;
   firstblur.defaultvalue = QVariant((double) 3);
   firstblur.name = "First blur";
   firstblur.min = QVariant(0);
   firstblur.max = QVariant(25);
   firstblur.order = 3;
   configurables.insert("firstblurlevel", firstblur);

   QStringList modes;
   modes.append("Multiply");
   modes.append("Enlarge");
   TextureGeneratorSetting mode;
   mode.name = "Mode";
   mode.defaultvalue = QVariant(modes);
   mode.order = 4;
   configurables.insert("mode", mode);

   TextureGeneratorSetting ontop;
   ontop.name = "Glow on top";
   ontop.defaultvalue = QVariant((bool) false);
   ontop.order = 5;
   configurables.insert("ontop", ontop);

   TextureGeneratorSetting cutoutx;
   cutoutx.name = "Cutout X (%)";
   cutoutx.defaultvalue = QVariant((double) 95);
   cutoutx.min = QVariant(0);
   cutoutx.max = QVariant(100);
   cutoutx.group = "cutout";
   cutoutx.order = 6;
   cutoutx.enabler = "ontop";
   configurables.insert("cutoutx", cutoutx);

   TextureGeneratorSetting cutouty;
   cutouty.name = "Cutout Y (%)";
   cutouty.defaultvalue = QVariant((double) 95);
   cutouty.min = QVariant(0);
   cutouty.max = QVariant(100);
   cutouty.group = "cutout";
   cutouty.enabler = "ontop";
   cutouty.order = 7;
   configurables.insert("cutouty", cutouty);

   TextureGeneratorSetting secondblur;
   secondblur.defaultvalue = QVariant((double) 3);
   secondblur.name = "Second blur";
   secondblur.min = QVariant(0);
   secondblur.max = QVariant(25);
   secondblur.enabler = "ontop";
   secondblur.order = 8;
   configurables.insert("secondblurlevel", secondblur);

   TextureGeneratorSetting includesource;
   includesource.name = "Include source";
   includesource.defaultvalue = QVariant((bool) true);
   includesource.order = 9;
   configurables.insert("includesource", includesource);
}

void GlowTextureGenerator::generate(QSize size,
                                    TexturePixel* destimage,
                                    QMap<int, TextureImagePtr> sourceimages,
                                    TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   StackBlurTextureGenerator stackblurgen;

   double offset = settings->value("size").toDouble();
   QString mode = settings->value("mode").toString();

   TransformTextureGenerator transformgen;
   TextureNodeSettings settingsForTransform;
   QMapIterator<QString, TextureGeneratorSetting> transformSettingsIterator(transformgen.getSettings());
   while (transformSettingsIterator.hasNext()) {
      transformSettingsIterator.next();
      settingsForTransform.insert(transformSettingsIterator.key(), transformSettingsIterator.value().defaultvalue);
   }
   auto* mergedImage = new TexturePixel[size.width() * size.height()];
   auto mergedImagePtr = TextureImagePtr(new TextureImage(size, mergedImage));

   if (mode == "Multiply") {
      auto* transformedLeftImage = new TexturePixel[size.width() * size.height()];
      auto* transformedRightImage = new TexturePixel[size.width() * size.height()];
      auto* transformedTopImage = new TexturePixel[size.width() * size.height()];
      auto* transformedDownImage = new TexturePixel[size.width() * size.height()];
      auto* transformedTopLeftImage = new TexturePixel[size.width() * size.height()];
      auto* transformedTopRightImage = new TexturePixel[size.width() * size.height()];
      auto* transformedDownLeftImage = new TexturePixel[size.width() * size.height()];
      auto* transformedDownRightImage = new TexturePixel[size.width() * size.height()];
      auto transformedLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedLeftImage));
      auto transformedRightImagePtr = TextureImagePtr(new TextureImage(size, transformedRightImage));
      auto transformedTopImagePtr = TextureImagePtr(new TextureImage(size, transformedTopImage));
      auto transformedDownImagePtr = TextureImagePtr(new TextureImage(size, transformedDownImage));
      auto transformedTopLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedTopLeftImage));
      auto transformedTopRightImagePtr = TextureImagePtr(new TextureImage(size, transformedTopRightImage));
      auto transformedDownLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedDownLeftImage));
      auto transformedDownRightImagePtr = TextureImagePtr(new TextureImage(size, transformedDownRightImage));
      settingsForTransform.insert("offsetleft", -offset);
      settingsForTransform.insert("offsettop", 0);
      transformgen.generate(size, transformedLeftImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", offset);
      settingsForTransform.insert("offsettop", 0);
      transformgen.generate(size, transformedRightImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", -offset);
      transformgen.generate(size, transformedTopImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", offset);
      transformgen.generate(size, transformedDownImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", -offset * 0.705);
      settingsForTransform.insert("offsettop", -offset * 0.705);
      transformgen.generate(size, transformedTopLeftImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", offset * 0.705);
      settingsForTransform.insert("offsettop", -offset * 0.705);
      transformgen.generate(size, transformedTopRightImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", -offset * 0.705);
      settingsForTransform.insert("offsettop", offset * 0.705);
      transformgen.generate(size, transformedDownLeftImage, sourceimages, &settingsForTransform);
      settingsForTransform.insert("offsetleft", offset * 0.705);
      settingsForTransform.insert("offsettop", offset * 0.705);
      transformgen.generate(size, transformedDownRightImage, sourceimages, &settingsForTransform);

      MergeTextureGenerator mergegen;
      QMap<int, TextureImagePtr> mergeImages;
      mergeImages.insert(0, transformedLeftImagePtr);
      mergeImages.insert(1, transformedRightImagePtr);
      mergeImages.insert(2, transformedTopImagePtr);
      mergeImages.insert(3, transformedDownImagePtr);
      mergeImages.insert(4, transformedTopLeftImagePtr);
      mergeImages.insert(5, transformedTopRightImagePtr);
      mergeImages.insert(6, transformedDownLeftImagePtr);
      mergeImages.insert(7, transformedDownRightImagePtr);
      mergegen.generate(size, mergedImage, mergeImages, settings);
   } else {
      settingsForTransform.insert("xscale", offset * 3 + 100);
      settingsForTransform.insert("yscale", offset * 3 + 100);
      transformgen.generate(size, mergedImage, sourceimages, &settingsForTransform);
   }

   FillTextureGenerator fillgen;
   auto* filledImage = new TexturePixel[size.width() * size.height()];
   auto filledImagePtr = TextureImagePtr(new TextureImage(size, filledImage));
   fillgen.generate(size, filledImage, sourceimages, settings);

   SetChannelsTextureGenerator setchannelsgen;
   QMap<int, TextureImagePtr> setchannelImages;
   setchannelImages.insert(0, filledImagePtr);
   setchannelImages.insert(1, mergedImagePtr);
   TextureNodeSettings settingsForSetchannels;
   settingsForSetchannels.insert("channelRed", QVariant("First's red"));
   settingsForSetchannels.insert("channelGreen", QVariant("First's green"));
   settingsForSetchannels.insert("channelBlue", QVariant("First's blue"));
   settingsForSetchannels.insert("channelAlpha", QVariant("Second's alpha"));
   auto* setchannelsImage = new TexturePixel[size.width() * size.height()];
   auto setchannelsImagePtr = TextureImagePtr(new TextureImage(size, setchannelsImage));
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, &settingsForSetchannels);

   ModifyLevelsTextureGenerator modifylevelsgen;
   QMap<int, TextureImagePtr> modifylevelsImages;
   modifylevelsImages.insert(0, setchannelsImagePtr);
   TextureNodeSettings settingsForModifyLevels;
   settingsForModifyLevels.insert("channel", "Only alpha");
   settingsForModifyLevels.insert("mode", "Multiply");
   settingsForModifyLevels.insert("level", 500);
   auto* modifylevelsImage = new TexturePixel[size.width() * size.height()];
   auto modifylevelsImagePtr = TextureImagePtr(new TextureImage(size, modifylevelsImage));
   modifylevelsgen.generate(size, modifylevelsImage, modifylevelsImages, &settingsForModifyLevels);

   QMap<int, TextureImagePtr> firstblurImages;
   firstblurImages.insert(0, modifylevelsImagePtr);
   TextureNodeSettings settingsForFirstBlur;
   settingsForFirstBlur.insert("level", QVariant(settings->value("firstblurlevel").toInt()));
   auto* firstblurredImage = new TexturePixel[size.width() * size.height()];
   auto firstblurredImagePtr = TextureImagePtr(new TextureImage(size, firstblurredImage));
   stackblurgen.generate(size, firstblurredImage, firstblurImages, &settingsForFirstBlur);

   if (settings->value("ontop").toBool()) {
      auto* smallerCutoutImage = new TexturePixel[size.width() * size.height()];
      auto smallerCutoutImagePtr = TextureImagePtr(new TextureImage(size, smallerCutoutImage));
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", 0);
      double cutoutx = settings->value("cutoutx").toDouble();
      double cutouty = settings->value("cutouty").toDouble();
      settingsForTransform.insert("xscale", cutoutx);
      settingsForTransform.insert("yscale", cutouty);
      transformgen.generate(size, smallerCutoutImage, sourceimages, &settingsForTransform);

      CutoutTextureGenerator cutoutgen;
      QMap<int, TextureImagePtr> cutoutImages;
      cutoutImages.insert(0, firstblurredImagePtr);
      cutoutImages.insert(1, smallerCutoutImagePtr);
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto* cutoutImage = new TexturePixel[size.width() * size.height()];
      auto cutoutImagePtr = TextureImagePtr(new TextureImage(size, cutoutImage));
      cutoutgen.generate(size, cutoutImage, cutoutImages, &settingsForCutout);

      QMap<int, TextureImagePtr> secondblurImages;
      secondblurImages.insert(0, cutoutImagePtr);
      TextureNodeSettings settingsForsecondBlur;
      settingsForsecondBlur.insert("level", QVariant(settings->value("secondblurlevel").toInt()));
      auto* secondblurredImage = new TexturePixel[size.width() * size.height()];
      auto secondblurredImagePtr = TextureImagePtr(new TextureImage(size, secondblurredImage));
      stackblurgen.generate(size, secondblurredImage, secondblurImages, &settingsForsecondBlur);
      firstblurredImagePtr = secondblurredImagePtr;
   } else {
      CutoutTextureGenerator cutoutgen;
      QMap<int, TextureImagePtr> cutoutImages;
      cutoutImages.insert(0, firstblurredImagePtr);
      cutoutImages.insert(1, sourceimages.value(0));
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto* cutoutImage = new TexturePixel[size.width() * size.height()];
      auto cutoutImagePtr = TextureImagePtr(new TextureImage(size, cutoutImage));
      cutoutgen.generate(size, cutoutImage, cutoutImages, &settingsForCutout);
      firstblurredImagePtr = cutoutImagePtr;
   }

   if (settings->value("includesource").toBool()) {
      BlendingTextureGenerator blendinggen;
      QMap<int, TextureImagePtr> sourceForBlend;
      sourceForBlend.insert(0, sourceimages.value(0));
      sourceForBlend.insert(1, firstblurredImagePtr);
      TextureNodeSettings settingsForBlend;
      QMapIterator<QString, TextureGeneratorSetting> blendSettingsIterator(blendinggen.getSettings());
      while (blendSettingsIterator.hasNext()) {
         blendSettingsIterator.next();
         settingsForBlend.insert(blendSettingsIterator.key(), blendSettingsIterator.value().defaultvalue);
      }
      auto* blendedImage = new TexturePixel[size.width() * size.height()];
      blendinggen.generate(size, blendedImage, sourceForBlend, &settingsForBlend);
      memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
      delete[] blendedImage;
   } else {
      memcpy(destimage, firstblurredImagePtr->getData(), size.width() * size.height() * sizeof(TexturePixel));
   }
}
