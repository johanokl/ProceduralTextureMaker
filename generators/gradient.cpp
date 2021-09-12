/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "gradient.h"
#include <QColor>
#include <QGradient>
#include <QGraphicsOpacityEffect>
#include <QPainter>


GradientTextureGenerator::GradientTextureGenerator()
{
   QStringList gradients;
   gradients.append("Linear Gradient");
   gradients.append("Radial Gradient");
   gradients.append("Conical Gradient");
   TextureGeneratorSetting gradient;
   gradient.name = "Gradient";
   gradient.defaultvalue = QVariant(gradients);
   gradient.order = 1;
   configurables.insert("gradient", gradient);

   QStringList spreads;
   spreads.append("Pad Spread");
   spreads.append("Reflect Spread");
   spreads.append("Repeat Spread");
   TextureGeneratorSetting spread;
   spread.name = "Spread";
   spread.defaultvalue = QVariant(spreads);
   spread.order = 2;
   configurables.insert("spread", spread);

   TextureGeneratorSetting startcolor;
   startcolor.name = "Start color";
   startcolor.defaultvalue = QVariant(QColor(255, 0, 0, 100));
   startcolor.order = 3;
   configurables.insert("startcolor", startcolor);

   TextureGeneratorSetting middlecolor;
   middlecolor.name = "Middle color";
   middlecolor.defaultvalue = QVariant(QColor(0, 255, 0, 255));
   middlecolor.order = 4;
   configurables.insert("middlecolor", middlecolor);

   TextureGeneratorSetting endcolor;
   endcolor.name = "End color";
   endcolor.defaultvalue = QVariant(QColor(0, 0, 255, 255));
   endcolor.order = 5;
   configurables.insert("endcolor", endcolor);

   TextureGeneratorSetting startposx;
   startposx.name = "Start pos X";
   startposx.defaultvalue = QVariant((double) -20);
   startposx.min = QVariant(-100);
   startposx.max = QVariant(100);
   startposx.order = 6;
   configurables.insert("startposx", startposx);

   TextureGeneratorSetting startposy;
   startposy.name = "Start pos Y";
   startposy.defaultvalue = QVariant((double) -20);
   startposy.min = QVariant(-100);
   startposy.max = QVariant(100);
   startposy.order = 7;
   configurables.insert("startposy", startposy);

   TextureGeneratorSetting middleposition;
   middleposition.name = "Middle position (%)";
   middleposition.defaultvalue = QVariant((double) 50);
   middleposition.min = QVariant(0);
   middleposition.max = QVariant(100);
   middleposition.order = 8;
   configurables.insert("middleposition", middleposition);

   TextureGeneratorSetting endposx;
   endposx.name = "End pos X";
   endposx.defaultvalue = QVariant((double) 0);
   endposx.min = QVariant(-100);
   endposx.max = QVariant(100);
   endposx.order = 9;
   configurables.insert("endposx", endposx);

   TextureGeneratorSetting endposy;
   endposy.name = "End pos Y";
   endposy.defaultvalue = QVariant((double) 20);
   endposy.min = QVariant(-100);
   endposy.max = QVariant(100);
   endposy.order = 10;
   configurables.insert("endposy", endposy);

   TextureGeneratorSetting radius;
   radius.name = "Radius (%)";
   radius.defaultvalue = QVariant((double) 50);
   radius.min = QVariant(0);
   radius.max = QVariant(200);
   radius.order = 11;
   configurables.insert("radius", radius);
}


void GradientTextureGenerator::generate(QSize size,
                                        TexturePixel* destimage,
                                        QMap<int, TextureImagePtr> sourceimages,
                                        TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   QString gradientmode = settings->value("gradient").toString();
   QString spreadmode = settings->value("spread").toString();
   QColor startcolor = settings->value("startcolor").value<QColor>();
   QColor middlecolor = settings->value("middlecolor").value<QColor>();
   QColor endcolor = settings->value("endcolor").value<QColor>();
   double middleposition = settings->value("middleposition").toDouble() / 100;
   double startposx = settings->value("startposx").toDouble() * size.width() / 100;
   double startposy = settings->value("startposy").toDouble() * size.height() / 100;
   double endposx = settings->value("endposx").toDouble() * size.width() / 100;
   double endposy = settings->value("endposy").toDouble() * size.height() / 100;
   double radius = settings->value("radius").toDouble() * size.width() / 100;

   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   if (sourceimages.contains(0)) {
      memcpy(tempimage.bits(), sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(tempimage.bits(), 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   startposx += (double) 50 * size.width() / 100;
   startposy += (double) 50 * size.height() / 100;
   endposx += (double) 50 * size.width() / 100;
   endposy += (double) 50 * size.height() / 100;

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

   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
