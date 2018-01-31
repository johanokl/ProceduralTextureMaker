/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SINEPLASMATEXTUREGENERATOR_H
#define SINEPLASMATEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The SinePlasmaTextureGenerator class
 */
class SinePlasmaTextureGenerator : public TextureGenerator
{
public:
   SinePlasmaTextureGenerator();
   virtual ~SinePlasmaTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Sine plasma"); }
   virtual const TextureGeneratorSettings& getSettings()  const { return configurables; }
   virtual QString getDescription() const { return QString("Repeating sine plasma."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // SINEPLASMATEXTUREGENERATOR_H
