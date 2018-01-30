/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TRANSFORMTEXTUREGENERATOR_H
#define TRANSFORMTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The TransformTextureGenerator class
 */
class TransformTextureGenerator : public TextureGenerator
{
public:
   TransformTextureGenerator();
   virtual ~TransformTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Transform"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Resizes/translates/rotates."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};


#endif // TRANSFORMTEXTUREGENERATOR_H
