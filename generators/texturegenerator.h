/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include <string>
#include <QMap>
#include <QtGlobal>
#include "global.h"
#include "core/textureimage.h"

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

   virtual ~TextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const = 0;
   virtual const TextureGeneratorSettings& getSettings() const = 0;
   virtual Type getType() const = 0;
   virtual int getNumSourceSlots() const = 0;
   virtual QString getName() const = 0;
   virtual QString getSlotName(int num) { return QString("Slot %1").arg(num + 1); }
   virtual QString getDescription() const = 0;
};

/**
 * @brief TextureGeneratorPtr
 *
 * Thread-safe smart pointer for TextureGenerator
 */
typedef QSharedPointer<TextureGenerator> TextureGeneratorPtr;

#endif // TEXTUREGENERATOR_H
