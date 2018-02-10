/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTTEXTUREGENERATOR_H
#define TEXTTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The TextTextureGenerator class
 */
class TextTextureGenerator : public TextureGenerator
{
public:
   TextTextureGenerator();
   virtual ~TextTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Text"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws a Text."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // TEXTTEXTUREGENERATOR_H
