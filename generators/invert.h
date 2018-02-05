/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef INVERTTEXTUREGENERATOR_H
#define INVERTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The InvertTextureGenerator class
 */
class InvertTextureGenerator : public TextureGenerator
{
public:
   InvertTextureGenerator();
   virtual ~InvertTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Invert"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Inverts the colors."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Filter; }

private:
   TextureGeneratorSettings configurables;
};

#endif // INVERTTEXTUREGENERATOR_H
