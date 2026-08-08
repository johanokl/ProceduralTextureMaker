
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "noise.h"
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QRandomGenerator>
#include <QtGlobal>
#include <QtMath>

NoiseTextureGenerator::NoiseTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(0, 0, 0));
   colorsetting.name = "Colour";
   colorsetting.description = "Colour assigned to the generated noise pixels.";
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting alphamin;
   alphamin.defaultvalue = QVariant((int)0);
   alphamin.name = "Minimum alpha";
   alphamin.description = "Lowest alpha value assigned to a generated noise pixel.";
   alphamin.min = QVariant(0);
   alphamin.max = QVariant(255);
   alphamin.id = "alphamin";
   configurables.append(alphamin);

   TextureGeneratorSetting alphamax;
   alphamax.defaultvalue = QVariant((int)255);
   alphamax.name = "Maximum alpha";
   alphamax.description = "Highest alpha value assigned to a generated noise pixel.";
   alphamax.min = QVariant(0);
   alphamax.max = QVariant(255);
   alphamax.id = "alphamax";
   configurables.append(alphamax);

   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((int)300);
   width.name = "Noise width (px)";
   width.description = "Width in pixels of the intermediate noise image before scaling.";
   width.min = QVariant(0);
   width.max = QVariant(1000);
   width.group = "size";
   width.id = "width";
   configurables.append(width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((int)300);
   height.name = "Noise height (px)";
   height.description = "Height in pixels of the intermediate noise image before scaling.";
   height.min = QVariant(0);
   height.max = QVariant(1000);
   height.group = "size";
   height.id = "height";
   configurables.append(height);

   TextureGeneratorSetting scatter;
   scatter.defaultvalue = QVariant((bool)true);
   scatter.name = "Scatter points";
   scatter.description = "Generates isolated noise points instead of filling every pixel.";
   scatter.id = "scatter";
   configurables.append(scatter);

   TextureGeneratorSetting numpoints;
   numpoints.defaultvalue = QVariant((int)10 * 1000);
   numpoints.name = "Point count";
   numpoints.description = "Number of random points generated when scattering is enabled.";
   numpoints.min = QVariant(0);
   numpoints.max = QVariant(1000 * 1000);
   numpoints.enabler = "scatter";
   numpoints.id = "numpoints";
   configurables.append(numpoints);

   TextureGeneratorSetting randomizer;
   randomizer.defaultvalue = QVariant((int)500);
   randomizer.min = QVariant(0);
   randomizer.max = QVariant(1000);
   randomizer.name = "Random seed";
   randomizer.description = "Selects the repeatable random noise pattern.";
   randomizer.id = "randomizer";
   configurables.append(randomizer);

   TextureGeneratorSetting smoothscale;
   smoothscale.defaultvalue = QVariant((bool)false);
   smoothscale.name = "Smooth scaling";
   smoothscale.description =
       "Uses smooth interpolation when resizing the intermediate noise image.";
   smoothscale.id = "smoothscale";
   configurables.append(smoothscale);
}
void NoiseTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                     const QMap<QString, TextureImagePtr>& sourceimages,
                                     const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor color = settings.value("color").value<QColor>();
   int alphamin = settings.value("alphamin").toInt();
   int alphamax = settings.value("alphamax").toInt();
   int randomizer = settings.value("randomizer").toInt();
   int width = settings.value("width").toInt();
   int height = settings.value("height").toInt();
   int numpoints = settings.value("numpoints").toInt();

   if (sourceimages.contains(QStringLiteral("Background"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Background"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   QImage destobject = makeTextureImageView(size, destimage);

   if (alphamax < alphamin) {
      // Switch
      alphamax = alphamax + alphamin;
      alphamin = alphamax - alphamin;
      alphamax = alphamax - alphamin;
   }

   if (width > 0 && height > 0) {
      TexturePixel baseColor(color.red(), color.green(), color.blue(), 255);
      QRandomGenerator random(randomizer);
      TextureImage noiseBuffer(QSize(width, height));
      QImage tempimage = noiseBuffer.toQImageView();
      TexturePixel* bufferImage = noiseBuffer.data();
      if (!settings.value("scatter").toBool()) {
         for (int i = 0; i < width * height; i++) {
            bufferImage[i] = baseColor;
            bufferImage[i].a = random.bounded(alphamin, alphamax + 1);
         }
      } else {
         memset(bufferImage, 0, width * height * sizeof(TexturePixel));
         for (int i = 0; i < numpoints; i++) {
            int index = random.bounded(width * height);
            bufferImage[index] = baseColor;
            bufferImage[index].a = random.bounded(alphamin, alphamax + 1);
         }
      }
      Qt::TransformationMode transformationMode = Qt::FastTransformation;
      if (settings.value("smoothscale").toBool()) {
         transformationMode = Qt::SmoothTransformation;
      }
      tempimage = tempimage.scaled(size, Qt::IgnoreAspectRatio, transformationMode);

      QPainter painter(&destobject);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(QPoint(0, 0), tempimage);
   }
}
