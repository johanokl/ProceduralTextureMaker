
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureimage.h"
#include "star.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <cmath>

StarTextureGenerator::StarTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Colour";
   colorsetting.description = "Colour used to draw the star.";
   colorsetting.defaultvalue = QVariant(QColor(200, 100, 0));
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting shapeWidth;
   shapeWidth.name = "Width (%)";
   shapeWidth.description = "Width of the star as a percentage of the texture width.";
   shapeWidth.defaultvalue = QVariant((double)80);
   shapeWidth.min = QVariant(0);
   shapeWidth.max = QVariant(200);
   shapeWidth.group = "size";
   shapeWidth.id = "width";
   configurables.append(shapeWidth);

   TextureGeneratorSetting shapeHeight;
   shapeHeight.name = "Height (%)";
   shapeHeight.description = "Height of the star as a percentage of the texture height.";
   shapeHeight.defaultvalue = QVariant((double)80);
   shapeHeight.min = QVariant(0);
   shapeHeight.max = QVariant(200);
   shapeHeight.group = "size";
   shapeHeight.id = "height";
   configurables.append(shapeHeight);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation (°)";
   rotation.description = "Rotates the star around its centre.";
   rotation.defaultvalue = QVariant((double)50);
   rotation.min = QVariant(0);
   rotation.max = QVariant(360);
   rotation.id = "rotation";
   configurables.append(rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the star horizontally from the texture centre.";
   offsetLeft.defaultvalue = QVariant((double)0);
   offsetLeft.min = QVariant(-100);
   offsetLeft.max = QVariant(100);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the star vertically from the texture centre.";
   offsetTop.defaultvalue = QVariant((double)0);
   offsetTop.min = QVariant(-100);
   offsetTop.max = QVariant(100);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);

   TextureGeneratorSetting arms;
   arms.name = "Point count";
   arms.description = "Number of points around the star.";
   arms.defaultvalue = QVariant((int)6);
   arms.min = QVariant(3);
   arms.max = QVariant(13);
   arms.id = "numarms";
   configurables.append(arms);

   TextureGeneratorSetting innerRadius;
   innerRadius.name = "Inner radius (%)";
   innerRadius.description = "Radius of the valleys between the star's points.";
   innerRadius.defaultvalue = QVariant((double)25);
   innerRadius.min = QVariant(0);
   innerRadius.max = QVariant(100);
   innerRadius.id = "innerradius";
   configurables.append(innerRadius);

   TextureGeneratorSetting outerRadius;
   outerRadius.name = "Outer radius (%)";
   outerRadius.description = "Radius from the centre to the tips of the star.";
   outerRadius.defaultvalue = QVariant((double)100);
   outerRadius.min = QVariant(0);
   outerRadius.max = QVariant(100);
   outerRadius.id = "outerradius";
   configurables.append(outerRadius);

   TextureGeneratorSetting cutoutInnerRadius;
   cutoutInnerRadius.name = "Cut-out inner radius (%)";
   cutoutInnerRadius.description = "Inner radius of the star-shaped transparent area.";
   cutoutInnerRadius.defaultvalue = QVariant((double)15);
   cutoutInnerRadius.min = QVariant(0);
   cutoutInnerRadius.max = QVariant(100);
   cutoutInnerRadius.id = "cutoutinnerradius";
   configurables.append(cutoutInnerRadius);

   TextureGeneratorSetting cutoutOuterRadius;
   cutoutOuterRadius.name = "Cut-out outer radius (%)";
   cutoutOuterRadius.description = "Outer radius of the star-shaped transparent area.";
   cutoutOuterRadius.defaultvalue = QVariant((double)65);
   cutoutOuterRadius.min = QVariant(0);
   cutoutOuterRadius.max = QVariant(100);
   cutoutOuterRadius.id = "cutoutouterradius";
   configurables.append(cutoutOuterRadius);

   TextureGeneratorSetting antialiasing;
   antialiasing.defaultvalue = QVariant((bool)true);
   antialiasing.name = "Antialiasing";
   antialiasing.description = "Smooths the edges of the star and its cut-out.";
   antialiasing.id = "antialiasing";
   configurables.append(antialiasing);
}
void StarTextureGenerator::generate(QSize size, TexturePixel* destimage,
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
   double arms = settings.value("numarms").toDouble();
   double innerRadius = settings.value("innerradius").toDouble() / 100;
   double outerRadius = settings.value("outerradius").toDouble() / 100;
   double cutoutInnerRadius = settings.value("cutoutinnerradius").toDouble() / 100;
   double cutoutOuterRadius = settings.value("cutoutouterradius").toDouble() / 100;
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
   painter.setRenderHint(QPainter::Antialiasing, antialiasing);
   painter.setPen(Qt::NoPen);
   painter.drawPath(path);
}
