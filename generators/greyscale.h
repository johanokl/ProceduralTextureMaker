/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef GREYSCALETEXTUREGENERATOR_H
#define GREYSCALETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The GreyscaleTextureGenerator class
 */
class GreyscaleTextureGenerator : public TextureGenerator
{
public:
   GreyscaleTextureGenerator() {}
   virtual ~GreyscaleTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Greyscale"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Blurs the source image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // GREYSCALETEXTUREGENERATOR_H
