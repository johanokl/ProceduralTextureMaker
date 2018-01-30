/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PERLINNOISETEXTUREGENERATOR_H
#define PERLINNOISETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The PerlinNoiseTextureGenerator class
 */
class PerlinNoiseTextureGenerator : public TextureGenerator
{
public:
   PerlinNoiseTextureGenerator();
   virtual ~PerlinNoiseTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return "PerlinNoise"; }
   virtual QString getDescription() const { return QString("Basic Perlin Noise"); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   double noise(double x, double y) const;
   double findnoise2(double x,double y) const;
   double interpolate(double a, double b, double x) const;

   TextureGeneratorSettings configurables;
};

#endif // PERLINNOISETEXTUREGENERATOR_H
