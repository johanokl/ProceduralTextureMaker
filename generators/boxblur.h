/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef BOXBLURTEXTUREGENERATOR_H
#define BOXBLURTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The BoxBlurTextureGenerator class
 */
class BoxBlurTextureGenerator : public TextureGenerator
{
public:
   BoxBlurTextureGenerator();
   virtual ~BoxBlurTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Box blur"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Blurs the source image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // BOXBLURTEXTUREGENERATOR_H
