// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef JSTEXGENMANAGER_H
#define JSTEXGENMANAGER_H

#include "base/jstexgen.h"
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

class QThread;
class TextureProject;

/// @brief Loads valid custom JavaScript generators recursively and aggregates diagnostics.
/// @param project Project that receives each valid generator.
/// @param directory Root directory searched recursively for `.js` files.
/// @return Empty on success; otherwise one line per rejected or colliding file.
[[nodiscard]] QString loadJavaScriptGenerators(TextureProject& project, const QString& directory);

/// @brief Loads and registers the compiled JavaScript generator catalog.
/// @param project Project that receives each valid bundled generator.
/// @return Validation diagnostics. A production caller should treat any entry as a build failure.
[[nodiscard]] QStringList registerBundledJavaScriptGenerators(TextureProject& project);

/// @brief Coordinates asynchronous custom-generator discovery and atomic definition replacement.
class JsTexGenManager final : public QObject {
   Q_OBJECT

public:
   /// @brief Creates a manager and starts its background file-discovery worker.
   /// @param project Project whose custom generator registry will be maintained.
   explicit JsTexGenManager(TextureProject* project);

   /// @brief Stops the discovery worker and waits for its thread to finish.
   ~JsTexGenManager() override;

public slots:
   /// @brief Sets the custom generator directory and optionally requests a scan.
   /// @param path Directory searched recursively for JavaScript definitions.
   /// @param forceScan Whether to scan even when the normalized path is unchanged.
   void setDirectory(const QString& path, bool forceScan = false);

   /// @brief Enables or disables custom JavaScript generator discovery.
   /// @param enabled Whether custom generators should be loaded and kept registered.
   void setEnabled(bool enabled);

   /// @brief Applies the current JavaScript generator application settings.
   void settingsUpdated();

   /// @brief Requests an asynchronous rescan of the configured directory.
   void reload();

signals:
   /// @brief Requests that the worker scan a directory.
   /// @param path Directory to scan recursively.
   void scanDirectory(QString path);

   /// @brief Reports diagnostics produced by the most recently completed scan.
   /// @param diagnostics Validation, file access, and collision messages.
   void diagnosticsAvailable(QStringList diagnostics);

   /// @brief Reports that a scan and registry update have completed.
   /// @param diagnostics Validation, file access, and collision messages.
   void reloadFinished(QStringList diagnostics);

private slots:
   /// @brief Receives ownership of a validated generator from the discovery worker.
   /// @param generator Heap-allocated generator to stage for the current scan.
   void generatorFound(JsTexGen* generator);

   /// @brief Records a source file rejected by the discovery worker.
   /// @param path Canonical path of the rejected definition.
   /// @param diagnostic Human-readable validation or file access error.
   void generatorRejected(QString path, QString diagnostic);

   /// @brief Commits or discards the staged scan results.
   /// @param encounteredPaths Canonical paths observed by the worker.
   /// @param cancelled Whether the worker stopped before completing the directory.
   void scanFinished(QStringList encounteredPaths, bool cancelled);

private:
   /// @brief Clears staging state and starts a worker scan when the directory is valid.
   void startScan();

   /// @brief Clears all generators and diagnostics staged by the current scan.
   void clearPending();

   /// @brief Unregisters all custom generators currently managed by this instance.
   void removeCustomGenerators();

   /// @brief Atomically reconciles completed scan results with the project registry.
   /// @param encounteredPaths Canonical paths observed during the completed scan.
   void commitScan(const QSet<QString>& encounteredPaths);

   /// @brief Describes a public-name collision between two generator definitions.
   /// @param oldGenerator Generator already registered under the public name.
   /// @param newGenerator Newly discovered generator with the same name.
   /// @return A diagnostic containing both source identities.
   QString collisionDiagnostic(const TextureGeneratorPtr& oldGenerator,
                               const TextureGeneratorPtr& newGenerator) const;

   /// @brief Normalized root directory used for custom generator discovery.
   QString directoryPath;
   /// @brief Non-owning pointer to the worker object living on fileFinderThread.
   QObject* fileFinder{nullptr};
   /// @brief Worker thread used for filesystem traversal and script validation.
   QThread* fileFinderThread{nullptr};
   /// @brief Non-owning project whose generator registry is maintained.
   TextureProject* project{nullptr};
   /// @brief Registered custom generators keyed by canonical source path.
   QMap<QString, TextureGeneratorPtr> activeGenerators;
   /// @brief Valid generators staged by the scan currently being committed.
   QMap<QString, TextureGeneratorPtr> pendingGenerators;
   /// @brief Paths encountered but rejected during the current scan.
   QSet<QString> rejectedPaths;
   /// @brief Diagnostics accumulated during the current scan.
   QStringList pendingDiagnostics;
   /// @brief Whether custom generator discovery is enabled.
   bool enabled{false};
   /// @brief Whether the worker is currently scanning a directory.
   bool scanInProgress{false};
   /// @brief Whether another scan should begin after the current scan stops.
   bool rescanRequested{false};
};

Q_DECLARE_METATYPE(JsTexGen*)

#endif  // JSTEXGENMANAGER_H
