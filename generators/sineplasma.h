
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SINEPLASMATEXTUREGENERATOR_H
#define SINEPLASMATEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The SinePlasmaTextureGenerator class
class SinePlasmaTextureGenerator : public TextureGenerator {
public:
   SinePlasmaTextureGenerator();
   ~SinePlasmaTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Background")}; }
   QString getName() const override { return QString("Sine plasma"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Generates a repeating two-dimensional sine-wave plasma.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // SINEPLASMATEXTUREGENERATOR_H
