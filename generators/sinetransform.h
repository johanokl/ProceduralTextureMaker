
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SINETRANSFORMTEXTUREGENERATOR_H
#define SINETRANSFORMTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The SineTransformTextureGenerator class
class SineTransformTextureGenerator : public TextureGenerator {
public:
   SineTransformTextureGenerator();
   ~SineTransformTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Image")}; }
   QString getName() const override { return QString("Sine transform"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Warps the input image with one or two configurable sine waves.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // SINETRANSFORMTEXTUREGENERATOR_H
