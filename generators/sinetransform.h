/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SINETRANSFORMTEXTUREGENERATOR_H
#define SINETRANSFORMTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The SineTransformTextureGenerator class
 */
class SineTransformTextureGenerator : public TextureGenerator
{
public:
   SineTransformTextureGenerator();
   ~SineTransformTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Sine transform"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Stretches the texture along a sinus wave."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // SINETRANSFORMTEXTUREGENERATOR_H
