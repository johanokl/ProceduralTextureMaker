/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef DROPSHADOWTEXTUREGENERATOR_H
#define DROPSHADOWTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The DropShadowTextureGenerator class
 */
class DropShadowTextureGenerator : public TextureGenerator
{
public:
   DropShadowTextureGenerator();
   virtual ~DropShadowTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Shadow"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Adds a shadow under the object."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // DROPSHADOWTEXTUREGENERATOR_H
