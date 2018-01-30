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
   virtual ~GaussianBlurTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Gaussian blur"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Blurs the source image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
   float* ComputeGaussianKernel(const int inRadius, const float inWeight) const;
};

#endif // GAUSSIANBLURTEXTUREGENERATOR_H
