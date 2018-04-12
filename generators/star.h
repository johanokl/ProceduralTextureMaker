/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */
#ifndef STARTEXTUREGENERATOR_H
#define STARTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The StarTextureGenerator class
 */
class StarTextureGenerator : public TextureGenerator
{
public:
   StarTextureGenerator();
   ~StarTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Star"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws a star."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // STARTEXTUREGENERATOR_H
