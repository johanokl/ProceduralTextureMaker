/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef STACKBLURTEXTUREGENERATOR_H
#define STACKBLURTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The StackBlurTextureGenerator class
 */
class StackBlurTextureGenerator : public TextureGenerator
{
public:
   StackBlurTextureGenerator();
   ~StackBlurTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Stack Blur"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blurs the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // STACKBLURTEXTUREGENERATOR_H
