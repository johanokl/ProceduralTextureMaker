/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "blending.h"

using namespace std;

BlendingTextureGenerator::BlendingTextureGenerator()
{
   QStringList modes;
   // Normal
   modes.append("Normal");
   modes.append("Darken");
   modes.append("Multiply");
   modes.append("Lighten");
   modes.append("Screen");
   modes.append("Linear Dodge (Add)");

   TextureGeneratorSetting mode;
   mode.name = "Mode";
   mode.description = "How to blend";
   mode.defaultvalue = QVariant(modes);
   mode.order = 1;
   configurables.insert("mode", mode);

   QStringList ordering;
   ordering.append("Slot 2 on top of Slot 1");
   ordering.append("Slot 1 on top of Slot 2");
   TextureGeneratorSetting order;
   order.name = "Order";
   order.description = "";
   order.defaultvalue = QVariant(ordering);
   order.order = 2;
   configurables.insert("order", order);

   TextureGeneratorSetting blendingAlpha;
   blendingAlpha.defaultvalue = QVariant((double) 128);
   blendingAlpha.name = "Level";
   blendingAlpha.min = 0;
   blendingAlpha.max = 100;
   blendingAlpha.description = "Alpha value of the blending (0-100)";
   blendingAlpha.order = 3;
   configurables.insert("alpha", blendingAlpha);
}

unsigned char BlendingTextureGenerator::blendColors(double originColor, double addColor,
                                                    double addPixelAlpha, double blendingAlpha,
                                                    BlendModes mode) const
{
   double val;
   double secondLevel;

   switch (mode) {
   case BlendModes::Normal:
      secondLevel = blendingAlpha * addPixelAlpha / 255;
      val = (1 - secondLevel) * originColor + secondLevel * addColor;
      break;
   case BlendModes::Darken:
      val = qMin(originColor, addColor);
      break;
   case BlendModes::Lighten:
      val = qMax(originColor, addColor);
      break;
   case BlendModes::Multiply:
      val = originColor * addColor / 255;
      break;
   case BlendModes::Screen:
      secondLevel = blendingAlpha * addPixelAlpha / 255;
      val = (255 - (((255 - (1 - secondLevel) * originColor) * (255 - secondLevel * addColor)) / 255));
      break;
   case BlendModes::LinearDodgeAdd:
      secondLevel = blendingAlpha * addPixelAlpha / 255;
      val = qMin(originColor + secondLevel * addColor, (double) 255);
      break;
   default:
      val = 0;
   }
   return qMax(qMin(val, (double)255), (double)0);
}


void BlendingTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                        QMap<int, TextureImagePtr> sourceimages,
                                        TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   double blendingAlpha = settings->value("alpha").toDouble() / 100.0;
   QString order = settings->value("order").toString();
   QString mode = settings->value("mode").toString();
   if (blendingAlpha > 1) {
      blendingAlpha = 1;
   } else if (blendingAlpha < 0) {
      blendingAlpha = 0;
   }

   BlendModes blendMode = BlendModes::Normal;
   if (mode == "Darken") {
      blendMode = BlendModes::Darken;
   } else if (mode == "Multiply") {
      blendMode = BlendModes::Multiply;
   } else if (mode == "Lighten") {
      blendMode = BlendModes::Lighten;
   } else if (mode == "Screen") {
      blendMode = BlendModes::Screen;
   } else if (mode == "Linear Dodge (Add)") {
      blendMode = BlendModes::LinearDodgeAdd;
   }
   TexturePixel* originSource = NULL;
   TexturePixel* addSource = NULL;

   int first = 0;
   int second = 1;
   if (order == "Slot 1 on top of Slot 2") {
      first = 1;
      second = 0;
   }

   if (sourceimages.contains(first)) {
      originSource = sourceimages.value(first).data()->getData();
   }
   if (sourceimages.contains(second)) {
      addSource = sourceimages.value(second).data()->getData();
   }

   int numPixels = size.width() * size.height();
   if (originSource && addSource) {
      for (int thisPos = 0; thisPos < numPixels; thisPos++) {
         destimage[thisPos].r = blendColors(originSource[thisPos].r, addSource[thisPos].r, addSource[thisPos].a, blendingAlpha, blendMode);
         destimage[thisPos].g = blendColors(originSource[thisPos].g, addSource[thisPos].g, addSource[thisPos].a, blendingAlpha, blendMode);
         destimage[thisPos].b = blendColors(originSource[thisPos].b, addSource[thisPos].b, addSource[thisPos].a, blendingAlpha, blendMode);
         destimage[thisPos].a = blendColors(originSource[thisPos].a, addSource[thisPos].a, addSource[thisPos].a, blendingAlpha, blendMode);
      }
   } else if (originSource) {
      memcpy(destimage, originSource, numPixels * sizeof(TexturePixel));
   } else if (addSource) {
      memcpy(destimage, addSource, numPixels * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, numPixels * sizeof(TexturePixel));
   }
}
