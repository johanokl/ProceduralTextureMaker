
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
   color.name = "Glow colour";
   color.description = "Colour used for the generated glow.";
   color.id = "color";
   configurables.append(color);

   TextureGeneratorSetting size;
   size.name = "Glow size (%)";
   size.description = "Controls how far the glow expands beyond the source shape.";
   size.defaultvalue = QVariant((double)4);
   size.min = QVariant(0);
   size.max = QVariant(20);
   size.id = "size";
   configurables.append(size);

   TextureGeneratorSetting firstblur;
   firstblur.defaultvalue = QVariant((double)3);
   firstblur.name = "Outer blur radius (px)";
   firstblur.description = "Softens the expanded outline before it is composited.";
   firstblur.min = QVariant(0);
   firstblur.max = QVariant(25);
   firstblur.id = "firstblurlevel";
   configurables.append(firstblur);

   QStringList modes;
   modes.append("Multiply");
   modes.append("Enlarge");
   TextureGeneratorSetting mode;
   mode.name = "Expansion mode";
   mode.description = "Selects whether the outline is expanded by copies or by scaling.";
   mode.defaultvalue = QVariant(modes);
   mode.id = "mode";
   configurables.append(mode);

   TextureGeneratorSetting ontop;
   ontop.name = "Glow on top";
   ontop.description = "Allows the glow to extend over the opaque area of the source image.";
   ontop.defaultvalue = QVariant((bool)false);
   ontop.id = "ontop";
   configurables.append(ontop);

   TextureGeneratorSetting cutoutx;
   cutoutx.name = "Cut-out width (%)";
   cutoutx.description = "Width of the inner area removed when the glow is drawn on top.";
   cutoutx.defaultvalue = QVariant((double)95);
   cutoutx.min = QVariant(0);
   cutoutx.max = QVariant(100);
   cutoutx.group = "cutout";
   cutoutx.enabler = "ontop";
   cutoutx.id = "cutoutx";
   configurables.append(cutoutx);

   TextureGeneratorSetting cutouty;
   cutouty.name = "Cut-out height (%)";
   cutouty.description = "Height of the inner area removed when the glow is drawn on top.";
   cutouty.defaultvalue = QVariant((double)95);
   cutouty.min = QVariant(0);
   cutouty.max = QVariant(100);
   cutouty.group = "cutout";
   cutouty.enabler = "ontop";
   cutouty.id = "cutouty";
   configurables.append(cutouty);

   TextureGeneratorSetting secondblur;
   secondblur.defaultvalue = QVariant((double)3);
   secondblur.name = "Inner blur radius (px)";
   secondblur.description = "Softens the cut-out edge when the glow is drawn on top.";
   secondblur.min = QVariant(0);
   secondblur.max = QVariant(25);
   secondblur.enabler = "ontop";
   secondblur.id = "secondblurlevel";
   configurables.append(secondblur);

   TextureGeneratorSetting includesource;
   includesource.name = "Include source image";
   includesource.description = "Composites the original input image with the generated glow.";
   includesource.defaultvalue = QVariant((bool)true);
   includesource.id = "includesource";
   configurables.append(includesource);
}
void GlowTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                    const QMap<QString, TextureImagePtr>& sourceimages,
                                    const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Image"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   StackBlurTextureGenerator stackblurgen;

   double offset = settings.value("size").toDouble();
   QString mode = settings.value("mode").toString();

   TransformTextureGenerator transformgen;
   TextureNodeSettings settingsForTransform;
   for (const TextureGeneratorSetting& setting : transformgen.getSettings()) {
      settingsForTransform.insert(setting.id, setting.defaultvalue);
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
      transformgen.generate(size, transformedLeftImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", offset);
      settingsForTransform.insert("offsettop", 0);
      transformgen.generate(size, transformedRightImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", -offset);
      transformgen.generate(size, transformedTopImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", offset);
      transformgen.generate(size, transformedDownImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", -offset * 0.705);
      settingsForTransform.insert("offsettop", -offset * 0.705);
      transformgen.generate(size, transformedTopLeftImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", offset * 0.705);
      settingsForTransform.insert("offsettop", -offset * 0.705);
      transformgen.generate(size, transformedTopRightImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", -offset * 0.705);
      settingsForTransform.insert("offsettop", offset * 0.705);
      transformgen.generate(size, transformedDownLeftImage, sourceimages, settingsForTransform);
      settingsForTransform.insert("offsetleft", offset * 0.705);
      settingsForTransform.insert("offsettop", offset * 0.705);
      transformgen.generate(size, transformedDownRightImage, sourceimages, settingsForTransform);

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
      transformgen.generate(size, mergedImage, sourceimages, settingsForTransform);
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
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, settingsForSetchannels);

   ModifyLevelsTextureGenerator modifylevelsgen;
   QMap<QString, TextureImagePtr> modifylevelsImages;
   modifylevelsImages.insert(QStringLiteral("Image"), setchannelsImagePtr);
   TextureNodeSettings settingsForModifyLevels;
   settingsForModifyLevels.insert("channel", "Only alpha");
   settingsForModifyLevels.insert("mode", "Multiply");
   settingsForModifyLevels.insert("level", 500);
   auto modifylevelsImagePtr = TextureImage::create(size);
   auto* modifylevelsImage = modifylevelsImagePtr->getData();
   modifylevelsgen.generate(size, modifylevelsImage, modifylevelsImages, settingsForModifyLevels);

   QMap<QString, TextureImagePtr> firstblurImages;
   firstblurImages.insert(QStringLiteral("Image"), modifylevelsImagePtr);
   TextureNodeSettings settingsForFirstBlur;
   settingsForFirstBlur.insert("level", QVariant(settings.value("firstblurlevel").toInt()));
   auto firstblurredImagePtr = TextureImage::create(size);
   auto* firstblurredImage = firstblurredImagePtr->getData();
   stackblurgen.generate(size, firstblurredImage, firstblurImages, settingsForFirstBlur);

   if (settings.value("ontop").toBool()) {
      auto smallerCutoutImagePtr = TextureImage::create(size);
      auto* smallerCutoutImage = smallerCutoutImagePtr->getData();
      settingsForTransform.insert("offsetleft", 0);
      settingsForTransform.insert("offsettop", 0);
      double cutoutx = settings.value("cutoutx").toDouble();
      double cutouty = settings.value("cutouty").toDouble();
      settingsForTransform.insert("xscale", cutoutx);
      settingsForTransform.insert("yscale", cutouty);
      transformgen.generate(size, smallerCutoutImage, sourceimages, settingsForTransform);

      CutoutTextureGenerator cutoutgen;
      QMap<QString, TextureImagePtr> cutoutImages;
      cutoutImages.insert(QStringLiteral("Image"), firstblurredImagePtr);
      cutoutImages.insert(QStringLiteral("Mask"), smallerCutoutImagePtr);
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto cutoutImagePtr = TextureImage::create(size);
      auto* cutoutImage = cutoutImagePtr->getData();
      cutoutgen.generate(size, cutoutImage, cutoutImages, settingsForCutout);

      QMap<QString, TextureImagePtr> secondblurImages;
      secondblurImages.insert(QStringLiteral("Image"), cutoutImagePtr);
      TextureNodeSettings settingsForsecondBlur;
      settingsForsecondBlur.insert("level", QVariant(settings.value("secondblurlevel").toInt()));
      auto secondblurredImagePtr = TextureImage::create(size);
      auto* secondblurredImage = secondblurredImagePtr->getData();
      stackblurgen.generate(size, secondblurredImage, secondblurImages, settingsForsecondBlur);
      firstblurredImagePtr = secondblurredImagePtr;
   } else {
      CutoutTextureGenerator cutoutgen;
      QMap<QString, TextureImagePtr> cutoutImages;
      cutoutImages.insert(QStringLiteral("Image"), firstblurredImagePtr);
      cutoutImages.insert(QStringLiteral("Mask"), sourceimages.value(QStringLiteral("Image")));
      TextureNodeSettings settingsForCutout;
      settingsForCutout.insert("factor", 255);
      auto cutoutImagePtr = TextureImage::create(size);
      auto* cutoutImage = cutoutImagePtr->getData();
      cutoutgen.generate(size, cutoutImage, cutoutImages, settingsForCutout);
      firstblurredImagePtr = cutoutImagePtr;
   }

   if (settings.value("includesource").toBool()) {
      BlendingTextureGenerator blendinggen;
      QMap<QString, TextureImagePtr> sourceForBlend;
      sourceForBlend.insert(QStringLiteral("Base"), sourceimages.value(QStringLiteral("Image")));
      sourceForBlend.insert(QStringLiteral("Blend"), firstblurredImagePtr);
      TextureNodeSettings settingsForBlend;
      for (const TextureGeneratorSetting& setting : blendinggen.getSettings()) {
         settingsForBlend.insert(setting.id, setting.defaultvalue);
      }
      auto* blendedImage = new TexturePixel[size.width() * size.height()];
      blendinggen.generate(size, blendedImage, sourceForBlend, settingsForBlend);
      memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
      delete[] blendedImage;
   } else {
      memcpy(destimage, firstblurredImagePtr->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   }
}
