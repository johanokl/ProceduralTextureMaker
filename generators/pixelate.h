/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PIXELATETEXTUREGENERATOR_H
#define PIXELATETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The PixelateTextureGenerator class
 */
class PixelateTextureGenerator : public TextureGenerator
{
public:
   PixelateTextureGenerator();
   ~PixelateTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Pixelate"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Pixelates the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // PIXELATETEXTUREGENERATOR_H
