/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "perlinnoise.h"
#include <QColor>
#include <QtMath>
#include <cmath>

PerlinNoiseTextureGenerator::PerlinNoiseTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.defaultvalue = QVariant(QColor(255, 255, 255));
   colorsetting.name = "Color";
   colorsetting.description = "Color of the circle";
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting numOctaves;
   numOctaves.defaultvalue = QVariant((int) 5);
   numOctaves.name = "Octaves";
   numOctaves.description = "Number of octaves";
   numOctaves.order = 2;
   configurables.insert("numoctaves", numOctaves);

   TextureGeneratorSetting persistence;
   persistence.defaultvalue = QVariant((double) 0.5);
   persistence.name = "Persistence";
   persistence.description = "This controls the roughness of the picture,";
   persistence.order = 3;
   configurables.insert("persistence", persistence);

   TextureGeneratorSetting zoom;
   zoom.defaultvalue = QVariant((double) 75);
   zoom.min = QVariant(10);
   zoom.max = QVariant(200);
   zoom.name = "Zoom";
   zoom.order = 4;
   configurables.insert("zoom", zoom);

   TextureGeneratorSetting randomizer;
   randomizer.defaultvalue = QVariant((double) 500);
   randomizer.min = QVariant(0);
   randomizer.max = QVariant(1000);
   randomizer.name = "Random seed";
   randomizer.order = 5;
   configurables.insert("randomizer", randomizer);
}


double PerlinNoiseTextureGenerator::findnoise2(double x, double y) const
{
   int n = (int) x + (int) y * 57;
   n = (n << 13) ^ n;
   int nn = (n * (n * n * 60493 + 19990303) + 1376312589)&  0x7fffffff;
   return 1.0 - ((double) nn / 1073741824.0);
}


double PerlinNoiseTextureGenerator::noise(double x, double y) const
{
   double s = findnoise2((int) x, (int) y);
   double t = findnoise2((int) x + 1, (int) y);
   double u = findnoise2((int) x, (int) y + 1);
   double v = findnoise2((int) x + 1, (int) y + 1);
   double int1 = interpolate(s, t, x - (int) x);
   double int2 = interpolate(u, v, x - (int) x);
   return interpolate(int1, int2, y - (int) y);
}


double PerlinNoiseTextureGenerator::interpolate(double a, double b, double x) const
{
   double f = (1.0 - cos(x * 3.1415927)) * 0.5;
   return a * (1.0 - f) + b * f;
}


void PerlinNoiseTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                           QMap<int, TextureImagePtr> sourceimages,
                                           TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }


   QColor color = settings->value("color").value<QColor>();
   int numOctaves = settings->value("numoctaves").toInt();
   double persistence = settings->value("persistence").toDouble();
   double zoom = settings->value("zoom").toDouble();
   double randomizer = settings->value("randomizer").toDouble() * 1000;
   double xFactor = (double) 500 / size.width();
   double yFactor = (double) 500 / size.height();
   bool blend = false;
   TexturePixel* sourceImg = nullptr;
   if (sourceimages.contains(0)) {
      sourceImg = sourceimages.value(0)->getData();
      blend = true;
   }
   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         double getnoise = 0;
         int thisPos = y * size.width() + x;

         for (int currOctave = 0; currOctave < numOctaves - 1; currOctave++) {
            double frequency = pow(2, currOctave); //This increases the frequency with every loop of the octave.
            double amplitude = pow(persistence, currOctave);//This decreases the amplitude with every loop of the octave.
            getnoise += noise(randomizer + static_cast<double>(x) * xFactor * frequency / zoom,
                              randomizer + static_cast<double>(y) * yFactor / zoom * frequency) * amplitude;
         }
         auto pixelColor = static_cast<int>((getnoise * 128.0) + 128.0);
         if (pixelColor > 255) {
            pixelColor = 255;
         }
         if (pixelColor < 0) {
            pixelColor = 0;
         }
         double fraction = (static_cast<double>(pixelColor) / 255.0);
         double negVal = 1 - (static_cast<double>(pixelColor) / 255.0);

         destimage[thisPos].r = qMin(qMax((int) (fraction * (float) color.red() + (blend ? (negVal * (float) sourceImg[thisPos].r) : 0)), 0), 255);
         destimage[thisPos].g = qMin(qMax((int) (fraction * (float) color.green() + (blend ? (negVal * (float) sourceImg[thisPos].g) : 0)), 0), 255);
         destimage[thisPos].b = qMin(qMax((int) (fraction * (float) color.blue() + (blend ? (negVal * (float) sourceImg[thisPos].b) : 0)), 0), 255);
         destimage[thisPos].a = 255;
      }
   }
}
