/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */


#include <math.h>
#include <QStringList>
#include <QColor>
#include <QPainter>
#include "core/textureimage.h"
#include "square.h"

using namespace std;

SquareTextureGenerator::SquareTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Color";
   colorsetting.defaultvalue = QVariant(QColor(200, 100, 0));
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting shapeWidth;
   shapeWidth.name = "Width";
   shapeWidth.description = "";
   shapeWidth.defaultvalue = QVariant((double) 80);
   shapeWidth.min = QVariant((double) 0);
   shapeWidth.max = QVariant((double) 200);
   shapeWidth.order = 2;
   configurables.insert("width", shapeWidth);

   TextureGeneratorSetting shapeHeight;
   shapeHeight.name = "Height";
   shapeHeight.description = "";
   shapeHeight.defaultvalue = QVariant((double) 80);
   shapeHeight.min = QVariant((double) 0);
   shapeHeight.max = QVariant((double) 200);
   shapeHeight.order = 3;
   configurables.insert("height", shapeHeight);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation";
   rotation.description = "";
   rotation.defaultvalue = QVariant((double) 50);
   rotation.min = QVariant((double) 0);
   rotation.max = QVariant((double) 360);
   rotation.order = 4;
   configurables.insert("rotation", rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset from left";
   offsetLeft.description = "";
   offsetLeft.defaultvalue = QVariant((double) 0);
   offsetLeft.min = QVariant((double) -100);
   offsetLeft.max = QVariant((double) 100);
   offsetLeft.order = 5;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset from top";
   offsetTop.description = "";
   offsetTop.defaultvalue = QVariant((double) 0);
   offsetTop.min = QVariant((double) -100);
   offsetTop.max = QVariant((double) 100);
   offsetTop.order = 6;
   configurables.insert("offsettop", offsetTop);

   TextureGeneratorSetting cutoutInnerRadius;
   cutoutInnerRadius.name = "Cutout width";
   cutoutInnerRadius.description = "";
   cutoutInnerRadius.defaultvalue = QVariant((double) 0);
   cutoutInnerRadius.min = QVariant((double) 0);
   cutoutInnerRadius.max = QVariant((double) 100);
   cutoutInnerRadius.order = 7;
   configurables.insert("cutoutwidth", cutoutInnerRadius);

   TextureGeneratorSetting cutoutOuterRadius;
   cutoutOuterRadius.name = "Cutout height";
   cutoutOuterRadius.description = "";
   cutoutOuterRadius.defaultvalue = QVariant((double) 0);
   cutoutOuterRadius.min = QVariant((double) 0);
   cutoutOuterRadius.max = QVariant((double) 100);
   cutoutOuterRadius.order = 8;
   configurables.insert("cutoutheight", cutoutOuterRadius);
}


void SquareTextureGenerator::generate(QSize size,
                                      TexturePixel* destimage,
                                      QMap<int, TextureImagePtr> sourceimages,
                                      TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }

   QColor color = settings->value("color").value<QColor>();
   double shapeWidth = settings->value("width").toDouble() * size.width() / 100;
   double shapeHeight = settings->value("height").toDouble() * size.height() / 100;
   double rotation = settings->value("rotation").toDouble();
   int offsetLeft = settings->value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings->value("offsettop").toDouble() * size.height() / 100;
   double cutoutInnerRadius = settings->value("cutoutwidth").toDouble() / 100;
   double cutoutOuterRadius = settings->value("cutoutheight").toDouble() / 100;

   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));

   offsetLeft += (double) 50 * size.width() / 100;
   offsetTop += (double) 50 * size.height() / 100;

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
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.setPen(Qt::NoPen);
   painter.drawPath(path);

   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
