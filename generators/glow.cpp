
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

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

GlowTextureGenerator::GlowTextureGenerator() {
   TextureGeneratorSetting color;
   color.defaultvalue = QVariant(QColor(255, 255, 0, 255));
   color.name = "Color";
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting size;
   size.name = "Size";
   size.defaultvalue = QVariant((double)4);
   size.min = QVariant(0);
   size.max = QVariant(20);
   size.order = 2;
   configurables.insert("size", size);

   TextureGeneratorSetting firstblur;
   firstblur.defaultvalue = QVariant((double)3);
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
   ontop.defaultvalue = QVariant((bool)false);
   ontop.order = 5;
   configurables.insert("ontop", ontop);

   TextureGeneratorSetting cutoutx;
   cutoutx.name = "Cutout X (%)";
   cutoutx.defaultvalue = QVariant((double)95);
   cutoutx.min = QVariant(0);
   cutoutx.max = QVariant(100);
   cutoutx.group = "cutout";
   cutoutx.order = 6;
   cutoutx.enabler = "ontop";
   configurables.insert("cutoutx", cutoutx);

   TextureGeneratorSetting cutouty;
   cutouty.name = "Cutout Y (%)";
   cutouty.defaultvalue = QVariant((double)95);
   cutouty.min = QVariant(0);
   cutouty.max = QVariant(100);
   cutouty.group = "cutout";
   cutouty.enabler = "ontop";
   cutouty.order = 7;
   configurables.insert("cutouty", cutouty);

   TextureGeneratorSetting secondblur;
   secondblur.defaultvalue = QVariant((double)3);
   secondblur.name = "Second blur";
   secondblur.min = QVariant(0);
   secondblur.max = QVariant(25);
   secondblur.enabler = "ontop";
   secondblur.order = 8;
   configurables.insert("secondblurlevel", secondblur);

   TextureGeneratorSetting includesource;
   includesource.name = "Include source";
   includesource.defaultvalue = QVariant((bool)true);
   includesource.order = 9;
   configurables.insert("includesource", includesource);
}
void GlowTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                    QMap<QString, TextureImagePtr> sourceimages,
                                    TextureNodeSettings* settings) const {
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   StackBlurTextureGenerator stackblurgen;

   double offset = settings->value("size").toDouble();
   QString mode = settings->value("mode").toString();

   TransformTextureGenerator transformgen;
   TextureNodeSettings settingsForTransform;
   QMapIterator<QString, TextureGeneratorSetting> transformSettingsIterator(
       transformgen.getSettings());
   while (transformSettingsIterator.hasNext()) {
      transformSettingsIterator.next();
      settingsForTransform.insert(transformSettingsIterator.key(),
                                  transformSettingsIterator.value().defaultvalue);
   }
   auto mergedImagePtr = TextureImage::create(size);
   auto* mergedImage = mergedImagePtr->getData();

   if (mode == "Multiply") {
      auto transformedLeftImagePtr = TextureImage::create(size);
      auto transformedRightImagePtr = TextureImage::create(size);
      auto transformedTopImagePtr = TextureImage::create(size);
      auto transformedDownImagePtr = TextureImage::create(size);
      auto transformedTopLeftImagePtr = TextureImage::create(size);
      auto transformedTopRightImagePtr = TextureImage::create(size);
      auto transformedDownLeftImagePtr = TextureImage::create(size);
      auto transformedDownRightImagePtr = TextureImage::create(size);
      auto* transformedLeftImage = transformedLeftImagePtr->getData();
      auto* transformedRightImage = transformedRightImagePtr->getData();
      auto* transformedTopImage = transformedTopImagePtr->getData();
      auto* transformedDownImage = transformedDownImagePtr->getData();
      auto* transformedTopLeftImage = transformedTopLeftImagePtr->getData();
      auto* transformedTopRightImage = transformedTopRightImagePtr->getData();
      auto* transformedDownLeftImage = transformedDownLeftImagePtr->getData();
      auto* transformedDownRightImage = transformedDownRightImagePtr->getData();
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
      QMap<QString, TextureImagePtr> mergeImages;
      mergeImages.insert(QStringLiteral("Layer 1"), transformedLeftImagePtr);
      mergeImages.insert(QStringLiteral("Layer 2"), transformedRightImagePtr);
      mergeImages.insert(QStringLiteral("Layer 3"), transformedTopImagePtr);
      mergeImages.insert(QStringLiteral("Layer 4"), transformedDownImagePtr);
      mergeImages.insert(QStringLiteral("Layer 5"), transformedTopLeftImagePtr);
      mergeImages.insert(QStringLiteral("Layer 6"), transformedTopRightImagePtr);
      mergeImages.insert(QStringLiteral("Layer 7"), transformedDownLeftImagePtr);
      mergeImages.insert(QStringLiteral("Layer 8"), transformedDownRightImagePtr);
      mergegen.generate(size, mergedImage, mergeImages, settings);
   } else {
      settingsForTransform.insert("xscale", offset * 3 + 100);
      settingsForTransform.insert("yscale", offset * 3 + 100);
      transformgen.generate(size, mergedImage, sourceimages, &settingsForTransform);
   }

   FillTextureGenerator fillgen;
   auto filledImagePtr = TextureImage::create(size);
   auto* filledImage = filledImagePtr->getData();
   fillgen.generate(size, filledImage, sourceimages, settings);

   SetChannelsTextureGenerator setchannelsgen;
   QMap<QString, TextureImagePtr> setchannelImages;
   setchannelImages.insert(QStringLiteral("First"), filledImagePtr);
   setchannelImages.insert(QStringLiteral("Second"), mergedImagePtr);
   TextureNodeSettings settingsForSetchannels;
   settingsForSetchannels.insert("channelRed", QVariant("First's red"));
   settingsForSetchannels.insert("channelGreen", QVariant("First's green"));
   settingsForSetchannels.insert("channelBlue", QVariant("First's blue"));
   settingsForSetchannels.insert("channelAlpha", QVariant("Second's alpha"));
   auto setchannelsImagePtr = TextureImage::create(size);
   auto* setchannelsImage = setchannelsImagePtr->getData();
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, &settingsForSetchannels);

   ModifyLevelsTextureGenerator modifylevelsgen;
   QMap<QString, TextureImagePtr> modifylevelsImages;
   modifylevelsImages.insert(QStringLiteral("Input"), setchannelsImagePtr);
   TextureNodeSettings settingsForModifyLevels;
   settingsForModifyLevels.insert("channel", "Only alpha");
   settingsForModifyLevels.insert("mode", "Multiply");
   settingsForModifyLevels.insert("level", 500);
   auto modifylevelsImagePtr = TextureImage::create(size);
   auto* modifylevelsImage = modifylevelsImagePtr->getData();
   modifylevelsgen.generate(size, modifylevelsImage, modifylevelsImages, &settingsForModifyLevels);

   QMap<QString, TextureImagePtr> firstblurImages;
   firstblurImages.insert(QStringLiteral("Input"), modifylevelsImagePtr);
   TextureNodeSettings settingsForFirstBlur;
   settingsForFirstBlur.insert("level", QVariant(settings->value("firstblurlevel").toInt()));
   auto firstblurredImagePtr = TextureImage::create(size);
   auto* firstblurredImage = firstblurredImagePtr->getData();
   stackblurgen.generate(size, firstblurredImage, firstblurImages, &settingsForFirstBlur);

   if (settings->value("ontop").toBool()) {
      auto smallerCutoutImagePtr = TextureImage::create(size);
      auto* smallerCutoutImage = smallerCutoutImagePtr->getData();
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", 0);
      double cutoutx = settings->value("cutoutx").toDouble();
      double cutouty = settings->value("cutouty").toDouble();
      settingsForTransform.insert("xscale", cutoutx);
      settingsForTransform.insert("yscale", cutouty);
      transformgen.generate(size, smallerCutoutImage, sourceimages, &settingsForTransform);

      CutoutTextureGenerator cutoutgen;
      QMap<QString, TextureImagePtr> cutoutImages;
      cutoutImages.insert(QStringLiteral("Image"), firstblurredImagePtr);
      cutoutImages.insert(QStringLiteral("Mask"), smallerCutoutImagePtr);
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto cutoutImagePtr = TextureImage::create(size);
      auto* cutoutImage = cutoutImagePtr->getData();
      cutoutgen.generate(size, cutoutImage, cutoutImages, &settingsForCutout);

      QMap<QString, TextureImagePtr> secondblurImages;
      secondblurImages.insert(QStringLiteral("Input"), cutoutImagePtr);
      TextureNodeSettings settingsForsecondBlur;
      settingsForsecondBlur.insert("level", QVariant(settings->value("secondblurlevel").toInt()));
      auto secondblurredImagePtr = TextureImage::create(size);
      auto* secondblurredImage = secondblurredImagePtr->getData();
      stackblurgen.generate(size, secondblurredImage, secondblurImages, &settingsForsecondBlur);
      firstblurredImagePtr = secondblurredImagePtr;
   } else {
      CutoutTextureGenerator cutoutgen;
      QMap<QString, TextureImagePtr> cutoutImages;
      cutoutImages.insert(QStringLiteral("Image"), firstblurredImagePtr);
      cutoutImages.insert(QStringLiteral("Mask"), sourceimages.value(QStringLiteral("Input")));
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto cutoutImagePtr = TextureImage::create(size);
      auto* cutoutImage = cutoutImagePtr->getData();
      cutoutgen.generate(size, cutoutImage, cutoutImages, &settingsForCutout);
      firstblurredImagePtr = cutoutImagePtr;
   }

   if (settings->value("includesource").toBool()) {
      BlendingTextureGenerator blendinggen;
      QMap<QString, TextureImagePtr> sourceForBlend;
      sourceForBlend.insert(QStringLiteral("Base"), sourceimages.value(QStringLiteral("Input")));
      sourceForBlend.insert(QStringLiteral("Blend"), firstblurredImagePtr);
      TextureNodeSettings settingsForBlend;
      QMapIterator<QString, TextureGeneratorSetting> blendSettingsIterator(
          blendinggen.getSettings());
      while (blendSettingsIterator.hasNext()) {
         blendSettingsIterator.next();
         settingsForBlend.insert(blendSettingsIterator.key(),
                                 blendSettingsIterator.value().defaultvalue);
      }
      auto* blendedImage = new TexturePixel[size.width() * size.height()];
      blendinggen.generate(size, blendedImage, sourceForBlend, &settingsForBlend);
      memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
      delete[] blendedImage;
   } else {
      memcpy(destimage, firstblurredImagePtr->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   }
}
