/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MERGETEXTUREGENERATOR_H
#define MERGETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The MergeTextureGenerator class
 */
class MergeTextureGenerator : public TextureGenerator
{
public:
   MergeTextureGenerator();
   virtual ~MergeTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 10; }
   virtual QString getName() const { return QString("Merge"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Merge multiple textures."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
};

#endif // MERGETEXTUREGENERATOR_H


