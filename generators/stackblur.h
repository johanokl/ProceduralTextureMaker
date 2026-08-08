
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef STACKBLURTEXTUREGENERATOR_H
#define STACKBLURTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The StackBlurTextureGenerator class
class StackBlurTextureGenerator : public TextureGenerator {
public:
   StackBlurTextureGenerator();
   ~StackBlurTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Image")}; }
   QString getName() const override { return QString("Stack Blur"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Softens the input image using the fast stack-blur algorithm.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // STACKBLURTEXTUREGENERATOR_H
