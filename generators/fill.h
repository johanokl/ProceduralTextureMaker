/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef FILLTEXTUREGENERATOR_H
#define FILLTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The FillTextureGenerator class
 */
class FillTextureGenerator : public TextureGenerator
{
public:
   FillTextureGenerator();
   virtual ~FillTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 0; }
   virtual QString getName() const { return QString("Fill"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Fills the whole texture with the specified colour."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // FILLTEXTUREGENERATOR_H
