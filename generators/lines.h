
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef LINESTEXTUREGENERATOR_H
#define LINESTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The LinesTextureGenerator class
class LinesTextureGenerator : public TextureGenerator {
public:
   LinesTextureGenerator();
   ~LinesTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QStringList getSourceSlots() const override { return {QStringLiteral("Canvas")}; }
   QString getName() const override { return QString("Lines"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override {
      return QString("Draws evenly spaced parallel lines over an optional input image.");
   }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // LINESTEXTUREGENERATOR_H
