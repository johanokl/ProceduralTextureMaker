
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SHADOWTEXTUREGENERATOR_H
#define SHADOWTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The ShadowTextureGenerator class
class ShadowTextureGenerator : public TextureGenerator {
public:
   ShadowTextureGenerator();
   ~ShadowTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Shadow"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Adds a coloured, blurred, and offset shadow behind the input image.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // SHADOWTEXTUREGENERATOR_H
