/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MODIFYALPHATEXTUREGENERATOR_H
#define MODIFYALPHATEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The ModifyAlphaTextureGenerator class
 */
class ModifyAlphaTextureGenerator : public TextureGenerator
{
public:
   ModifyAlphaTextureGenerator();
   virtual ~ModifyAlphaTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Modify alpha"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Modifies the alpha."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // MODIFYALPHATEXTUREGENERATOR_H
