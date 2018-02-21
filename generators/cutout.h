/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CUTOUTTEXTUREGENERATOR_H
#define CUTOUTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The CutoutTextureGenerator class
 */
class CutoutTextureGenerator : public TextureGenerator
{
public:
   CutoutTextureGenerator();
   virtual ~CutoutTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 2; }
   virtual QString getName() const { return QString("Cutout"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Cut alpha from one image using another."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
};

#endif // CUTOUTTEXTUREGENERATOR_H


