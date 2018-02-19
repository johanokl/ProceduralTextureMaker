/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CHECKBOARDTEXTUREGENERATOR_H
#define CHECKBOARDTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The CheckboardTextureGenerator class
 */
class CheckboardTextureGenerator : public TextureGenerator
{
public:
   CheckboardTextureGenerator();
   virtual ~CheckboardTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Checkboard"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws a checkboard."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // CHECKBOARDTEXTUREGENERATOR_H
