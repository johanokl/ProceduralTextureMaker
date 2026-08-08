
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef DISPLACEMENTMAPTEXTUREGENERATOR_H
#define DISPLACEMENTMAPTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The DisplacementMapTextureGenerator class
class DisplacementMapTextureGenerator : public TextureGenerator {
public:
   DisplacementMapTextureGenerator();
   ~DisplacementMapTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override {
      return {QStringLiteral("Source image"), QStringLiteral("Map")};
   }
   QString getName() const override { return QString("Displacement"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Distorts the source image using the luminance of a displacement map.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // DISPLACEMENTMAPTEXTUREGENERATOR_H
