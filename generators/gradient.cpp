
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gradient.h"
#include <QColor>
#include <QGradient>
#include <QPainter>

GradientTextureGenerator::GradientTextureGenerator() {
   QStringList gradients;
   gradients.append("Linear Gradient");
   gradients.append("Radial Gradient");
   gradients.append("Conical Gradient");
   TextureGeneratorSetting gradient;
   gradient.name = "Gradient type";
   gradient.description = "Selects a linear, radial, or conical gradient.";
   gradient.defaultvalue = QVariant(gradients);
   gradient.id = "gradient";
   configurables.append(gradient);

   QStringList spreads;
   spreads.append("Pad Spread");
   spreads.append("Reflect Spread");
   spreads.append("Repeat Spread");
   TextureGeneratorSetting spread;
   spread.name = "Spread mode";
   spread.description = "Controls how the gradient continues beyond its start and end points.";
   spread.defaultvalue = QVariant(spreads);
   spread.id = "spread";
   configurables.append(spread);

   TextureGeneratorSetting startcolor;
   startcolor.name = "Start colour";
   startcolor.description = "Colour at the beginning of the gradient.";
   startcolor.defaultvalue = QVariant(QColor(255, 0, 0, 100));
   startcolor.id = "startcolor";
   configurables.append(startcolor);

   TextureGeneratorSetting middlecolor;
   middlecolor.name = "Middle colour";
   middlecolor.description = "Colour at the configurable middle position.";
   middlecolor.defaultvalue = QVariant(QColor(0, 255, 0, 255));
   middlecolor.id = "middlecolor";
   configurables.append(middlecolor);

   TextureGeneratorSetting endcolor;
   endcolor.name = "End colour";
   endcolor.description = "Colour at the end of the gradient.";
   endcolor.defaultvalue = QVariant(QColor(0, 0, 255, 255));
   endcolor.id = "endcolor";
   configurables.append(endcolor);

   TextureGeneratorSetting startposx;
   startposx.name = "Start X (%)";
   startposx.description = "Horizontal start position as a percentage of the texture width.";
   startposx.defaultvalue = QVariant((double)-20);
   startposx.min = QVariant(-100);
   startposx.max = QVariant(100);
   startposx.id = "startposx";
   configurables.append(startposx);

   TextureGeneratorSetting startposy;
   startposy.name = "Start Y (%)";
   startposy.description = "Vertical start position as a percentage of the texture height.";
   startposy.defaultvalue = QVariant((double)-20);
   startposy.min = QVariant(-100);
   startposy.max = QVariant(100);
   startposy.id = "startposy";
   configurables.append(startposy);

   TextureGeneratorSetting middleposition;
   middleposition.name = "Middle position (%)";
   middleposition.description = "Position of the middle colour between the gradient endpoints.";
   middleposition.defaultvalue = QVariant((double)50);
   middleposition.min = QVariant(0);
   middleposition.max = QVariant(100);
   middleposition.id = "middleposition";
   configurables.append(middleposition);

   TextureGeneratorSetting endposx;
   endposx.name = "End X (%)";
   endposx.description = "Horizontal end position as a percentage of the texture width.";
   endposx.defaultvalue = QVariant((double)0);
   endposx.min = QVariant(-100);
   endposx.max = QVariant(100);
   endposx.id = "endposx";
   configurables.append(endposx);

   TextureGeneratorSetting endposy;
   endposy.name = "End Y (%)";
   endposy.description = "Vertical end position as a percentage of the texture height.";
   endposy.defaultvalue = QVariant((double)20);
   endposy.min = QVariant(-100);
   endposy.max = QVariant(100);
   endposy.id = "endposy";
   configurables.append(endposy);

   TextureGeneratorSetting radius;
   radius.name = "Radial radius (%)";
   radius.description = "Radius of a radial gradient as a percentage of the texture width.";
   radius.defaultvalue = QVariant((double)50);
   radius.min = QVariant(0);
   radius.max = QVariant(200);
   radius.id = "radius";
   configurables.append(radius);
}
void GradientTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                        const QMap<QString, TextureImagePtr>& sourceimages,
                                        const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QString gradientmode = settings.value("gradient").toString();
   QString spreadmode = settings.value("spread").toString();
   QColor startcolor = settings.value("startcolor").value<QColor>();
   QColor middlecolor = settings.value("middlecolor").value<QColor>();
   QColor endcolor = settings.value("endcolor").value<QColor>();
   double middleposition = settings.value("middleposition").toDouble() / 100;
   double startposx = settings.value("startposx").toDouble() * size.width() / 100;
   double startposy = settings.value("startposy").toDouble() * size.height() / 100;
   double endposx = settings.value("endposx").toDouble() * size.width() / 100;
   double endposy = settings.value("endposy").toDouble() * size.height() / 100;
   double radius = settings.value("radius").toDouble() * size.width() / 100;

   QImage tempimage = makeTextureImageView(size, destimage);
   if (sourceimages.contains(QStringLiteral("Input"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Input"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   startposx += (double)50 * size.width() / 100;
   startposy += (double)50 * size.height() / 100;
   endposx += (double)50 * size.width() / 100;
   endposy += (double)50 * size.height() / 100;

   QGradient gradient;
   if (gradientmode == "Linear Gradient") {
      gradient = QLinearGradient(startposx, startposy, endposx, endposy);
   } else if (gradientmode == "Radial Gradient") {
      gradient = QRadialGradient(startposx, startposy, radius, endposx, endposy);
   } else if (gradientmode == "Conical Gradient") {
      QLineF l1(startposx, startposy, endposx, endposy);
      QLineF l2(0, 0, 1, 0);
      double angle = qMin(l1.angleTo(l2), l2.angleTo(l1));
      if (l1.dy() > 0) {
         angle = 360 - angle;
      }
      gradient = QConicalGradient(startposx, startposy, angle);
   }

   QGradient::Spread spread = QGradient::PadSpread;
   if (spreadmode == "Reflect Spread") {
      spread = QGradient::ReflectSpread;
   } else if (spreadmode == "Repeat Spread") {
      spread = QGradient::RepeatSpread;
   }
   gradient.setSpread(spread);
   gradient.setColorAt(0, startcolor);
   gradient.setColorAt(middleposition, middlecolor);
   gradient.setColorAt(1, endcolor);

   QPainter painter(&tempimage);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, size.width(), size.height(), gradient);
}
