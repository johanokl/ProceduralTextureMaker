/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PIXELATETEXTUREGENERATOR_H
#define PIXELATETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The PixelateTextureGenerator class
 */
class PixelateTextureGenerator : public TextureGenerator
{
public:
   PixelateTextureGenerator();
   virtual ~PixelateTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Pixelate"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Pixelates the source image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // PIXELATETEXTUREGENERATOR_H
