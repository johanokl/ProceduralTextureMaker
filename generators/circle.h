/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CIRCLETEXTUREGENERATOR_H
#define CIRCLETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The CircleTextureGenerator class
 */
class CircleTextureGenerator : public TextureGenerator
{
public:
   CircleTextureGenerator();
   ~CircleTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Circle"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws a filled circle."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // CIRCLETEXTUREGENERATOR_H
