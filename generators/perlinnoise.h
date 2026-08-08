
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PERLINNOISETEXTUREGENERATOR_H
#define PERLINNOISETEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The PerlinNoiseTextureGenerator class
class PerlinNoiseTextureGenerator : public TextureGenerator {
public:
   PerlinNoiseTextureGenerator();
   ~PerlinNoiseTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getName() const override { return "Perlin noise"; }
   QString getDescription() const override {
      return QString("Generates multi-octave Perlin noise tinted with a selected colour.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   double noise(double x, double y) const;
   double findnoise2(double x, double y) const;
   double interpolate(double a, double b, double x) const;

   TextureGeneratorSettings configurables;
};

#endif  // PERLINNOISETEXTUREGENERATOR_H
