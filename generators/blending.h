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
   virtual ~BlendingTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 2; }
   virtual QString getName() const { return QString("Blending"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Blends two textures together."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
   double blendColors(BlendModes mode, double originColor, double addColor) const;
   int alphaCompose(double originAlpha, double addAlpha, double compositeAlpha,
                    double originColor, double addColor, double compositeColor) const;
};

#endif // BLENDINGTEXTUREGENERATOR_H


