/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SINETRANSFORMTEXTUREGENERATOR_H
#define SINETRANSFORMTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The SineTransformTextureGenerator class
 */
class SineTransformTextureGenerator : public TextureGenerator
{
public:
   SineTransformTextureGenerator();
   virtual ~SineTransformTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Sine transform"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Stretches the texture along a sinus wave."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // SINETRANSFORMTEXTUREGENERATOR_H
