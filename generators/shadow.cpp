
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "blending.h"
#include "fill.h"
#include "setchannels.h"
#include "shadow.h"
#include "stackblur.h"
#include "transform.h"
#include <QColor>
#include <QMap>
#include <cmath>

ShadowTextureGenerator::ShadowTextureGenerator() {
   TextureGeneratorSetting color;
   color.defaultvalue = QVariant(QColor(100, 100, 100, 255));
   color.name = "Color";
   color.order = 1;
   configurables.insert("color", color);

   TextureGeneratorSetting scaleX;
   scaleX.name = "Scale X (%)";
   scaleX.defaultvalue = QVariant((double)100);
   scaleX.min = QVariant(0);
   scaleX.max = QVariant(500);
   scaleX.order = 2;
   configurables.insert("xscale", scaleX);

   TextureGeneratorSetting scaleY;
   scaleY.name = "Scale Y (%)";
   scaleY.defaultvalue = QVariant((double)100);
   scaleY.min = QVariant(0);
   scaleY.max = QVariant(500);
   scaleY.order = 3;
   configurables.insert("yscale", scaleY);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset left";
   offsetLeft.defaultvalue = QVariant((double)-10);
   offsetLeft.min = QVariant(-100);
   offsetLeft.max = QVariant(100);
   offsetLeft.order = 4;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset top";
   offsetTop.defaultvalue = QVariant((double)10);
   offsetTop.min = QVariant(-100);
   offsetTop.max = QVariant(100);
   offsetTop.order = 5;
   configurables.insert("offsettop", offsetTop);

   TextureGeneratorSetting blursetting;
   blursetting.defaultvalue = QVariant((int)10);
   blursetting.name = "Blur level";
   blursetting.min = QVariant(0);
   blursetting.max = QVariant(20);
   blursetting.order = 6;
   configurables.insert("level", blursetting);
}
void ShadowTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   FillTextureGenerator fillgen;
   auto filledImagePtr = TextureImage::create(size);
   auto* filledImage = filledImagePtr->getData();
   fillgen.generate(size, filledImage, sourceimages, settings);

   SetChannelsTextureGenerator setchannelsgen;
   QMap<QString, TextureImagePtr> setchannelImages;
   setchannelImages.insert(QStringLiteral("First"), filledImagePtr);
   setchannelImages.insert(QStringLiteral("Second"), sourceimages.value(QStringLiteral("Input")));
   TextureNodeSettings settingsForSetchannels;
   settingsForSetchannels.insert("channelRed", QVariant("First's red"));
   settingsForSetchannels.insert("channelGreen", QVariant("First's green"));
   settingsForSetchannels.insert("channelBlue", QVariant("First's blue"));
   settingsForSetchannels.insert("channelAlpha", QVariant("Second's alpha"));
   auto setchannelsImagePtr = TextureImage::create(size);
   auto* setchannelsImage = setchannelsImagePtr->getData();
   setchannelsgen.generate(size, setchannelsImage, setchannelImages, settingsForSetchannels);

   StackBlurTextureGenerator stackblurgen;
   QMap<QString, TextureImagePtr> blurSettingsIterator;
   blurSettingsIterator.insert(QStringLiteral("Input"), setchannelsImagePtr);
   TextureNodeSettings settingsForBlur;
   settingsForBlur.insert("level", QVariant(settings.value("level").toInt()));
   auto blurredImagePtr = TextureImage::create(size);
   auto* blurredImage = blurredImagePtr->getData();
   stackblurgen.generate(size, blurredImage, blurSettingsIterator, settingsForBlur);

   TransformTextureGenerator transformgen;
   QMap<QString, TextureImagePtr> sourceForTransform;
   sourceForTransform.insert(QStringLiteral("Input"), blurredImagePtr);
   TextureNodeSettings settingsForTransform;
   QMapIterator<QString, TextureGeneratorSetting> transformSettingsIterator(
       transformgen.getSettings());
   while (transformSettingsIterator.hasNext()) {
      transformSettingsIterator.next();
      settingsForTransform.insert(transformSettingsIterator.key(),
                                  transformSettingsIterator.value().defaultvalue);
   }
   settingsForTransform.insert("offsetleft", QVariant(settings.value("offsetleft").toDouble()));
   settingsForTransform.insert("offsettop", QVariant(settings.value("offsettop").toDouble()));
   settingsForTransform.insert("xscale", QVariant(settings.value("xscale").toDouble()));
   settingsForTransform.insert("yscale", QVariant(settings.value("yscale").toDouble()));
   auto transformedImagePtr = TextureImage::create(size);
   auto* transformedImage = transformedImagePtr->getData();
   transformgen.generate(size, transformedImage, sourceForTransform, settingsForTransform);

   BlendingTextureGenerator blendinggen;
   QMap<QString, TextureImagePtr> sourceForBlend;
   sourceForBlend.insert(QStringLiteral("Base"), transformedImagePtr);
   sourceForBlend.insert(QStringLiteral("Blend"), sourceimages.value(QStringLiteral("Input")));
   TextureNodeSettings settingsForBlend;
   QMapIterator<QString, TextureGeneratorSetting> blendSettingsIterator(blendinggen.getSettings());
   while (blendSettingsIterator.hasNext()) {
      blendSettingsIterator.next();
      settingsForBlend.insert(blendSettingsIterator.key(),
                              blendSettingsIterator.value().defaultvalue);
   }
   auto* blendedImage = new TexturePixel[size.width() * size.height()];
   blendinggen.generate(size, blendedImage, sourceForBlend, settingsForBlend);
   memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
   delete[] blendedImage;
}
