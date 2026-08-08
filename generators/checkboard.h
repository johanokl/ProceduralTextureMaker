
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef CHECKBOARDTEXTUREGENERATOR_H
#define CHECKBOARDTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The CheckboardTextureGenerator class
class CheckboardTextureGenerator : public TextureGenerator {
public:
   CheckboardTextureGenerator();
   ~CheckboardTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Checkboard"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Draws a chequerboard pattern over an optional input image.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // CHECKBOARDTEXTUREGENERATOR_H
