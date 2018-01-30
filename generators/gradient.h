/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef GRADIENTTEXTUREGENERATOR_H
#define GRADIENTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The GradientTextureGenerator class
 */
class GradientTextureGenerator : public TextureGenerator
{
public:
   GradientTextureGenerator();
   virtual ~GradientTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Gradient"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws a filled Gradient."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // GRADIENTTEXTUREGENERATOR_H
