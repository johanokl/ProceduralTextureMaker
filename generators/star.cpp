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
#include "star.h"

using namespace std;

StarTextureGenerator::StarTextureGenerator()
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

   TextureGeneratorSetting arms;
   arms.name = "Number of points";
   arms.description = "";
   arms.defaultvalue = QVariant((int) 6);
   arms.min = QVariant((int) 3);
   arms.max = QVariant((int) 13);
   arms.order = 7;
   configurables.insert("numarms", arms);

   TextureGeneratorSetting innerRadius;
   innerRadius.name = "Innner radius";
   innerRadius.description = "";
   innerRadius.defaultvalue = QVariant((double) 25);
   innerRadius.min = QVariant((double) 0);
   innerRadius.max = QVariant((double) 100);
   innerRadius.order = 8;
   configurables.insert("innerradius", innerRadius);

   TextureGeneratorSetting outerRadius;
   outerRadius.name = "Outer radius";
   outerRadius.description = "";
   outerRadius.defaultvalue = QVariant((double) 100);
   outerRadius.min = QVariant((double) 0);
   outerRadius.max = QVariant((double) 100);
   outerRadius.order = 9;
   configurables.insert("outerradius", outerRadius);

   TextureGeneratorSetting cutoutInnerRadius;
   cutoutInnerRadius.name = "Cutout innner radius";
   cutoutInnerRadius.description = "";
   cutoutInnerRadius.defaultvalue = QVariant((double) 15);
   cutoutInnerRadius.min = QVariant((double) 0);
   cutoutInnerRadius.max = QVariant((double) 100);
   cutoutInnerRadius.order = 10;
   configurables.insert("cutoutinnerradius", cutoutInnerRadius);

   TextureGeneratorSetting cutoutOuterRadius;
   cutoutOuterRadius.name = "Cutout outer radius";
   cutoutOuterRadius.description = "";
   cutoutOuterRadius.defaultvalue = QVariant((double) 65);
   cutoutOuterRadius.min = QVariant((double) 0);
   cutoutOuterRadius.max = QVariant((double) 100);
   cutoutOuterRadius.order = 11;
   configurables.insert("cutoutouterradius", cutoutOuterRadius);
}


void StarTextureGenerator::generate(QSize size,
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
   double arms = settings->value("numarms").toDouble();
   double innerRadius = settings->value("innerradius").toDouble() / 100;
   double outerRadius = settings->value("outerradius").toDouble() / 100;
   double cutoutInnerRadius = settings->value("cutoutinnerradius").toDouble() / 100;
   double cutoutOuterRadius = settings->value("cutoutouterradius").toDouble() / 100;

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

   QPolygonF starPolygon;
   for (int i = 0; i < 2 * arms; i++) {
      // Use outer or inner radius depending on what iteration we are in.
      double r = (i & 1) == 0 ? outerRadius : innerRadius;
      starPolygon << QPointF(0.5 + 0.5 * cos(i * M_PI / arms) * r,
                             0.5 + 0.5 * sin(i * M_PI / arms) * r);
   }

   QPolygonF removeStarPolygon;
   for (int i = 0; i < 2 * arms; i++) {
      // Use outer or inner radius depending on what iteration we are in.
      double r = (i & 1) == 0 ? cutoutOuterRadius : cutoutInnerRadius;
      removeStarPolygon << QPointF(0.5 + 0.5 * cos(i * M_PI / arms) * r,
                                   0.5 + 0.5 * sin(i * M_PI / arms) * r);
   }
   path.addPolygon(starPolygon);
   QPainterPath removepath;
   removepath.addPolygon(removeStarPolygon);
   path = path.subtracted(removepath);

   painter.scale(shapeWidth, shapeHeight);
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.setPen(Qt::NoPen);
   painter.drawPath(path);

   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
