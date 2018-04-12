/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef NOISETEXTUREGENERATOR_H
#define NOISETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The NoiseTextureGenerator class
 */
class NoiseTextureGenerator : public TextureGenerator
{
public:
   NoiseTextureGenerator();
   ~NoiseTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Noise"); }
   const TextureGeneratorSettings& getSettings()  const override { return configurables; }
   QString getDescription() const override { return QString("Pure noise."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // NOISETEXTUREGENERATOR_H
