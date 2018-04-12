/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTTEXTUREGENERATOR_H
#define TEXTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The TextTextureGenerator class
 */
class TextTextureGenerator : public TextureGenerator
{
public:
   TextTextureGenerator();
   ~TextTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Text"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws a Text."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // TEXTTEXTUREGENERATOR_H
