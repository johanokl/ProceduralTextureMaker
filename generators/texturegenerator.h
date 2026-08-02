
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include "base/textureimage.h"
#include "global.h"
#include <QMap>
#include <QStringList>
class TextureImage;

/// @brief The TextureGenerator class
/// Pure abstract class for TextureGenerator.
class TextureGenerator {
public:
   enum class Type { Filter, Combiner, Generator };

   virtual ~TextureGenerator() = default;
   virtual void generate(QSize size, TexturePixel* destimage,
                         QMap<QString, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const = 0;
   virtual const TextureGeneratorSettings& getSettings() const = 0;
   virtual Type getType() const = 0;
   /// @brief Returns the ordered, stable names of the generator's input slots.
   /// @details The order controls automatic connection placement and legacy numeric-slot imports.
   /// Slot names must be non-empty and unique. One-input generators use `Input` by default.
   virtual QStringList getSourceSlots() const { return {QStringLiteral("Input")}; }
   virtual QString getName() const = 0;
   virtual QString getDescription() const = 0;

   /// @brief Resolves a canonical or legacy serialized slot identifier.
   /// @param serializedSlot Canonical name, zero-based numeric slot, or one-based `Slot N` label.
   /// @return Canonical slot name, or a null string when the identifier is invalid.
   QString resolveSourceSlot(const QString& serializedSlot) const;
};

/// @brief Thread-safe smart pointer for TextureGenerator instances.
typedef QSharedPointer<TextureGenerator> TextureGeneratorPtr;

#endif  // TEXTUREGENERATOR_H
