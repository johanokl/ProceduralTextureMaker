
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PIXELATETEXTUREGENERATOR_H
#define PIXELATETEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The PixelateTextureGenerator class
class PixelateTextureGenerator : public TextureGenerator {
public:
   PixelateTextureGenerator();
   ~PixelateTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Pixelate"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Reduces the input image to a grid of rectangular colour blocks.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // PIXELATETEXTUREGENERATOR_H
