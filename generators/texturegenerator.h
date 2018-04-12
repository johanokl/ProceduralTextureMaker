/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include "core/textureimage.h"
#include "global.h"
#include <QMap>

class TextureImage;

/**
 * @brief The TextureGenerator class
 *
 * Pure abstract class for TextureGenerator.
 */
class TextureGenerator
{
public:
   enum class Type { Filter, Combiner, Generator };

   virtual ~TextureGenerator() = default;
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const = 0;
   virtual const TextureGeneratorSettings& getSettings() const = 0;
   virtual Type getType() const = 0;
   virtual int getNumSourceSlots() const = 0;
   virtual QString getName() const = 0;
   virtual QString getSlotName(int id);
   virtual QString getDescription() const = 0;
};

/**
 * @brief TextureGeneratorPtr
 *
 * Thread-safe smart pointer for TextureGenerator
 */
typedef QSharedPointer<TextureGenerator> TextureGeneratorPtr;

#endif // TEXTUREGENERATOR_H
