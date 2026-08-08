
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef GREYSCALETEXTUREGENERATOR_H
#define GREYSCALETEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The GreyscaleTextureGenerator class
class GreyscaleTextureGenerator : public TextureGenerator {
public:
   GreyscaleTextureGenerator() = default;
   ~GreyscaleTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Greyscale"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Converts the input image to greyscale while preserving alpha.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // GREYSCALETEXTUREGENERATOR_H
