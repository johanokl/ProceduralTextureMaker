
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef NOISETEXTUREGENERATOR_H
#define NOISETEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The NoiseTextureGenerator class
class NoiseTextureGenerator : public TextureGenerator {
public:
   NoiseTextureGenerator();
   ~NoiseTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Background")}; }
   QString getName() const override { return QString("Noise"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Generates coloured random noise over an optional input image.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // NOISETEXTUREGENERATOR_H
