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
   virtual ~WhirlTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Whirl"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Twirls the image."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // WHIRLTEXTUREGENERATOR_H
