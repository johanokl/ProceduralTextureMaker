/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef EMPTYGENERATOR_H
#define EMPTYGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The EmptyGenerator class
 */
class EmptyGenerator : public TextureGenerator
{
public:
   EmptyGenerator() {}
   virtual ~EmptyGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 3; }
   virtual QString getName() const { return QString("Empty"); }
   virtual const TextureGeneratorSettings& getSettings() const { return _settings; }
   virtual QString getDescription() const { return QString("Empty generator."); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings _settings;
};

#endif // EMPTYGENERATOR_H
