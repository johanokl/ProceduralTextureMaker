/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef GLOWTEXTUREGENERATOR_H
#define GLOWTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The GlowTextureGenerator class
 */
class GlowTextureGenerator : public TextureGenerator
{
public:
   GlowTextureGenerator();
   ~GlowTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Glow"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Adds a glow around the object."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // GLOWTEXTUREGENERATOR_H
