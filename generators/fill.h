/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef FILLTEXTUREGENERATOR_H
#define FILLTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The FillTextureGenerator class
 */
class FillTextureGenerator : public TextureGenerator
{
public:
   FillTextureGenerator();
   ~FillTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 0; }
   QString getName() const override { return QString("Fill"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Fills the whole texture with the specified color."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // FILLTEXTUREGENERATOR_H
