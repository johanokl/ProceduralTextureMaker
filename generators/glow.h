
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef GLOWTEXTUREGENERATOR_H
#define GLOWTEXTUREGENERATOR_H

#include "base/texturegenerator.h"

/// @brief The GlowTextureGenerator class
class GlowTextureGenerator : public TextureGenerator {
public:
   GlowTextureGenerator();
   ~GlowTextureGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;
   QString getName() const override { return QString("Glow"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Adds a glow around the object."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif  // GLOWTEXTUREGENERATOR_H
