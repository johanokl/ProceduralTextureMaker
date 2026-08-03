
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
   QString getName() const override { return QString("Invert"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Inverts the colors."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // INVERTTEXTUREGENERATOR_H
