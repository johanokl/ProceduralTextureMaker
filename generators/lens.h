/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef LENSTEXTUREGENERATOR_H
#define LENSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The LensTextureGenerator class
 */
class LensTextureGenerator : public TextureGenerator
{
public:
   LensTextureGenerator();
   virtual ~LensTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Lens"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Applies a lens effect."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // LENSTEXTUREGENERATOR_H
