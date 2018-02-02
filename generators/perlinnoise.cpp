/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <math.h>
#include <QtMath>
#include <QColor>
#include "perlinnoise.h"

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
}


double PerlinNoiseTextureGenerator::findnoise2(double x,double y) const
{
   int n = (int) x + (int) y * 57;
   n = (n << 13) ^ n;
   int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
   return 1.0 - ((double) nn / 1073741824.0);
}


double PerlinNoiseTextureGenerator::noise(double x,double y) const
{
   double floorx = (double) ((int)x);
   double floory = (double) ((int)y);
   double s, t, u, v;
   s = findnoise2(floorx, floory);
   t = findnoise2(floorx + 1, floory);
   u = findnoise2(floorx, floory + 1);
   v = findnoise2(floorx + 1, floory + 1);
   double int1 = interpolate(s, t, x - floorx);
   double int2 = interpolate(u, v, x - floorx);
   return interpolate(int1, int2, y - floory);
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

   double zoom = 75;
   bool blend = false;
   TexturePixel* sourceImg = NULL;
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
            getnoise += noise(((double) x) * frequency / zoom, ((double) y) / zoom * frequency) * amplitude;
         }
         int pixelColor = (int) ((getnoise * 128.0) + 128.0);
         if (pixelColor > 255) {
            pixelColor = 255;
         }
         if (pixelColor < 0) {
            pixelColor = 0;
         }
         double fraction = ((double) pixelColor / 255.0);
         double negVal = 1 - ((double) pixelColor / 255.0);

         destimage[thisPos].r = qMin(qMax((int) (fraction * (float) color.red() + (blend ? (negVal * (float) sourceImg[thisPos].r) : 0)), 0), 255);
         destimage[thisPos].g = qMin(qMax((int) (fraction * (float) color.green() + (blend ? (negVal * (float) sourceImg[thisPos].g) : 0)), 0), 255);
         destimage[thisPos].b = qMin(qMax((int) (fraction * (float) color.blue() + (blend ? (negVal * (float) sourceImg[thisPos].b) : 0)), 0), 255);
      }
   }
}
