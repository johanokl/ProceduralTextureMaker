/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include <QMap>
#include "glow.h"
#include "transform.h"
#include "stackblur.h"
#include "cutout.h"
#include "merge.h"
#include "fill.h"
#include "modifylevels.h"
#include "setchannels.h"
#include "modifylevels.h"
#include "blending.h"

using namespace std;

GlowTextureGenerator::GlowTextureGenerator()
{
   TextureGeneratorSetting color;
   color.defaultvalue = QVariant(QColor(255, 255, 0, 255));
   color.name = "Color";
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting size;
   size.name = "Size";
   size.defaultvalue = QVariant((double) 2);
   size.min = QVariant((double) 0);
   size.max = QVariant((double) 20);
   size.order = 2;
   configurables.insert("size", size);

   TextureGeneratorSetting blur;
   blur.defaultvalue = QVariant((double) 3);
   blur.name = "Blur level";
   blur.min = QVariant((double) 0);
   blur.max = QVariant((double) 25);
   blur.order = 3;
   configurables.insert("level", blur);

   QStringList modes;
   modes.append("Multiply");
   modes.append("Enlarge");
   TextureGeneratorSetting mode;
   mode.name = "Mode";
   mode.defaultvalue = QVariant(modes);
   configurables.insert("mode", mode);
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

   TransformTextureGenerator transformgen;
   StackBlurTextureGenerator stackblurgen;
   BlendingTextureGenerator blendinggen;
   MergeTextureGenerator mergegen;
   FillTextureGenerator fillgen;
   ModifyLevelsTextureGenerator modifylevelsgen;
   SetChannelsTextureGenerator setchannelsgen;

   double offset = settings->value("size").toDouble();
   QString mode = settings->value("mode").toString();

   TextureNodeSettings settingsForTransform;
   QMapIterator<QString, TextureGeneratorSetting> transformSettingsIterator(transformgen.getSettings());
   while (transformSettingsIterator.hasNext()) {
      transformSettingsIterator.next();
      settingsForTransform.insert(transformSettingsIterator.key(), transformSettingsIterator.value().defaultvalue);
   }
   TexturePixel* mergedImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr mergedImagePtr = TextureImagePtr(new TextureImage(size, mergedImage));

   if (mode == "Multiply") {
      TexturePixel* transformedLeftImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedRightImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedTopImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedDownImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedTopLeftImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedTopRightImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedDownLeftImage = new TexturePixel[size.width() * size.height()];
      TexturePixel* transformedDownRightImage = new TexturePixel[size.width() * size.height()];
      TextureImagePtr transformedLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedLeftImage));
      TextureImagePtr transformedRightImagePtr = TextureImagePtr(new TextureImage(size, transformedRightImage));
      TextureImagePtr transformedTopImagePtr = TextureImagePtr(new TextureImage(size, transformedTopImage));
      TextureImagePtr transformedDownImagePtr = TextureImagePtr(new TextureImage(size, transformedDownImage));
      TextureImagePtr transformedTopLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedTopLeftImage));
      TextureImagePtr transformedTopRightImagePtr = TextureImagePtr(new TextureImage(size, transformedTopRightImage));
      TextureImagePtr transformedDownLeftImagePtr = TextureImagePtr(new TextureImage(size, transformedDownLeftImage));
      TextureImagePtr transformedDownRightImagePtr = TextureImagePtr(new TextureImage(size, transformedDownRightImage));
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

   TexturePixel* filledImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr filledImagePtr = TextureImagePtr(new TextureImage(size, filledImage));
   fillgen.generate(size, filledImage, sourceimages, settings);

   QMap<int, TextureImagePtr> setchannelImages;
   setchannelImages.insert(0, filledImagePtr);
   setchannelImages.insert(1, mergedImagePtr);
   TextureNodeSettings settingsForSetchannels;
   settingsForSetchannels.insert("channelRed", QVariant("First's red"));
   settingsForSetchannels.insert("channelGreen", QVariant("First's green"));
   settingsForSetchannels.insert("channelBlue", QVariant("First's blue"));
   settingsForSetchannels.insert("channelAlpha", QVariant("Second's alpha"));
   TexturePixel* setchannelsImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr setchannelsImagePtr = TextureImagePtr(new TextureImage(size, setchannelsImage));
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, &settingsForSetchannels);

   QMap<int, TextureImagePtr> modifylevelsImages;
   modifylevelsImages.insert(0, setchannelsImagePtr);
   TextureNodeSettings settingsForModifyLevels;
   settingsForModifyLevels.insert("channel", "Only alpha");
   settingsForModifyLevels.insert("mode", "Multiply");
   settingsForModifyLevels.insert("level", 500);
   TexturePixel* modifylevelsImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr modifylevelsImagePtr = TextureImagePtr(new TextureImage(size, modifylevelsImage));
   modifylevelsgen.generate(size, modifylevelsImage, modifylevelsImages, &settingsForModifyLevels);

   QMap<int, TextureImagePtr> firstblurImages;
   firstblurImages.insert(0, modifylevelsImagePtr);
   TextureNodeSettings settingsForFirstBlur;
   settingsForFirstBlur.insert("level", QVariant(settings->value("level").toInt()));
   TexturePixel* firstblurredImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr firstblurredImagePtr = TextureImagePtr(new TextureImage(size, firstblurredImage));
   stackblurgen.generate(size, firstblurredImage, firstblurImages, &settingsForFirstBlur);

   CutoutTextureGenerator cutoutgen;
   QMap<int, TextureImagePtr> cutoutImages;
   cutoutImages.insert(0, firstblurredImagePtr);
   cutoutImages.insert(1, sourceimages.value(0));
   TextureNodeSettings settingsForCutout;
   settingsForCutout.insert("factor", 255);
   TexturePixel* cutoutImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr cutoutImagePtr = TextureImagePtr(new TextureImage(size, cutoutImage));
   cutoutgen.generate(size, cutoutImage, cutoutImages, &settingsForCutout);

   QMap<int, TextureImagePtr> sourceForBlend;
   sourceForBlend.insert(0, sourceimages.value(0));
   sourceForBlend.insert(1, cutoutImagePtr);
   TextureNodeSettings settingsForBlend;
   QMapIterator<QString, TextureGeneratorSetting> blendSettingsIterator(blendinggen.getSettings());
   while (blendSettingsIterator.hasNext()) {
      blendSettingsIterator.next();
      settingsForBlend.insert(blendSettingsIterator.key(), blendSettingsIterator.value().defaultvalue);
   }
   TexturePixel* blendedImage = new TexturePixel[size.width() * size.height()];
   blendinggen.generate(size, blendedImage, sourceForBlend, &settingsForBlend);
   memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
   delete[] blendedImage;
}
