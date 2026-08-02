
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef JSTEXGENMANAGER_H
#define JSTEXGENMANAGER_H

#include "texturegenerator.h"
#include <QObject>
#include <QReadWriteLock>
#include <atomic>
class GeneratorFileFinder;
class JsTexGen;
class TextureProject;
class QThread;

/// @brief Loads JavaScript texture generators synchronously from a directory tree.
/// @param project Project that receives valid generators.
/// @param directory Directory searched recursively for JavaScript files.
/// @return An empty string on success, or a description of the first error encountered.
[[nodiscard]] QString loadJavaScriptGenerators(TextureProject& project, const QString& directory);

/// @brief Discovers JavaScript generator files and registers them with a texture project.
/// @details The manager reads the directory and enabled state from `SettingsManager`, delegates
/// recursive file discovery to a worker thread, and forwards valid scripts to `TextureProject`.
class JSTexGenManager : public QObject {
   Q_OBJECT

public:
   /// @brief Starts a file-finder thread and connects discovered generators to a texture project.
   /// @param project Project that receives discovered generators.
   explicit JSTexGenManager(TextureProject* project);

   /// @brief Cancels any active scan, stops the file-finder thread, and releases the thread.
   ~JSTexGenManager() override;

public slots:
   /// @brief Stores the JavaScript generator directory and scans it when discovery is enabled.
   /// @param path Directory to search recursively.
   /// @param forceScan Whether to scan even when the configured path is unchanged.
   void setDirectory(const QString& path, bool forceScan = false);

   /// @brief Wraps a discovered generator in a shared pointer and requests project registration.
   /// @param generator Newly allocated generator whose ownership is transferred.
   void addGenerator(JsTexGen* generator);

   /// @brief Enables directory scanning and starts the first pending scan when necessary.
   /// @param enabled Whether scanning is enabled.
   void setEnabled(bool enabled);

   /// @brief Reads the enabled flag and generator directory from the project's settings manager.
   void settingsUpdated();

signals:
   /// @brief Emitted when a valid generator is ready to be registered.
   /// @param generator Discovered generator.
   void generatorAdded(TextureGeneratorPtr generator);

   /// @brief Emitted when a generator should be unregistered.
   /// @param generator Generator to remove.
   void generatorRemoved(TextureGeneratorPtr generator);

   /// @brief Requests a recursive scan on the file-finder thread.
   /// @param path Directory to scan.
   void scanDirectory(QString path);

private:
   /// @brief Directory currently configured for recursive scanning.
   QString directoryPath;
   /// @brief Worker object that discovers JavaScript generator files.
   GeneratorFileFinder* filefinder;
   /// @brief Worker thread in which the file finder runs.
   QThread* filefinderthread;
   /// @brief Non-owning project that receives discovered generators.
   TextureProject* project;
   /// @brief Whether JavaScript generator discovery is enabled.
   bool enabled;
   /// @brief Whether the currently configured directory has already been scanned.
   bool hasScannedDirectory;
};

/// @brief Adapts a JavaScript texture-generator script to the C++ generator interface.
class JsTexGen : public TextureGenerator {
public:
   /// @brief Parses a JavaScript generator and extracts its metadata and setting definitions.
   /// @param jsContent Complete JavaScript source text.
   explicit JsTexGen(const QString& jsContent);

   /// @brief Destroys the JavaScript generator.
   ~JsTexGen() override = default;

   /// @brief Runs the script's generate function and copies its result into an image buffer.
   /// @param size Width and height of the requested image.
   /// @param destimage Destination pixel buffer.
   /// @param sourceimages Source images indexed by canonical input-slot name.
   /// @param settings Current generator setting values.
   /// @details Settings are passed as JSON. Images use packed RGBA pixels unless the script
   /// requests separate color channels.
   void generate(QSize size, TexturePixel* destimage, QMap<QString, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;

   /// @brief Returns the ordered source-image slots accepted by the script.
   /// @return Canonical source-slot names.
   QStringList getSourceSlots() const override { return inputSlots; }

   /// @brief Returns the generator name declared by the script.
   /// @return Public generator name.
   QString getName() const override { return name; }

   /// @brief Returns the setting definitions declared by the script.
   /// @return Generator settings keyed by identifier.
   const TextureGeneratorSettings& getSettings() const override { return configurables; }

   /// @brief Returns the generator description declared by the script.
   /// @return Public generator description.
   QString getDescription() const override { return description; }

   /// @brief Returns the generator category exposed to the node palette.
   /// @return The generator category.
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

   /// @brief Reports whether the script defines the required generator API.
   /// @return @c true when the script parsed successfully and defines required functions.
   bool isValid();

private:
   /// @brief Setting definitions parsed from the script.
   TextureGeneratorSettings configurables;
   /// @brief Public generator name parsed from the script.
   QString name;
   /// @brief Public generator description parsed from the script.
   QString description;
   /// @brief JavaScript source evaluated for each generation request.
   QString scriptContent;
   /// @brief Serializes evaluation of this generator instance.
   mutable QReadWriteLock mutex;
   /// @brief Ordered source-image slot names declared by the script.
   QStringList inputSlots;
   /// @brief Whether the script uses the legacy positional `numSlots` input API.
   bool legacyPositionalInputs;
   /// @brief Whether parsing found a valid generator API.
   bool valid;
   /// @brief Whether image arrays contain individual color-channel entries.
   bool separateColorChannels;
};

/// @brief Recursively discovers and validates JavaScript generator files on a worker thread.
class GeneratorFileFinder : public QObject {
   Q_OBJECT

public slots:
   /// @brief Requests cancellation of the current directory scan.
   void abort();

   /// @brief Searches a directory tree for valid JavaScript generators.
   /// @param basepath Directory to search recursively.
   void scanDirectory(QString basepath);

signals:
   /// @brief Emitted after a scan finishes or is cancelled.
   void scanFinished();

   /// @brief Emitted for each valid generator found during a scan.
   /// @param generator Newly allocated generator whose ownership is transferred to the receiver.
   void generatorFound(JsTexGen* generator);

private:
   /// @brief Thread-safe cancellation flag checked while scanning.
   std::atomic_bool aborted = false;
};

#endif  // JSTEXGENMANAGER_H
