/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SHADOWTEXTUREGENERATOR_H
#define SHADOWTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The ShadowTextureGenerator class
 */
class ShadowTextureGenerator : public TextureGenerator
{
public:
   ShadowTextureGenerator();
   ~ShadowTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Shadow"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Adds a shadow under the object."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // SHADOWTEXTUREGENERATOR_H
