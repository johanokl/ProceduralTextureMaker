
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef WHIRLTEXTUREGENERATOR_H
#define WHIRLTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The WhirlTextureGenerator class
class WhirlTextureGenerator : public TextureGenerator {
public:
   WhirlTextureGenerator();
   ~WhirlTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Whirl"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Twirls the image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // WHIRLTEXTUREGENERATOR_H
