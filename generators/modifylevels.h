/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MODIFYLEVELSTEXTUREGENERATOR_H
#define MODIFYLEVELSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The ModifyLevelsTextureGenerator class
 */
class ModifyLevelsTextureGenerator : public TextureGenerator
{
public:
   ModifyLevelsTextureGenerator();
   ~ModifyLevelsTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Modify levels"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Modifies the different."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // MODIFYLEVELSTEXTUREGENERATOR_H
