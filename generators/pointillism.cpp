
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
   points.name = "Point count";
   points.description = "Number of randomly positioned ellipses used to recreate the image.";
   points.min = QVariant(0);
   points.max = QVariant(250 * 1000);
   points.id = "points";
   configurables.append(points);

   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((double)1);
   width.name = "Horizontal radius (%)";
   width.description = "Horizontal radius of each ellipse as a percentage of the texture width.";
   width.min = QVariant(0.2);
   width.max = QVariant(50);
   width.group = "size";
   width.id = "width";
   configurables.append(width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((double)1);
   height.name = "Vertical radius (%)";
   height.description = "Vertical radius of each ellipse as a percentage of the texture height.";
   height.min = QVariant(0.2);
   height.max = QVariant(50);
   height.group = "size";
   height.id = "height";
   configurables.append(height);

   TextureGeneratorSetting includesource;
   includesource.defaultvalue = QVariant((bool)true);
   includesource.name = "Include source image";
   includesource.description = "Keeps the original input visible beneath the generated points.";
   includesource.id = "includesource";
   configurables.append(includesource);

   TextureGeneratorSetting antialiasing;
   antialiasing.defaultvalue = QVariant((bool)true);
   antialiasing.name = "Antialiasing";
   antialiasing.description = "Smooths the edges of the generated ellipses.";
   antialiasing.id = "antialiasing";
   configurables.append(antialiasing);

   TextureGeneratorSetting randseed;
   randseed.defaultvalue = QVariant((int)500);
   randseed.name = "Random seed";
   randseed.description = "Selects the repeatable positions and sampled colours of the points.";
   randseed.min = QVariant(0);
   randseed.max = QVariant(1000);
   randseed.id = "randseed";
   configurables.append(randseed);
}
void PointillismTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                           const QMap<QString, TextureImagePtr>& sourceimages,
                                           const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   double shapeWidth = settings.value("width").toDouble() * size.width() / 100;
   double shapeHeight = settings.value("height").toDouble() * size.height() / 100;
   int randseed = settings.value("randseed").toInt();
   int points = settings.value("points").toInt();
   bool includesource = settings.value("includesource").toBool();
   bool antialiasing = settings.value("antialiasing").toBool();

   if (!sourceimages.contains(QStringLiteral("Image"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }

   QRandomGenerator random(randseed);

   TexturePixel* sourceImage = sourceimages.value(QStringLiteral("Image"))->getData();
   if (includesource) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Image"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = makeTextureImageView(size, destimage);

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
}
