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
   virtual ~CircleTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Circle"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws a filled circle."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // CIRCLETEXTUREGENERATOR_H
