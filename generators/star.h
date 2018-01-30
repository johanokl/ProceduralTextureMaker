/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */
#ifndef STARTEXTUREGENERATOR_H
#define STARTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The StarTextureGenerator class
 */
class StarTextureGenerator : public TextureGenerator
{
public:
   StarTextureGenerator();
   virtual ~StarTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Star"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws a star."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // STARTEXTUREGENERATOR_H
