/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPainter>
#include "fire.h"

/**
 * @brief FireTextureGenerator::FireTextureGenerator
 *
 * The algorithm is taken from Lode's Computer Graphics Tutorial
 * http://lodev.org/cgtutor/fire.html
 */
FireTextureGenerator::FireTextureGenerator()
{
   TextureGeneratorSetting falloff;
   falloff.defaultvalue = QVariant((double) 0.10);
   falloff.min = QVariant((double) 0);
   falloff.max = QVariant((double) 0.5);
   falloff.name = "Falloff";
   falloff.order = 1;
   configurables.insert("falloff", falloff);

   TextureGeneratorSetting iterations;
   iterations.defaultvalue = QVariant((int) 150);
   iterations.min = QVariant((int) 1);
   iterations.max = QVariant((int) 400);
   iterations.name = "Iterations";
   iterations.order = 2;
   configurables.insert("iterations", iterations);

   TextureGeneratorSetting randomize;
   randomize.defaultvalue = QVariant((int) 5);
   randomize.min = QVariant((int) 1);
   randomize.max = QVariant((int) 500);
   randomize.name = "Randomize";
   randomize.order = 3;
   configurables.insert("randomize", randomize);
}


void FireTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                    QMap<int, TextureImagePtr> sourceimages,
                                    TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   double falloff = 0.245 + settings->value("falloff").toDouble() / 100;
   int iterations = settings->value("iterations").toInt();
   uint randomize = settings->value("randomize").toUInt();
   qsrand(randomize);

   // Speed things up by having a smaller rendering surface.
   int screenWidth = 150;
   int screenHeight = 150;
   TexturePixel* renderSurface = new TexturePixel[150 * 150];

   uint fire[screenHeight][screenWidth];  //this buffer will contain the fire
   memset(&fire, 0, screenWidth * screenWidth * sizeof(uint));
   QColor palette[256]; //this will contain the color palette

   for (int x = 0; x < 256; x++) {
      // generate the palette
      // Hue goes from 0 to 85: red to yellow
      // Saturation is always the maximum: 255
      // Lightness is 0..255 for x=0..128, and 255 for x=128..255
      palette[x] = QColor::fromHsl(x / 3, 255, std::min(255, x * 2));
   }
   for (int i = 0; i < iterations; i++) {
      // randomize the bottom row of the fire buffer
      for (int x = 0; x < screenWidth; x++) {
         fire[screenHeight - 1][x] = abs(32768 + qrand()) % 256;
      }
      // do the fire calculations for every pixel
      for (int y = 0; y < screenHeight - 1; y++) {
         for (int x = 0; x < screenWidth; x++) {
            fire[y][x] = (int)
                  ((fire[(y + 1) % screenHeight][(x - 1 + screenWidth) % screenWidth]
                  + fire[(y + 2) % screenHeight][(x) % screenWidth]
                  + fire[(y + 1) % screenHeight][(x + 1) % screenWidth]
                  + fire[(y + 3) % screenHeight][(x) % screenWidth])
                  * falloff) % 255;
         }
      }
      for (int y = 0; y < screenHeight; y++) {
         for (int x = 0; x < screenWidth; x++) {
            int thisPos = y * screenWidth + x;
            renderSurface[thisPos].r = palette[fire[y][x]].red();
            renderSurface[thisPos].g = palette[fire[y][x]].green();
            renderSurface[thisPos].b = palette[fire[y][x]].blue();
            renderSurface[thisPos].a = renderSurface[thisPos].r;
         }
      }
   }
   QImage tempimage = QImage(screenWidth, screenHeight, QImage::Format_ARGB32);
   memcpy(tempimage.bits(), renderSurface, screenWidth * screenHeight * sizeof(TexturePixel));
   tempimage = tempimage.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

   QImage destobject(size.width(), size.height(), QImage::Format_ARGB32);
   if (sourceimages.contains(0)) {
      memcpy(destobject.bits(), sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destobject.bits(), 0, size.width() * size.height() * sizeof(TexturePixel));
   }
   QPainter painter(&destobject);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.drawImage(QPoint(0, 0), tempimage);
   memcpy(destimage, destobject.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
