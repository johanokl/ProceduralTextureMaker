/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef LINESTEXTUREGENERATOR_H
#define LINESTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The LinesTextureGenerator class
 */
class LinesTextureGenerator : public TextureGenerator
{
public:
   LinesTextureGenerator();
   ~LinesTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Lines"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Draws filled lines."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // LINESTEXTUREGENERATOR_H
