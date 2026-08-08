
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef GRADIENTTEXTUREGENERATOR_H
#define GRADIENTTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The GradientTextureGenerator class
class GradientTextureGenerator : public TextureGenerator {
public:
   GradientTextureGenerator();
   ~GradientTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Gradient"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Generates a linear, radial, or conical three-colour gradient.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // GRADIENTTEXTUREGENERATOR_H
