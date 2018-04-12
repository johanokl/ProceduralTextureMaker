/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TRANSFORMTEXTUREGENERATOR_H
#define TRANSFORMTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The TransformTextureGenerator class
 */
class TransformTextureGenerator : public TextureGenerator
{
public:
   TransformTextureGenerator();
   ~TransformTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Transform"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Resizes/translates/rotates."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};


#endif // TRANSFORMTEXTUREGENERATOR_H
