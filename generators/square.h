/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SQUARETEXTUREGENERATOR_H
#define SQUARETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The SquareTextureGenerator class
 */
class SquareTextureGenerator : public TextureGenerator
{
public:
   SquareTextureGenerator();
   ~SquareTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Square"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws a square."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // SQUARETEXTUREGENERATOR_H
