/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef DISPLACEMENTMAPTEXTUREGENERATOR_H
#define DISPLACEMENTMAPTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The DisplacementMapTextureGenerator class
 */
class DisplacementMapTextureGenerator : public TextureGenerator
{
public:
   DisplacementMapTextureGenerator();
   virtual ~DisplacementMapTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 2; }
   virtual QString getSlotName(int num) {
      if (num == 1) return QString("Map");
      return QString("Source image");
   }
   virtual QString getName() const { return QString("Displacement"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const {
      return QString("Transforms the image based on a gray-scale transform map.");
   }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // DISPLACEMENTMAPTEXTUREGENERATOR_H
