// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef JSTEXGEN_H
#define JSTEXGEN_H

#include "base/texturegenerator.h"
#include <QByteArray>
#include <QString>
#include <atomic>
#include <memory>

/// @brief Adapts a validated JavaScript texture-generator definition to TextureGenerator.
class JsTexGen final : public TextureGenerator {
public:
   /// @brief Parses and validates a JavaScript generator definition.
   /// @param jsContent Complete JavaScript source.
   /// @param sourceIdentity Canonical filesystem path, resource URL, or diagnostic label.
   /// @param origin Whether the trusted loader treats the definition as built-in or custom.
   explicit JsTexGen(QString jsContent, QString sourceIdentity = QStringLiteral("<memory>"),
                     Origin origin = Origin::Custom);

   /// @brief Releases the validated definition and its runtime-cache lifetime token.
   ~JsTexGen() override;

   /// @brief Executes the JavaScript generator for one destination image.
   /// @param size Width and height of the destination and source images.
   /// @param destimage Writable destination pixel buffer.
   /// @param sourceimages Source images keyed by the script's declared input slots.
   /// @param settings Current generator setting values.
   void generate(QSize size, TexturePixel* destimage,
                 const QMap<QString, TextureImagePtr>& sourceimages,
                 const TextureNodeSettings& settings) const override;

   /// @brief Gets the ordered input slots declared by the script.
   /// @return Stable input slot names.
   QStringList getSourceSlots() const override { return inputSlots; }

   /// @brief Gets the public generator name declared by the script.
   /// @return The validated generator name.
   QString getName() const override { return name; }

   /// @brief Gets the configurable settings declared by the script.
   /// @return Validated setting definitions in descriptor order, each with a stable unique ID.
   const TextureGeneratorSettings& getSettings() const override { return configurables; }

   /// @brief Gets the user-facing description declared by the script.
   /// @return The generator description.
   QString getDescription() const override { return description; }

   /// @brief Gets the generator category declared by the script.
   /// @return The filter, combiner, or source-generator category.
   Type getType() const override { return type; }

   /// @brief Gets the trusted origin assigned by the loader.
   /// @return Whether the definition is bundled or custom.
   Origin getOrigin() const override { return origin; }

   /// @brief Gets the path or resource URL used to load the script.
   /// @return The stable source identity used in diagnostics and reloads.
   QString getSourceIdentity() const override { return sourceIdentity; }

   /// @brief Reports whether validation accepted the script.
   /// @return @c true when the definition can generate images.
   bool isValid() const noexcept { return valid; }

   /// @brief Returns the complete validation diagnostic when isValid() is false.
   /// @return An empty string for a valid script, otherwise its validation error.
   QString validationError() const { return diagnostic; }

   /// @brief Returns the supported descriptor API version.
   /// @return Always 1.
   int apiVersion() const noexcept { return 1; }

   /// @brief Returns a SHA-256 content revision used by reload and runtime caches.
   /// @return The digest of the original JavaScript source.
   QByteArray contentRevision() const { return revision; }

   /// @brief Returns the original source so bundled definitions can be viewed or copied.
   /// @return The complete JavaScript source supplied to the constructor.
   QString source() const { return scriptContent; }

   /// @brief Interrupts JavaScript currently executing on any render worker.
   static void interruptActiveEngines();

   /// @brief Returns the number of descriptor runtime evaluations, for diagnostics and tests.
   /// @return The process-wide count of descriptor programs evaluated by render workers.
   static quint64 runtimeEvaluationCount() noexcept;

private:
   /// @brief Evaluates the definition in an isolated engine and records validated metadata.
   void validate();

   /// @brief Executes a descriptor-API generator using reusable per-thread JavaScript state.
   /// @param size Width and height of all image buffers.
   /// @param destimage Writable destination pixel buffer.
   /// @param sourceimages Source images keyed by declared input slot.
   /// @param settings Current generator setting values.
   void generateDescriptor(QSize size, TexturePixel* destimage,
                           const QMap<QString, TextureImagePtr>& sourceimages,
                           const TextureNodeSettings& settings) const;

   /// @brief Validated setting definitions in descriptor order.
   TextureGeneratorSettings configurables;
   /// @brief Public generator name parsed from the definition.
   QString name;
   /// @brief User-facing generator description parsed from the definition.
   QString description;
   /// @brief Original JavaScript source retained for runtime evaluation and inspection.
   QString scriptContent;
   /// @brief Stable path, resource URL, or diagnostic label for the source.
   QString sourceIdentity;
   /// @brief Validation error, or an empty string for a valid definition.
   QString diagnostic;
   /// @brief Ordered input slot names declared by the definition.
   QStringList inputSlots;
   /// @brief SHA-256 digest of scriptContent.
   QByteArray revision;
   /// @brief Token whose expiration invalidates per-worker runtime cache entries.
   std::shared_ptr<void> lifetimeToken;
   /// @brief Process-unique identifier used as part of the runtime cache key.
   quint64 stableId = 0;
   /// @brief Validated add-node category.
   Type type = Type::Generator;
   /// @brief Trusted origin assigned by the loader rather than by script content.
   Origin origin = Origin::Custom;
   /// @brief Whether validation completed successfully.
   bool valid = false;

   /// @brief Supplies process-unique IDs for runtime cache keys.
   static std::atomic<quint64> nextStableId;
   /// @brief Counts descriptor evaluations performed by render-worker runtimes.
   static std::atomic<quint64> evaluationCount;
};

#endif  // JSTEXGEN_H
