
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef BOXBLURTEXTUREGENERATOR_H
#define BOXBLURTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The BoxBlurTextureGenerator class
class BoxBlurTextureGenerator : public TextureGenerator {
public:
   BoxBlurTextureGenerator();
   ~BoxBlurTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Box blur"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Blurs the source image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // BOXBLURTEXTUREGENERATOR_H
