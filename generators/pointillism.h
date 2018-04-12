/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef POINTILLISMTEXTUREGENERATOR_H
#define POINTILLISMTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The PointillismTextureGenerator class
 */
class PointillismTextureGenerator : public TextureGenerator
{
public:
   PointillismTextureGenerator();
   ~PointillismTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Pointillism"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString(""); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // POINTILLISMTEXTUREGENERATOR_H
