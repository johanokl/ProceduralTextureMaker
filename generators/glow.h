/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef GLOWTEXTUREGENERATOR_H
#define GLOWTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The GlowTextureGenerator class
 */
class GlowTextureGenerator : public TextureGenerator
{
public:
   GlowTextureGenerator();
   virtual ~GlowTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Glow"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Adds a glow around the object."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // GLOWTEXTUREGENERATOR_H
