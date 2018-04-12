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
   ~MirrorTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Mirror"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blurs the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};


#endif // MIRRORTEXTUREGENERATOR_H
