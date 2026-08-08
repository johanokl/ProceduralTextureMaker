
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef STARTEXTUREGENERATOR_H
#define STARTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The StarTextureGenerator class
class StarTextureGenerator : public TextureGenerator {
public:
   StarTextureGenerator();
   ~StarTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Star"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Draws a configurable multi-pointed star with an optional cut-out.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // STARTEXTUREGENERATOR_H
