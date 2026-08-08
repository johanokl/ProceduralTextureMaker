
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef INVERTTEXTUREGENERATOR_H
#define INVERTTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The InvertTextureGenerator class
class InvertTextureGenerator : public TextureGenerator {
public:
   InvertTextureGenerator();
   ~InvertTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Image")}; }
   QString getName() const override { return QString("Invert"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Selectively inverts the red, green, blue, and alpha channels.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // INVERTTEXTUREGENERATOR_H
