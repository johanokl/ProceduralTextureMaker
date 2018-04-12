/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef GAUSSIANBLURTEXTUREGENERATOR_H
#define GAUSSIANBLURTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The GaussianBlurTextureGenerator class
 */
class GaussianBlurTextureGenerator : public TextureGenerator
{
public:
   GaussianBlurTextureGenerator();
   ~GaussianBlurTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Gaussian blur"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blurs the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
   float* ComputeGaussianKernel(const int inRadius, const float inWeight) const;
};

#endif // GAUSSIANBLURTEXTUREGENERATOR_H
