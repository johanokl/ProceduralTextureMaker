
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef CIRCLETEXTUREGENERATOR_H
#define CIRCLETEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The CircleTextureGenerator class
class CircleTextureGenerator : public TextureGenerator {
public:
   CircleTextureGenerator();
   ~CircleTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Canvas")}; }
   QString getName() const override { return QString("Circle"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Draws a filled circle or ring over an optional input image.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // CIRCLETEXTUREGENERATOR_H
