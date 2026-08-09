// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include "base/textureimage.h"
#include "global.h"
#include <QList>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QStringList>

/// @brief Language-independent texture-generator contract used by the graph and renderer.
class TextureGenerator {
public:
   /// @brief Snapshot of recently completed generation calls.
   struct GenerationTiming {
      /// @brief Average duration of the retained calls in milliseconds.
      double averageMilliseconds{0.0};
      /// @brief Number of calls represented by the average.
      int runCount{0};
   };

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

   /// @brief Calls generate() and records its duration in the rolling timing window.
   /// @param size Width and height of the destination and source images.
   /// @param destimage Writable destination pixel buffer.
   /// @param sourceimages Source images keyed by input-slot name.
   /// @param settings Current generator settings.
   void generateWithTiming(QSize size, TexturePixel* destimage,
                           const QMap<QString, TextureImagePtr>& sourceimages,
                           const TextureNodeSettings& settings) const;

   /// @brief Returns timing data for up to the last ten completed generation calls.
   /// @return A thread-safe snapshot; runCount is zero before the first completed call.
   GenerationTiming getGenerationTiming() const;

   /// @brief Gets the configurable settings exposed by the generator.
   /// @return Setting definitions in presentation order, each with a stable unique ID.
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
   virtual QStringList getSourceSlots() const = 0;

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

private:
   /// @brief Adds a completed call duration to the rolling timing window.
   /// @param elapsedNanoseconds Duration reported by the monotonic timer.
   void recordGenerationTime(qint64 elapsedNanoseconds) const;

   /// @brief Protects generationTimesNanoseconds across render and UI threads.
   mutable QMutex generationTimesMutex;
   /// @brief Durations of the last ten completed generate() calls.
   mutable QList<qint64> generationTimesNanoseconds;
};

/// @brief Finds a generator setting definition by its stable ID.
/// @param settings Ordered setting definitions to search.
/// @param id Case-sensitive setting ID.
/// @return A non-owning pointer to the definition, or null if no definition matches.
const TextureGeneratorSetting* findTextureGeneratorSetting(const TextureGeneratorSettings& settings,
                                                           const QString& id);

/// @brief Validates that generator setting IDs are non-empty and unique.
/// @param settings Ordered setting definitions to validate.
/// @return An empty string when valid, otherwise a diagnostic describing the first invalid ID.
QString validateTextureGeneratorSettings(const TextureGeneratorSettings& settings);

/// @brief Shared ownership pointer used for registered texture generators.
using TextureGeneratorPtr = QSharedPointer<TextureGenerator>;

#endif  // TEXTUREGENERATOR_H
