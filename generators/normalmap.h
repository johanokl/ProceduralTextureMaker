
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef NORMALMAPTEXTUREGENERATOR_H
#define NORMALMAPTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The NormalMapTextureGenerator class
class NormalMapTextureGenerator : public TextureGenerator {
public:
   NormalMapTextureGenerator() = default;
   ~NormalMapTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Normal-map"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Converts image luminance into a tangent-space normal map.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // NORMALMAPTEXTUREGENERATOR_H
