
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef LENSTEXTUREGENERATOR_H
#define LENSTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The LensTextureGenerator class
class LensTextureGenerator : public TextureGenerator {
public:
   LensTextureGenerator();
   ~LensTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Lens"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Warps the input image through a configurable circular lens.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // LENSTEXTUREGENERATOR_H
