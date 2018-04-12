/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef LENSTEXTUREGENERATOR_H
#define LENSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The LensTextureGenerator class
 */
class LensTextureGenerator : public TextureGenerator
{
public:
   LensTextureGenerator();
   ~LensTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Lens"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Applies a lens effect."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // LENSTEXTUREGENERATOR_H
