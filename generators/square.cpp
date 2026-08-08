
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "square.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

SquareTextureGenerator::SquareTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Colour";
   colorsetting.description = "Colour used to draw the rectangle.";
   colorsetting.defaultvalue = QVariant(QColor(30, 100, 30));
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting shapeWidth;
   shapeWidth.name = "Width (%)";
   shapeWidth.description = "Width of the rectangle as a percentage of the texture width.";
   shapeWidth.defaultvalue = QVariant((double)80);
   shapeWidth.min = QVariant(0);
   shapeWidth.max = QVariant(200);
   shapeWidth.group = "size";
   shapeWidth.id = "width";
   configurables.append(shapeWidth);

   TextureGeneratorSetting shapeHeight;
   shapeHeight.name = "Height (%)";
   shapeHeight.description = "Height of the rectangle as a percentage of the texture height.";
   shapeHeight.defaultvalue = QVariant((double)80);
   shapeHeight.min = QVariant(0);
   shapeHeight.max = QVariant(200);
   shapeHeight.group = "size";
   shapeHeight.id = "height";
   configurables.append(shapeHeight);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation (°)";
   rotation.description = "Rotates the rectangle around its centre.";
   rotation.defaultvalue = QVariant((double)0);
   rotation.min = QVariant(0);
   rotation.max = QVariant(360);
   rotation.id = "rotation";
   configurables.append(rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the rectangle horizontally from the texture centre.";
   offsetLeft.defaultvalue = QVariant((double)0);
   offsetLeft.min = QVariant(-100);
   offsetLeft.max = QVariant(100);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the rectangle vertically from the texture centre.";
   offsetTop.defaultvalue = QVariant((double)0);
   offsetTop.min = QVariant(-100);
   offsetTop.max = QVariant(100);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);

   TextureGeneratorSetting cutoutInnerRadius;
   cutoutInnerRadius.name = "Cut-out width (%)";
   cutoutInnerRadius.description =
       "Width of the transparent area as a percentage of the rectangle width.";
   cutoutInnerRadius.defaultvalue = QVariant((double)0);
   cutoutInnerRadius.min = QVariant(0);
   cutoutInnerRadius.max = QVariant(100);
   cutoutInnerRadius.group = "cutout";
   cutoutInnerRadius.id = "cutoutwidth";
   configurables.append(cutoutInnerRadius);

   TextureGeneratorSetting cutoutOuterRadius;
   cutoutOuterRadius.name = "Cut-out height (%)";
   cutoutOuterRadius.description =
       "Height of the transparent area as a percentage of the rectangle height.";
   cutoutOuterRadius.defaultvalue = QVariant((double)0);
   cutoutOuterRadius.min = QVariant(0);
   cutoutOuterRadius.max = QVariant(100);
   cutoutOuterRadius.group = "cutout";
   cutoutOuterRadius.id = "cutoutheight";
   configurables.append(cutoutOuterRadius);

   TextureGeneratorSetting antialiasing;
   antialiasing.defaultvalue = QVariant((bool)true);
   antialiasing.name = "Antialiasing";
   antialiasing.description = "Smooths the edges of the rectangle and its cut-out.";
   antialiasing.id = "antialiasing";
   configurables.append(antialiasing);
}
void SquareTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }

   QColor color = settings.value("color").value<QColor>();
   double shapeWidth = settings.value("width").toDouble() * size.width() / 100;
   double shapeHeight = settings.value("height").toDouble() * size.height() / 100;
   double rotation = settings.value("rotation").toDouble();
   int offsetLeft = settings.value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings.value("offsettop").toDouble() * size.height() / 100;
   double cutoutInnerRadius = settings.value("cutoutwidth").toDouble() / 100;
   double cutoutOuterRadius = settings.value("cutoutheight").toDouble() / 100;
   bool antialiasing = settings.value("antialiasing").toBool();

   if (sourceimages.contains(QStringLiteral("Canvas"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Canvas"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = makeTextureImageView(size, destimage);

   offsetLeft += (double)50 * size.width() / 100;
   offsetTop += (double)50 * size.height() / 100;

   QPainter painter(&tempimage);
   painter.translate(offsetLeft, offsetTop);
   painter.rotate(rotation);
   painter.translate(-shapeWidth / 2, -shapeHeight / 2);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setBrush(QBrush(color, Qt::BrushStyle::SolidPattern));
   QPainterPath path;

   QPolygonF squarePolygon;
   squarePolygon << QPointF(0, 0);
   squarePolygon << QPointF(0, 1);
   squarePolygon << QPointF(1, 1);
   squarePolygon << QPointF(1, 0);
   path.addPolygon(squarePolygon);

   cutoutInnerRadius /= 2;
   cutoutOuterRadius /= 2;

   QPolygonF removeSquarePolygon;
   removeSquarePolygon << QPointF(0.5 - cutoutInnerRadius, 0.5 - cutoutOuterRadius);
   removeSquarePolygon << QPointF(0.5 - cutoutInnerRadius, 0.5 + cutoutOuterRadius);
   removeSquarePolygon << QPointF(0.5 + cutoutInnerRadius, 0.5 + cutoutOuterRadius);
   removeSquarePolygon << QPointF(0.5 + cutoutInnerRadius, 0.5 - cutoutOuterRadius);

   QPainterPath removepath;
   removepath.addPolygon(removeSquarePolygon);
   path = path.subtracted(removepath);

   painter.scale(shapeWidth, shapeHeight);
   painter.setRenderHint(QPainter::Antialiasing, antialiasing);
   painter.setPen(Qt::NoPen);
   painter.drawPath(path);
}
