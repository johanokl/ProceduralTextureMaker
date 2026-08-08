
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef POINTILLISMTEXTUREGENERATOR_H
#define POINTILLISMTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The PointillismTextureGenerator class
class PointillismTextureGenerator : public TextureGenerator {
public:
   PointillismTextureGenerator();
   ~PointillismTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Image")}; }
   QString getName() const override { return QString("Pointillism"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Recreates the input image with randomly placed coloured ellipses.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // POINTILLISMTEXTUREGENERATOR_H
