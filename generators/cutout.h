/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CUTOUTTEXTUREGENERATOR_H
#define CUTOUTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The CutoutTextureGenerator class
 */
class CutoutTextureGenerator : public TextureGenerator
{
public:
   CutoutTextureGenerator();
   ~CutoutTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 2; }
   QString getName() const override { return QString("Cutout"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Cut alpha from one image using another."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
};

#endif // CUTOUTTEXTUREGENERATOR_H


