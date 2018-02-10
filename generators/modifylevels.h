/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MODIFYLEVELSTEXTUREGENERATOR_H
#define MODIFYLEVELSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The ModifyLevelsTextureGenerator class
 */
class ModifyLevelsTextureGenerator : public TextureGenerator
{
public:
   ModifyLevelsTextureGenerator();
   virtual ~ModifyLevelsTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Modify levels"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Modifies the different."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // MODIFYLEVELSTEXTUREGENERATOR_H
