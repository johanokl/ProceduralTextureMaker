/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

/**
 * Algorithms taken from Florian Reuschel's
 * https://github.com/Loilo/color-blend
 */

#include <math.h>
#include "blending.h"

using namespace std;

BlendingTextureGenerator::BlendingTextureGenerator()
{
   QStringList modes;
   modes.append("Normal");
   modes.append("Darken");
   modes.append("Multiply");
   modes.append("Lighten");
   modes.append("Screen");
   modes.append("Color Dodge");
   modes.append("Color Burn");
   modes.append("Overlay");
   modes.append("Soft Light");
   modes.append("Hard Light");
   modes.append("Difference");
   modes.append("Exclusion");
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
   blendingAlpha.defaultvalue = QVariant((double) 100);
   blendingAlpha.name = "Level";
   blendingAlpha.min = 0;
   blendingAlpha.max = 100;
   blendingAlpha.description = "Alpha value of the blending (0-100)";
   blendingAlpha.order = 3;
   configurables.insert("alpha", blendingAlpha);
}


int BlendingTextureGenerator::alphaCompose(double originAlpha, double addAlpha,
                                           double compositeAlpha,
                                           double originColor, double addColor,
                                           double compositeColor) const
{
   return qMax(0, qMin(255, (int)
                       ((1 - (addAlpha / compositeAlpha)) * originColor +
                        (addAlpha / compositeAlpha) *
                        qRound((1 - originAlpha) * addColor +
                               originAlpha * compositeColor))));
}


double BlendingTextureGenerator::blendColors(BlendModes mode, double originColor,
                                             double addColor) const
{
   originColor /= 255;
   addColor /= 255;
   switch (mode) {
   case BlendModes::Multiply:
      return originColor * addColor;
   case BlendModes::Screen:
      return originColor + addColor - originColor * addColor;
   case BlendModes::Overlay:
      return blendColors(BlendModes::HardLight, addColor * 255, originColor * 255);
   case BlendModes::Darken:
      return qMin(originColor, addColor);
   case BlendModes::Lighten:
      return qMin(qMax(originColor, addColor), 1.0);
   case BlendModes::ColorDodge:
      if (originColor == 0) {
         return 0;
      }
      if (addColor == 1) {
         return 1;
      }
      return qMin(1.0, originColor / (1 - addColor));
   case BlendModes::ColorBurn:
      if (originColor == 1) {
         return 1;
      }
      if (addColor == 0) {
         return 0;
      }
      return 1 - qMin(1.0, (1 - originColor) / addColor);
   case BlendModes::HardLight:
      if (addColor <= 0.5) {
         return blendColors(BlendModes::Multiply, originColor * 255, (2 * addColor) * 255);
      }
      return blendColors(BlendModes::Screen, originColor * 255, (2 * addColor - 1) * 255);
   case BlendModes::SoftLight:
      if (addColor <= 0.5) {
         return originColor - (1 - 2 * addColor) * originColor * (1 - originColor);
      }
      return originColor - (2 * addColor - 1) * (originColor - ((originColor <= 0.25) ?
         ((16 * originColor - 12) * originColor + 4) * originColor : sqrt(originColor)));
   case BlendModes::Difference:
      return qAbs(originColor - addColor);
   case BlendModes::Exclusion:
      return originColor + addColor - 2 * originColor * addColor;
   default:
      return addColor;
   }
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
   } else if (mode == "Color Dodge") {
      blendMode = BlendModes::ColorDodge;
   } else if (mode == "Color Burn") {
      blendMode = BlendModes::ColorBurn;
   } else if (mode == "Overlay") {
      blendMode = BlendModes::Overlay;
   } else if (mode == "Soft Light") {
      blendMode = BlendModes::SoftLight;
   } else if (mode == "Hard Light") {
      blendMode = BlendModes::HardLight;
   } else if (mode == "Difference") {
      blendMode = BlendModes::Difference;
   } else if (mode == "Exclusion") {
      blendMode = BlendModes::Exclusion;
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
         double addAlpha = (blendingAlpha * addSource[thisPos].a) / 255;
         double originAlpha = ((double) originSource[thisPos].a) / 255;
         double pixelAlpha = addAlpha + originAlpha - addAlpha * originAlpha;
         int r = blendColors(blendMode, originSource[thisPos].r, addSource[thisPos].r) * 255;
         int g = blendColors(blendMode, originSource[thisPos].g, addSource[thisPos].g) * 255;
         int b = blendColors(blendMode, originSource[thisPos].b, addSource[thisPos].b) * 255;
         destimage[thisPos].r = alphaCompose(originAlpha, addAlpha, pixelAlpha,
                                             originSource[thisPos].r, addSource[thisPos].r, r);
         destimage[thisPos].g = alphaCompose(originAlpha, addAlpha, pixelAlpha,
                                             originSource[thisPos].g, addSource[thisPos].g, g);
         destimage[thisPos].b = alphaCompose(originAlpha, addAlpha, pixelAlpha,
                                             originSource[thisPos].b, addSource[thisPos].b, b);
         destimage[thisPos].a = pixelAlpha * 255;
      }
   } else if (originSource) {
      memcpy(destimage, originSource, numPixels * sizeof(TexturePixel));
   } else if (addSource) {
      memcpy(destimage, addSource, numPixels * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, numPixels * sizeof(TexturePixel));
   }
}
