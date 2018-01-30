/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QColor>
#include <QPainter>
#include <QLinearGradient>
#include <QGraphicsOpacityEffect>
#include "gradient.h"

using namespace std;

GradientTextureGenerator::GradientTextureGenerator()
{
   TextureGeneratorSetting startcolor;
   startcolor.name = "Start color";
   startcolor.defaultvalue = QVariant(QColor(255, 0, 0, 100));
   startcolor.order = 1;
   configurables.insert("startcolor", startcolor);

   TextureGeneratorSetting middlecolor;
   middlecolor.name = "Middle color";
   middlecolor.defaultvalue = QVariant(QColor(0, 255, 0, 255));
   middlecolor.order = 2;
   configurables.insert("middlecolor", middlecolor);

   TextureGeneratorSetting endcolor;
   endcolor.name = "End color";
   endcolor.defaultvalue = QVariant(QColor(0, 0, 255, 255));
   endcolor.order = 3;
   configurables.insert("endcolor", endcolor);

   TextureGeneratorSetting startposx;
   startposx.name = "Start pos X";
   startposx.defaultvalue = QVariant((double) -20);
   startposx.min = QVariant((double) -100);
   startposx.max = QVariant((double) 100);
   startposx.order = 4;
   configurables.insert("startposx", startposx);

   TextureGeneratorSetting startposy;
   startposy.name = "Start pos Y";
   startposy.defaultvalue = QVariant((double) -20);
   startposy.min = QVariant((double) -100);
   startposy.max = QVariant((double) 100);
   startposy.order = 5;
   configurables.insert("startposy", startposy);

   TextureGeneratorSetting middleposition;
   middleposition.name = "Middle position (%)";
   middleposition.defaultvalue = QVariant((double) 50);
   middleposition.min = QVariant((double) 0);
   middleposition.max = QVariant((double) 100);
   middleposition.order = 6;
   configurables.insert("middleposition", middleposition);

   TextureGeneratorSetting endposx;
   endposx.name = "End pos X";
   endposx.defaultvalue = QVariant((double) 0);
   endposx.min = QVariant((double) -100);
   endposx.max = QVariant((double) 100);
   endposx.order = 7;
   configurables.insert("endposx", endposx);

   TextureGeneratorSetting endposy;
   endposy.name = "End pos Y";
   endposy.defaultvalue = QVariant((double) 20);
   endposy.min = QVariant((double) -100);
   endposy.max = QVariant((double) 100);
   endposy.order = 8;
   configurables.insert("endposy", endposy);
}


void GradientTextureGenerator::generate(QSize size,
                                        TexturePixel* destimage,
                                        QMap<int, TextureImagePtr> sourceimages,
                                        TextureNodeSettings* settings) const
{
   if (!destimage || !size.isValid()) {
      return;
   }
   QColor startcolor = configurables.value("startcolor").defaultvalue.value<QColor>();
   QColor middlecolor = configurables.value("middlecolor").defaultvalue.value<QColor>();
   QColor endcolor = configurables.value("endcolor").defaultvalue.value<QColor>();
   double middleposition = configurables.value("middleposition").defaultvalue.toDouble() / 100;
   int startposx = configurables.value("startposx").defaultvalue.toDouble() * size.width() / 100;
   int startposy = configurables.value("startposy").defaultvalue.toDouble() * size.height() / 100;
   int endposx = configurables.value("endposx").defaultvalue.toDouble() * size.width() / 100;
   int endposy = configurables.value("endposy").defaultvalue.toDouble() * size.height() / 100;
   if (settings != NULL) {
      if (settings->contains("startcolor")) {
         startcolor = settings->value("startcolor").value<QColor>();
      }
      if (settings->contains("middlecolor")) {
         middlecolor = settings->value("middlecolor").value<QColor>();
      }
      if (settings->contains("endcolor")) {
         endcolor = settings->value("endcolor").value<QColor>();
      }
      if (settings->contains("middleposition")) {
         middleposition = settings->value("middleposition").toDouble() / 100;
      }
      if (settings->contains("startposx")) {
         startposx = settings->value("startposx").toDouble() * size.width() / 100;
      }
      if (settings->contains("startposy")) {
         startposy = settings->value("startposy").toDouble() * size.height() / 100;
      }
      if (settings->contains("endposx")) {
         endposx = settings->value("endposx").toDouble() * size.width() / 100;
      }
      if (settings->contains("endposy")) {
         endposy = settings->value("endposy").toDouble() * size.height() / 100;
      }
   }
   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));

   startposx += (double) 50 * size.width() / 100;
   startposy += (double) 50 * size.height() / 100;
   endposx += (double) 50 * size.width() / 100;
   endposy += (double) 50 * size.height() / 100;

   QLinearGradient gradient(startposx, startposy, endposx, endposy);
   gradient.setColorAt(0, startcolor);
   gradient.setColorAt(middleposition, middlecolor);
   gradient.setColorAt(1, endcolor);

   QPainter painter(&tempimage);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, size.width(), size.height(), gradient);

   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
