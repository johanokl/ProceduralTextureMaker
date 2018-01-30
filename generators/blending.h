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
      LinearDodgeAdd = 6,
      Overlay = 7,
      SoftLight = 8
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
   unsigned char blendColors(double originColor, double addColor,
                             double addPixelAlpha, double blendingAlpha,
                             BlendModes mode) const;
};

#endif // BLENDINGTEXTUREGENERATOR_H


