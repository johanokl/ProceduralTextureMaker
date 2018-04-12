/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef WHIRLTEXTUREGENERATOR_H
#define WHIRLTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The WhirlTextureGenerator class
 */
class WhirlTextureGenerator : public TextureGenerator
{
public:
   WhirlTextureGenerator();
   ~WhirlTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return QString("Whirl"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString("Twirls the image."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // WHIRLTEXTUREGENERATOR_H
