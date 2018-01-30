/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MIRRORTEXTUREGENERATOR_H
#define MIRRORTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The MirrorTextureGenerator class
 */
class MirrorTextureGenerator : public TextureGenerator
{
public:
   MirrorTextureGenerator();
   virtual ~MirrorTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Mirror"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Blurs the source image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};


#endif // MIRRORTEXTUREGENERATOR_H
