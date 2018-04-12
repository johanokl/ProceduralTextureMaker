/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef BLENDINGTEXTUREGENERATOR_H
#define BLENDINGTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The BlendingTextureGenerator class
 */
class BlendingTextureGenerator : public TextureGenerator
{
public:
   enum BlendModes {
      Normal = 1,
      Darken = 2,
      Multiply = 3,
      Lighten = 4,
      Screen = 5,
      ColorDodge = 6,
      ColorBurn = 7,
      Overlay = 8,
      SoftLight = 9,
      HardLight = 10,
      Difference = 11,
      Exclusion = 12
   };

   BlendingTextureGenerator();
   ~BlendingTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 2; }
   QString getName() const override { return QString("Blending"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blends two textures together."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
   double blendColors(BlendModes mode, double originColor, double addColor) const;
   int alphaCompose(double originAlpha, double addAlpha, double compositeAlpha,
                    double originColor, double addColor, double compositeColor) const;
};

#endif // BLENDINGTEXTUREGENERATOR_H


