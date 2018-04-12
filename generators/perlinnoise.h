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
   ~PerlinNoiseTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return "Perlin noise"; }
   QString getDescription() const override { return QString("Basic Perlin Noise"); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   double noise(double x, double y) const;
   double findnoise2(double x,double y) const;
   double interpolate(double a, double b, double x) const;

   TextureGeneratorSettings configurables;
};

#endif // PERLINNOISETEXTUREGENERATOR_H
