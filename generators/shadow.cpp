/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include <QMap>
#include "shadow.h"
#include "fill.h"
#include "setchannels.h"
#include "transform.h"
#include "stackblur.h"
#include "blending.h"

ShadowTextureGenerator::ShadowTextureGenerator()
{
   TextureGeneratorSetting color;
   color.defaultvalue = QVariant(QColor(100, 100, 100, 255));
   color.name = "Color";
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting scaleX;
   scaleX.name = "Scale X (%)";
   scaleX.defaultvalue = QVariant((double) 100);
   scaleX.min = QVariant((double) 0);
   scaleX.max = QVariant((double) 500);
   scaleX.order = 2;
   configurables.insert("xscale", scaleX);

   TextureGeneratorSetting scaleY;
   scaleY.name = "Scale Y (%)";
   scaleY.defaultvalue = QVariant((double) 100);
   scaleY.min = QVariant((double) 0);
   scaleY.max = QVariant((double) 500);
   scaleY.order = 3;
   configurables.insert("yscale", scaleY);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset left";
   offsetLeft.defaultvalue = QVariant((double) -10);
   offsetLeft.min = QVariant((double) -100);
   offsetLeft.max = QVariant((double) 100);
   offsetLeft.order = 4;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset top";
   offsetTop.defaultvalue = QVariant((double) 10);
   offsetTop.min = QVariant((double) -100);
   offsetTop.max = QVariant((double) 100);
   offsetTop.order = 5;
   configurables.insert("offsettop", offsetTop);

   TextureGeneratorSetting blursetting;
   blursetting.defaultvalue = QVariant((int) 10);
   blursetting.name = "Blur level";
   blursetting.min = QVariant((int) 0);
   blursetting.max = QVariant((int) 20);
   blursetting.order = 6;
   configurables.insert("level", blursetting);
}


void ShadowTextureGenerator::generate(QSize size,
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
   FillTextureGenerator fillgen;
   TexturePixel* filledImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr filledImagePtr = TextureImagePtr(new TextureImage(size, filledImage));
   fillgen.generate(size, filledImage, sourceimages, settings);

   SetChannelsTextureGenerator setchannelsgen;
   QMap<int, TextureImagePtr> setchannelImages;
   setchannelImages.insert(0, filledImagePtr);
   setchannelImages.insert(1, sourceimages.value(0));
   TextureNodeSettings settingsForSetchannels;
   settingsForSetchannels.insert("channelRed", QVariant("First's red"));
   settingsForSetchannels.insert("channelGreen", QVariant("First's green"));
   settingsForSetchannels.insert("channelBlue", QVariant("First's blue"));
   settingsForSetchannels.insert("channelAlpha", QVariant("Second's alpha"));
   TexturePixel* setchannelsImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr setchannelsImagePtr = TextureImagePtr(new TextureImage(size, setchannelsImage));
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, &settingsForSetchannels);

   StackBlurTextureGenerator stackblurgen;
   QMap<int, TextureImagePtr> blurSettingsIterator;
   blurSettingsIterator.insert(0, setchannelsImagePtr);
   TextureNodeSettings settingsForBlur;
   settingsForBlur.insert("level", QVariant(settings->value("level").toInt()));
   TexturePixel* blurredImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr blurredImagePtr = TextureImagePtr(new TextureImage(size, blurredImage));
   stackblurgen.generate(size, blurredImage, blurSettingsIterator, &settingsForBlur);

   TransformTextureGenerator transformgen;
   QMap<int, TextureImagePtr> sourceForTransform;
   sourceForTransform.insert(0, blurredImagePtr);
   TextureNodeSettings settingsForTransform;
   QMapIterator<QString, TextureGeneratorSetting> transformSettingsIterator(transformgen.getSettings());
   while (transformSettingsIterator.hasNext()) {
      transformSettingsIterator.next();
      settingsForTransform.insert(transformSettingsIterator.key(), transformSettingsIterator.value().defaultvalue);
   }
   settingsForTransform.insert("offsetleft", QVariant(settings->value("offsetleft").toDouble()));
   settingsForTransform.insert("offsettop", QVariant(settings->value("offsettop").toDouble()));
   settingsForTransform.insert("xscale", QVariant(settings->value("xscale").toDouble()));
   settingsForTransform.insert("yscale", QVariant(settings->value("yscale").toDouble()));
   TexturePixel* transformedImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr transformedImagePtr = TextureImagePtr(new TextureImage(size, transformedImage));
   transformgen.generate(size, transformedImage, sourceForTransform, &settingsForTransform);

   BlendingTextureGenerator blendinggen;
   QMap<int, TextureImagePtr> sourceForBlend;
   sourceForBlend.insert(0, transformedImagePtr);
   sourceForBlend.insert(1, sourceimages.value(0));
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
