/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef BRICKSTEXTUREGENERATOR_H
#define BRICKSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The BricksTextureGenerator class
 */
class BricksTextureGenerator : public TextureGenerator
{
public:
   BricksTextureGenerator();
   ~BricksTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Bricks"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws rectangle bricks"); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // BRICKSTEXTUREGENERATOR_H
