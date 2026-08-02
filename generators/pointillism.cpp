
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "pointillism.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>
#include <cmath>

PointillismTextureGenerator::PointillismTextureGenerator() {
   TextureGeneratorSetting points;
   points.defaultvalue = QVariant((int)10000);
   points.name = "Points";
   points.min = QVariant(0);
   points.max = QVariant(250 * 1000);
   points.order = 1;
   configurables.insert("points", points);

   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((double)1);
   width.name = "Width";
   width.min = QVariant(0.2);
   width.max = QVariant(50);
   width.order = 2;
   width.group = "size";
   configurables.insert("width", width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((double)1);
   height.name = "Height";
   height.min = QVariant(0.2);
   height.max = QVariant(50);
   height.order = 3;
   height.group = "size";
   configurables.insert("height", height);

   TextureGeneratorSetting includesource;
   includesource.defaultvalue = QVariant((bool)true);
   includesource.name = "Include source";
   includesource.order = 4;
   configurables.insert("includesource", includesource);

   TextureGeneratorSetting antialiasing;
   antialiasing.defaultvalue = QVariant((bool)true);
   antialiasing.name = "Antialiasing";
   antialiasing.order = 5;
   configurables.insert("antialiasing", antialiasing);

   TextureGeneratorSetting randseed;
   randseed.defaultvalue = QVariant((int)500);
   randseed.name = "Random seed";
   randseed.min = QVariant(0);
   randseed.max = QVariant(1000);
   randseed.order = 6;
   configurables.insert("randseed", randseed);
}
void PointillismTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                           QMap<QString, TextureImagePtr> sourceimages,
                                           TextureNodeSettings* settings) const {
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   double shapeWidth = settings->value("width").toDouble() * size.width() / 100;
   double shapeHeight = settings->value("height").toDouble() * size.height() / 100;
   int randseed = settings->value("randseed").toInt();
   int points = settings->value("points").toInt();
   bool includesource = settings->value("includesource").toBool();
   bool antialiasing = settings->value("antialiasing").toBool();

   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   QRandomGenerator random(randseed);

   TexturePixel* sourceImage = sourceimages.value(QStringLiteral("Input"))->getData();
   if (includesource) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Input"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));

   QPainter painter(&tempimage);
   painter.setPen(Qt::NoPen);
   painter.setRenderHint(QPainter::Antialiasing, antialiasing);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

   for (int i = 0; i < points; i++) {
      int x = random.bounded(0, size.width());
      int y = random.bounded(0, size.height());
      TexturePixel sourcePixel = sourceImage[y * size.width() + x];
      QColor sourceColor(sourcePixel.r, sourcePixel.g, sourcePixel.b, sourcePixel.a);
      painter.setBrush(QBrush(sourceColor, Qt::BrushStyle::SolidPattern));
      painter.drawEllipse(QPointF(x, y), shapeWidth, shapeHeight);
   }
   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
