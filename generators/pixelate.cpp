/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "pixelate.h"

PixelateTextureGenerator::PixelateTextureGenerator()
{
   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((double) 5);
   width.name = "Width";
   width.min = QVariant((double) 0);
   width.max = QVariant((double) 50);
   width.order = 1;
   configurables.insert("width", width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((double) 5);
   height.name = "Height";
   height.min = QVariant((double) 0);
   height.max = QVariant((double) 50);
   height.order = 2;
   configurables.insert("height", height);

   TextureGeneratorSetting offsetx;
   offsetx.defaultvalue = QVariant((double) 0);
   offsetx.name = "Offset left";
   offsetx.min = QVariant((double) -50);
   offsetx.max = QVariant((double) 50);
   offsetx.order = 3;
   configurables.insert("offsetx", offsetx);

   TextureGeneratorSetting offsety;
   offsety.defaultvalue = QVariant((double) 0);
   offsety.name = "Offset top";
   offsety.min = QVariant((double) -50);
   offsety.max = QVariant((double) 50);
   offsety.order = 4;
   configurables.insert("offsety", offsety);
}


void PixelateTextureGenerator::generate(QSize size,
                                        TexturePixel* destimage,
                                        QMap<int, TextureImagePtr> sourceimages,
                                        TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   int width = qMax(settings->value("width").toDouble() / 100 * size.width(), (double) 1);
   int height = qMax(settings->value("height").toDouble() / 100 * size.height(), (double) 1);
   int offsetx = settings->value("offsetx").toDouble() / 100 * size.width();
   int offsety = settings->value("offsety").toDouble() / 100 * size.height();
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(0).data()->getData();
   int y = offsety - size.height();
   while (y < size.height()) {
      if (y + height > 0) {
         int x = offsetx - size.width();
         while (x < size.width()) {
            if (x + width > 0) {
               int totalPixels = 0;
               unsigned long red = 0;
               unsigned long green = 0;
               unsigned long blue = 0;
               unsigned long alpha = 0;
               for (int ypos = y; ypos < y + height; ypos++) {
                  int currY = ypos;
                  if (currY < 0) {
                     currY += size.height() * 5;
                  }
                  currY %= size.height();
                  currY *= size.width();
                  for (int xpos = x; xpos < x + width; xpos++) {
                     int currX = xpos;
                     if (currX < 0) {
                        currX += size.width() * 5;
                     }
                     currX %= size.width();
                     TexturePixel sourcePixel = sourceImage[currY + currX];
                     totalPixels++;
                     red += sourcePixel.r;
                     green += sourcePixel.g;
                     blue += sourcePixel.b;
                     alpha += sourcePixel.a;
                  }
               }
               if (totalPixels > 0) {
                  TexturePixel color(red / totalPixels, green / totalPixels,
                                     blue / totalPixels, alpha / totalPixels);
                  int stopY = qMin(y + height, size.height());
                  for (int ypos = qMax(y, 0); ypos < stopY; ypos++) {
                     int linestart = ypos * size.width();
                     int stopX = qMin(x + width, size.width());
                     for (int xpos = qMax(x, 0); xpos < stopX; xpos++) {
                        destimage[linestart + xpos] = color;
                     }
                  }
               }
            }
            x += width;
         }
      }
      y += height;
   }
}
