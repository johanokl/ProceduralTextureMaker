// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include "base/textureimage.h"
#include "global.h"
#include <QMap>
#include <QSharedPointer>
#include <QStringList>

/// @brief Language-independent texture-generator contract used by the graph and renderer.
class TextureGenerator {
public:
   /// @brief Identifies how a generator uses input images.
   enum class Type {
      /// @brief Transforms one input image.
      Filter,
      /// @brief Combines two or more input images.
      Combiner,
      /// @brief Produces an image without requiring an input image.
      Generator
   };

   /// @brief Identifies whether a generator ships with the application or was supplied by a user.
   enum class Origin {
      /// @brief Generator definition compiled into the application.
      BuiltIn,
      /// @brief Generator definition discovered in a user-configured directory.
      Custom
   };

   /// @brief Destroys the texture generator.
   virtual ~TextureGenerator() = default;

   /// @brief Generates an image using the supplied source images and settings.
   /// @param size Width and height of the destination and source images.
   /// @param destimage Writable destination pixel buffer containing `size.width() * size.height()`
   /// pixels.
   /// @param sourceimages Source images keyed by the names returned from getSourceSlots().
   /// @param settings Current values for the settings returned from getSettings().
   virtual void generate(QSize size, TexturePixel* destimage,
                         const QMap<QString, TextureImagePtr>& sourceimages,
                         const TextureNodeSettings& settings) const = 0;

   /// @brief Gets the configurable settings exposed by the generator.
   /// @return Setting definitions keyed by their stable identifiers.
   virtual const TextureGeneratorSettings& getSettings() const = 0;

   /// @brief Gets the generator category used by the add-node panel.
   /// @return The generator's filter, combiner, or source-generator category.
   virtual Type getType() const = 0;

   /// @brief Returns whether this definition ships with the application or was loaded by a user.
   /// @return The origin of the generator definition.
   virtual Origin getOrigin() const { return Origin::BuiltIn; }

   /// @brief Returns a stable source identity used in diagnostics and reload collision messages.
   /// @return A source path, resource URL, diagnostic label, or an empty string for C++ generators.
   virtual QString getSourceIdentity() const { return QString(); }

   /// @brief Returns the ordered, stable names of the generator's input slots.
   /// @return Input slot names in serialization and presentation order.
   virtual QStringList getSourceSlots() const { return {QStringLiteral("Input")}; }

   /// @brief Gets the public name used to register and display the generator.
   /// @return The unique generator name.
   virtual QString getName() const = 0;

   /// @brief Gets the user-facing description of the generator.
   /// @return A description suitable for the generator information panel.
   virtual QString getDescription() const = 0;

   /// @brief Resolves a canonical or legacy serialized slot identifier.
   /// @param serializedSlot A current slot name, zero-based numeric index, or legacy `Slot N` name.
   /// @return The canonical slot name, or an empty string if the identifier cannot be resolved.
   QString resolveSourceSlot(const QString& serializedSlot) const;
};

/// @brief Shared ownership pointer used for registered texture generators.
using TextureGeneratorPtr = QSharedPointer<TextureGenerator>;

#endif  // TEXTUREGENERATOR_H
