/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MERGETEXTUREGENERATOR_H
#define MERGETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The MergeTextureGenerator class
 */
class MergeTextureGenerator : public TextureGenerator
{
public:
   MergeTextureGenerator() = default;
   ~MergeTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 10; }
   QString getName() const override { return QString("Merge"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Merge multiple textures."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
};

#endif // MERGETEXTUREGENERATOR_H


