
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef GAUSSIANBLURTEXTUREGENERATOR_H
#define GAUSSIANBLURTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The GaussianBlurTextureGenerator class
class GaussianBlurTextureGenerator : public TextureGenerator {
public:
   GaussianBlurTextureGenerator();
   ~GaussianBlurTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Gaussian blur"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blurs the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
   float* ComputeGaussianKernel(const int inRadius, const float inWeight) const;
};

#endif  // GAUSSIANBLURTEXTUREGENERATOR_H
