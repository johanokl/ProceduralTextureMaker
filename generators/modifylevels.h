
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef MODIFYLEVELSTEXTUREGENERATOR_H
#define MODIFYLEVELSTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The ModifyLevelsTextureGenerator class
class ModifyLevelsTextureGenerator : public TextureGenerator {
public:
   ModifyLevelsTextureGenerator();
   ~ModifyLevelsTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Modify levels"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Modifies the different."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // MODIFYLEVELSTEXTUREGENERATOR_H
