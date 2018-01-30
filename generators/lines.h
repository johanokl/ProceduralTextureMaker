/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef LINESTEXTUREGENERATOR_H
#define LINESTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The LinesTextureGenerator class
 */
class LinesTextureGenerator : public TextureGenerator
{
public:
   LinesTextureGenerator();
   virtual ~LinesTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 1; }
   virtual QString getName() const { return QString("Lines"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString("Draws filled lines."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings configurables;
};

#endif // LINESTEXTUREGENERATOR_H
