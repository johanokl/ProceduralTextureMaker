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
#include "greyscale.h"
#include "transform.h"
#include "stackblur.h"
#include "blending.h"

using namespace std;

ShadowTextureGenerator::ShadowTextureGenerator()
{
   TextureGeneratorSetting scaleX;
   scaleX.name = "Scale X (%)";
   scaleX.defaultvalue = QVariant((double) 100);
   scaleX.min = QVariant((double) 0);
   scaleX.max = QVariant((double) 500);
   scaleX.order = 1;
   configurables.insert("xscale", scaleX);

   TextureGeneratorSetting scaleY;
   scaleY.name = "Scale Y (%)";
   scaleY.defaultvalue = QVariant((double) 100);
   scaleY.min = QVariant((double) 0);
   scaleY.max = QVariant((double) 500);
   scaleY.order = 2;
   configurables.insert("yscale", scaleY);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset left";
   offsetLeft.defaultvalue = QVariant((double) -10);
   offsetLeft.min = QVariant((double) -100);
   offsetLeft.max = QVariant((double) 100);
   offsetLeft.order = 3;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset top";
   offsetTop.defaultvalue = QVariant((double) 10);
   offsetTop.min = QVariant((double) -100);
   offsetTop.max = QVariant((double) 100);
   offsetTop.order = 4;
   configurables.insert("offsettop", offsetTop);

   TextureGeneratorSetting blursetting;
   blursetting.defaultvalue = QVariant((int) 10);
   blursetting.name = "Blur level";
   blursetting.min = QVariant((int) 0);
   blursetting.max = QVariant((int) 20);
   blursetting.order = 5;
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
   GreyscaleTextureGenerator greygen;
   TransformTextureGenerator transformgen;
   StackBlurTextureGenerator stackblurgen;
   BlendingTextureGenerator blendinggen;

   TexturePixel* greyImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr greyImagePtr = TextureImagePtr(new TextureImage(size, greyImage));
   greygen.generate(size, greyImage, sourceimages, settings);

   QMap<int, TextureImagePtr> blurSettingsIterator;
   blurSettingsIterator.insert(0, greyImagePtr);
   TextureNodeSettings settingsForBlur;
   settingsForBlur.insert("level", QVariant(settings->value("level").toInt()));
   TexturePixel* blurredImage = new TexturePixel[size.width() * size.height()];
   TextureImagePtr blurredImagePtr = TextureImagePtr(new TextureImage(size, blurredImage));
   stackblurgen.generate(size, blurredImage, blurSettingsIterator, &settingsForBlur);

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
