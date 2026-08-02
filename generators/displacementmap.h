
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef DISPLACEMENTMAPTEXTUREGENERATOR_H
#define DISPLACEMENTMAPTEXTUREGENERATOR_H

#include "texturegenerator.h"

/// @brief The DisplacementMapTextureGenerator class
class DisplacementMapTextureGenerator : public TextureGenerator {
public:
   DisplacementMapTextureGenerator();
   ~DisplacementMapTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage, QMap<QString, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   QStringList getSourceSlots() const override {
      return {QStringLiteral("Source image"), QStringLiteral("Map")};
   }
   QString getName() const override { return QString("Displacement"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Transforms the image based on a gray-scale transform map.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // DISPLACEMENTMAPTEXTUREGENERATOR_H
