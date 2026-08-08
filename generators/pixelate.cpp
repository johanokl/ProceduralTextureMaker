
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "pixelate.h"

PixelateTextureGenerator::PixelateTextureGenerator() {
   TextureGeneratorSetting width;
   width.defaultvalue = QVariant((double)5);
   width.name = "Block width (%)";
   width.description = "Width of each pixel block as a percentage of the texture width.";
   width.min = QVariant(0);
   width.max = QVariant(50);
   width.group = "size";
   width.id = "width";
   configurables.append(width);

   TextureGeneratorSetting height;
   height.defaultvalue = QVariant((double)5);
   height.name = "Block height (%)";
   height.description = "Height of each pixel block as a percentage of the texture height.";
   height.min = QVariant(0);
   height.max = QVariant(50);
   height.group = "size";
   height.id = "height";
   configurables.append(height);

   TextureGeneratorSetting offsetx;
   offsetx.defaultvalue = QVariant((double)0);
   offsetx.name = "Horizontal offset (%)";
   offsetx.description = "Moves the block grid horizontally across the texture.";
   offsetx.min = QVariant(-50);
   offsetx.max = QVariant(50);
   offsetx.id = "offsetx";
   configurables.append(offsetx);

   TextureGeneratorSetting offsety;
   offsety.defaultvalue = QVariant((double)0);
   offsety.name = "Vertical offset (%)";
   offsety.description = "Moves the block grid vertically across the texture.";
   offsety.min = QVariant(-50);
   offsety.max = QVariant(50);
   offsety.id = "offsety";
   configurables.append(offsety);
}
void PixelateTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                        const QMap<QString, TextureImagePtr>& sourceimages,
                                        const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   int width = qMax(settings.value("width").toDouble() / 100 * size.width(), (double)1);
   int height = qMax(settings.value("height").toDouble() / 100 * size.height(), (double)1);
   int offsetx = settings.value("offsetx").toDouble() / 100 * size.width();
   int offsety = settings.value("offsety").toDouble() / 100 * size.height();
   if (!sourceimages.contains(QStringLiteral("Input"))) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(QStringLiteral("Input")).data()->getData();
   int y = offsety - size.height();
   while (y < size.height()) {
      if (y + height > 0) {
         int x = offsetx - size.width();
         while (x < size.width()) {
            if (x + width > 0) {
               int totalPixels = 0;
               quint64 red = 0;
               quint64 green = 0;
               quint64 blue = 0;
               quint64 alpha = 0;
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
                  TexturePixel color(red / totalPixels, green / totalPixels, blue / totalPixels,
                                     alpha / totalPixels);
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
