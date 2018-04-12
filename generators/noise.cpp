/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "noise.h"
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QtGlobal>
#include <QtMath>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

NoiseTextureGenerator::NoiseTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(0, 0, 0));
   colorsetting.name = "Color";
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting alphamin;
   alphamin.defaultvalue = QVariant((int) 0);
   alphamin.name = "Min alpha";
   alphamin.min = QVariant(0);
   alphamin.max = QVariant(255);
   alphamin.order = 2;
   configurables.insert("alphamin", alphamin);

   TextureGeneratorSetting alphamax;
   alphamax.defaultvalue = QVariant((int) 255);
   alphamax.name = "Max alpha";
   alphamax.min = QVariant(0);
   alphamax.max = QVariant(255);
   alphamax.order = 3;
   configurables.insert("alphamax", alphamax);

   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((int) 300);
   width.name = "Width";
   width.min = QVariant(0);
   width.max = QVariant(1000);
   width.group = "size";
   width.order = 4;
   configurables.insert("width", width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((int) 300);
   height.name = "Height";
   height.min = QVariant(0);
   height.max = QVariant(1000);
   height.group = "size";
   height.order = 5;
   configurables.insert("height", height);

   TextureGeneratorSetting scatter;
   scatter.defaultvalue = QVariant((bool) true);
   scatter.name = "Scatter";
   scatter.order = 6;
   configurables.insert("scatter", scatter);

   TextureGeneratorSetting numpoints;
   numpoints.defaultvalue = QVariant((int) 10 * 1000);
   numpoints.name = "Number of points";
   numpoints.min = QVariant(0);
   numpoints.max = QVariant(1000 * 1000);
   numpoints.order = 7;
   numpoints.enabler = "scatter";
   configurables.insert("numpoints", numpoints);

   TextureGeneratorSetting randomizer;
   randomizer.defaultvalue = QVariant((int) 500);
   randomizer.min = QVariant(0);
   randomizer.max = QVariant(1000);
   randomizer.name = "Random seed";
   randomizer.order = 8;
   configurables.insert("randomizer", randomizer);

   TextureGeneratorSetting smoothscale;
   smoothscale.defaultvalue = QVariant((bool) false);
   smoothscale.name = "Smooth scale";
   smoothscale.order = 9;
   configurables.insert("smoothscale", smoothscale);
}


void NoiseTextureGenerator::generate(QSize size,
                                     TexturePixel* destimage,
                                     QMap<int, TextureImagePtr> sourceimages,
                                     TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QColor color = settings->value("color").value<QColor>();
   int alphamin = settings->value("alphamin").toInt();
   int alphamax = settings->value("alphamax").toInt();
   int randomizer = settings->value("randomizer").toInt();
   int width = settings->value("width").toInt();
   int height = settings->value("height").toInt();
   int numpoints = settings->value("numpoints").toInt();

   QImage destobject(size.width(), size.height(), QImage::Format_ARGB32);
   if (sourceimages.contains(0)) {
      memcpy(destobject.bits(), sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destobject.bits(), 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   if (alphamax < alphamin) {
      // Switch
      alphamax = alphamax + alphamin;
      alphamin = alphamax - alphamin;
      alphamax = alphamax - alphamin;
   }

   if (width > 0 && height > 0) {
      TexturePixel baseColor(color.red(), color.green(), color.blue());
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
      int alpharange = (alphamax - alphamin + 1);
      srand(randomizer);
#else
      QRandomGenerator random(randomizer);
#endif
      QImage tempimage = QImage(width, height, QImage::Format_ARGB32);
      TexturePixel* bufferImage = (TexturePixel*) tempimage.bits();
      if (!settings->value("scatter").toBool()) {
         for (int i = 0; i < width * height; i++) {
            bufferImage[i] = baseColor;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            bufferImage[i].a = alphamin + (qrand() % alpharange);
#else
            bufferImage[i].a = random.bounded(alphamin, alphamax + 1);
#endif
         }
      } else {
         memset(bufferImage, 0, width * height * sizeof(TexturePixel));
         for (int i = 0; i < numpoints; i++) {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            int index = qrand() % (width * height);
#else
            int index = random.bounded(width * height);
#endif
            bufferImage[index] = baseColor;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            bufferImage[index].a = alphamin + (qrand() % alpharange);
#else
            bufferImage[index].a = random.bounded(alphamin, alphamax + 1);
#endif
         }
      }
      Qt::TransformationMode transformationMode = Qt::FastTransformation;
      if (settings->value("smoothscale").toBool()) {
         transformationMode = Qt::SmoothTransformation;
      }
      tempimage = tempimage.scaled(size, Qt::IgnoreAspectRatio, transformationMode);

      QPainter painter(&destobject);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(QPoint(0, 0), tempimage);
   }
   memcpy(destimage, destobject.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
