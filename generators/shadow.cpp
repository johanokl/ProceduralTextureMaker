
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
   color.name = "Shadow colour";
   color.description = "Colour used for the generated shadow.";
   color.id = "color";
   configurables.append(color);

   TextureGeneratorSetting scaleX;
   scaleX.name = "Horizontal scale (%)";
   scaleX.description = "Scales the shadow horizontally relative to the input image.";
   scaleX.defaultvalue = QVariant((double)100);
   scaleX.min = QVariant(0);
   scaleX.max = QVariant(500);
   scaleX.id = "xscale";
   configurables.append(scaleX);

   TextureGeneratorSetting scaleY;
   scaleY.name = "Vertical scale (%)";
   scaleY.description = "Scales the shadow vertically relative to the input image.";
   scaleY.defaultvalue = QVariant((double)100);
   scaleY.min = QVariant(0);
   scaleY.max = QVariant(500);
   scaleY.id = "yscale";
   configurables.append(scaleY);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the shadow horizontally across the texture.";
   offsetLeft.defaultvalue = QVariant((double)-10);
   offsetLeft.min = QVariant(-100);
   offsetLeft.max = QVariant(100);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the shadow vertically across the texture.";
   offsetTop.defaultvalue = QVariant((double)10);
   offsetTop.min = QVariant(-100);
   offsetTop.max = QVariant(100);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);

   TextureGeneratorSetting blursetting;
   blursetting.defaultvalue = QVariant((int)10);
   blursetting.name = "Blur radius (px)";
   blursetting.description = "Controls how softly the shadow fades at its edges.";
   blursetting.min = QVariant(0);
   blursetting.max = QVariant(20);
   blursetting.id = "level";
   configurables.append(blursetting);
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
   for (const TextureGeneratorSetting& setting : transformgen.getSettings()) {
      settingsForTransform.insert(setting.id, setting.defaultvalue);
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
   for (const TextureGeneratorSetting& setting : blendinggen.getSettings()) {
      settingsForBlend.insert(setting.id, setting.defaultvalue);
   }
   auto* blendedImage = new TexturePixel[size.width() * size.height()];
   blendinggen.generate(size, blendedImage, sourceForBlend, settingsForBlend);
   memcpy(destimage, blendedImage, size.width() * size.height() * sizeof(TexturePixel));
   delete[] blendedImage;
}
