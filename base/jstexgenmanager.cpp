// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/jstexgenmanager.h"
#include "base/settingsmanager.h"
#include "base/textureproject.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <algorithm>
#include <memory>
#include <utility>

namespace {

/// @brief Produces a generator source label suitable for collision diagnostics.
/// @param generator Generator whose source should be described.
/// @return A source identity, a C++ generator label, or `<unknown>` for a null pointer.
QString generatorSource(const TextureGeneratorPtr& generator) {
   if (generator.isNull()) {
      return QStringLiteral("<unknown>");
   }
   const QString source = generator->getSourceIdentity();
   return source.isEmpty() ? QStringLiteral("built-in C++ generator '%1'").arg(generator->getName())
                           : source;
}

/// @brief Resolves a filesystem path to its canonical or absolute representation.
/// @param path Path to normalize.
/// @return The canonical path when available, otherwise the absolute path.
QString canonicalPath(const QString& path) {
   const QFileInfo info(path);
   const QString canonical = info.canonicalFilePath();
   return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}

/// @brief Finds JavaScript source files below a directory in stable path order.
/// @param directory Root directory to search recursively.
/// @param controller Optional scan cancellation state checked between directory entries.
/// @return Unique canonical paths sorted lexicographically.
QStringList scriptPaths(const QString& directory,
                        const QSharedPointer<ExecutionController>& controller = {}) {
   QStringList paths;
   QDirIterator iterator(directory, QStringList{QStringLiteral("*.js")}, QDir::Files,
                         QDirIterator::Subdirectories);
   while ((!controller || !controller->isCancelled()) && iterator.hasNext()) {
      paths.append(canonicalPath(iterator.next()));
   }
   paths.removeDuplicates();
   std::sort(paths.begin(), paths.end());
   return paths;
}

/// @brief Reads and validates one JavaScript generator definition.
/// @param path Filesystem path or Qt resource URL to read.
/// @param origin Trusted origin assigned to the generator.
/// @param diagnostic Destination for a file access or validation failure.
/// @param controller Optional scan cancellation state, including during validation.
/// @return The validated generator, or null on failure.
std::unique_ptr<JsTexGen> readGenerator(
    const QString& path, const TextureGenerator::Origin origin, QString& diagnostic,
    const QSharedPointer<ExecutionController>& controller = {}) {
   QFile file(path);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      diagnostic =
          QStringLiteral("%1: could not read JavaScript source: %2").arg(path, file.errorString());
      return {};
   }
   const QString source = QString::fromUtf8(file.readAll());
   if (controller && controller->isCancelled()) {
      return {};
   }
   auto generator = std::make_unique<JsTexGen>(source, path, origin, controller);
   if (!generator->isValid()) {
      diagnostic = generator->validationError();
      return {};
   }
   return generator;
}

/// @brief Discovers and validates custom JavaScript generators on a worker thread.
class GeneratorFileFinder final : public QObject {
   Q_OBJECT

public slots:
   /// @brief Scans a directory recursively and emits each validated or rejected definition.
   /// @param directory Root directory to search for `.js` files.
   /// @param controller Cancellation state created before this scan was queued.
   void scanDirectory(const QString& directory, QSharedPointer<ExecutionController> controller) {
      QStringList encountered;
      const QStringList paths = scriptPaths(directory, controller);
      for (const QString& path : paths) {
         if (controller->isCancelled()) {
            emit scanFinished(encountered, true);
            return;
         }
         encountered.append(path);
         QString diagnostic;
         std::unique_ptr<JsTexGen> generator =
             readGenerator(path, TextureGenerator::Origin::Custom, diagnostic, controller);
         if (controller->isCancelled()) {
            emit scanFinished(encountered, true);
            return;
         }
         if (!generator) {
            emit generatorRejected(path, diagnostic);
            continue;
         }
         emit generatorFound(TextureGeneratorPtr(generator.release()));
      }
      emit scanFinished(encountered, controller->isCancelled());
   }

signals:
   /// @brief Transfers a validated generator to the manager thread.
   /// @param generator Shared generator transferred safely through the queued signal.
   void generatorFound(TextureGeneratorPtr generator);

   /// @brief Reports a definition that could not be read or validated.
   /// @param path Canonical path of the rejected source.
   /// @param diagnostic Human-readable failure description.
   void generatorRejected(QString path, QString diagnostic);

   /// @brief Reports that directory discovery completed or was cancelled.
   /// @param encounteredPaths Canonical paths observed before completion or cancellation.
   /// @param cancelled Whether an abort request stopped the scan.
   void scanFinished(QStringList encounteredPaths, bool cancelled);
};

}  // namespace

QStringList registerBundledJavaScriptGenerators(TextureProject& project) {
   Q_INIT_RESOURCE(generators);
   QStringList diagnostics;
   QDir directory(QStringLiteral(":/generators"));
   const QStringList files =
       directory.entryList(QStringList{QStringLiteral("*.js")}, QDir::Files, QDir::Name);
   for (const QString& fileName : files) {
      const QString path = directory.filePath(fileName);
      QString diagnostic;
      std::unique_ptr<JsTexGen> generator =
          readGenerator(path, TextureGenerator::Origin::BuiltIn, diagnostic);
      if (!generator) {
         diagnostics.append(diagnostic);
         continue;
      }
      const TextureGeneratorPtr collision = project.getGenerators().value(generator->getName());
      if (!collision.isNull()) {
         diagnostics.append(QStringLiteral("Duplicate bundled generator '%1': %2 conflicts with %3")
                                .arg(generator->getName(), path, generatorSource(collision)));
         continue;
      }
      project.addGenerator(TextureGeneratorPtr(generator.release()));
   }
   return diagnostics;
}

QString loadJavaScriptGenerators(TextureProject& project, const QString& directory) {
   const QDir sourceDirectory(directory);
   if (!sourceDirectory.exists()) {
      return QStringLiteral("JavaScript generator directory does not exist: %1").arg(directory);
   }
   QStringList diagnostics;
   for (const QString& path : scriptPaths(directory)) {
      QString diagnostic;
      std::unique_ptr<JsTexGen> generator =
          readGenerator(path, TextureGenerator::Origin::Custom, diagnostic);
      if (!generator) {
         diagnostics.append(diagnostic);
         continue;
      }
      const TextureGeneratorPtr collision = project.getGenerators().value(generator->getName());
      if (!collision.isNull()) {
         diagnostics.append(QStringLiteral("Duplicate generator '%1': %2 conflicts with %3")
                                .arg(generator->getName(), path, generatorSource(collision)));
         continue;
      }
      project.addGenerator(TextureGeneratorPtr(generator.release()));
   }
   return diagnostics.join(QLatin1Char('\n'));
}

JsTexGenManager::JsTexGenManager(TextureProject* project) : project(project) {
   qRegisterMetaType<TextureGeneratorPtr>();
   qRegisterMetaType<QSharedPointer<ExecutionController>>();
   auto* finder = new GeneratorFileFinder;
   fileFinderThread = new QThread(this);
   finder->moveToThread(fileFinderThread);
   QObject::connect(this, &JsTexGenManager::scanDirectory, finder,
                    &GeneratorFileFinder::scanDirectory);
   QObject::connect(finder, &GeneratorFileFinder::generatorFound, this,
                    &JsTexGenManager::generatorFound);
   QObject::connect(finder, &GeneratorFileFinder::generatorRejected, this,
                    &JsTexGenManager::generatorRejected);
   QObject::connect(finder, &GeneratorFileFinder::scanFinished, this,
                    &JsTexGenManager::scanFinished);
   QObject::connect(fileFinderThread, &QThread::finished, finder, &QObject::deleteLater);
   fileFinderThread->start();

   if (project != nullptr && project->getSettingsManager() != nullptr) {
      QObject::connect(project->getSettingsManager(), &SettingsManager::settingsUpdated, this,
                       &JsTexGenManager::settingsUpdated);
      settingsUpdated();
   }
}

JsTexGenManager::~JsTexGenManager() {
   if (scanController) {
      scanController->cancel();
   }
   fileFinderThread->quit();
   // JavaScript interruption normally completes promptly; never silently wait forever.
   if (!fileFinderThread->wait(10000)) {
      qFatal("Generator discovery did not stop within 10 seconds");
   }
}

void JsTexGenManager::setDirectory(const QString& path, const bool forceScan) {
   const QString normalized = path.isEmpty() ? QString() : QDir(path).absolutePath();
   if (!forceScan && normalized == directoryPath) {
      return;
   }
   directoryPath = normalized;
   if (enabled) {
      reload();
   }
}

void JsTexGenManager::setEnabled(const bool enabled) {
   if (this->enabled == enabled) {
      return;
   }
   this->enabled = enabled;
   if (enabled) {
      reload();
   } else {
      if (scanController) {
         scanController->cancel();
      }
      removeCustomGenerators();
   }
}

void JsTexGenManager::settingsUpdated() {
   if (project == nullptr || project->getSettingsManager() == nullptr) {
      return;
   }
   const SettingsManager* settings = project->getSettingsManager();
   const QString path = settings->getJSTextureGeneratorsPath();
   const bool shouldEnable = settings->getJSTextureGeneratorsEnabled();
   const bool pathChanged = QDir(path).absolutePath() != directoryPath;
   directoryPath = path.isEmpty() ? QString() : QDir(path).absolutePath();
   if (enabled != shouldEnable) {
      setEnabled(shouldEnable);
   } else if (shouldEnable && pathChanged) {
      reload();
   }
}

void JsTexGenManager::reload() {
   if (!enabled) {
      return;
   }
   if (scanInProgress) {
      rescanRequested = true;
      scanController->cancel();
      return;
   }
   startScan();
}

void JsTexGenManager::startScan() {
   clearPending();
   if (directoryPath.isEmpty() || !QDir(directoryPath).exists()) {
      pendingDiagnostics.append(
          QStringLiteral("JavaScript generator directory does not exist: %1").arg(directoryPath));
      emit diagnosticsAvailable(pendingDiagnostics);
      emit reloadFinished(pendingDiagnostics);
      return;
   }
   scanInProgress = true;
   scanController = QSharedPointer<ExecutionController>::create();
   emit scanDirectory(directoryPath, scanController);
}

void JsTexGenManager::generatorFound(TextureGeneratorPtr generator) {
   pendingGenerators.insert(generator->getSourceIdentity(), generator);
}

void JsTexGenManager::generatorRejected(QString path, QString diagnostic) {
   rejectedPaths.insert(std::move(path));
   pendingDiagnostics.append(std::move(diagnostic));
}

void JsTexGenManager::scanFinished(QStringList encounteredPaths, const bool cancelled) {
   scanInProgress = false;
   if (!cancelled && enabled && scanController && !scanController->isCancelled()) {
      commitScan(QSet<QString>(encounteredPaths.cbegin(), encounteredPaths.cend()));
      emit diagnosticsAvailable(pendingDiagnostics);
      emit reloadFinished(pendingDiagnostics);
   }
   clearPending();
   if (rescanRequested && enabled) {
      rescanRequested = false;
      startScan();
   }
}

void JsTexGenManager::commitScan(const QSet<QString>& encounteredPaths) {
   const QList<QString> activePaths = activeGenerators.keys();
   for (const QString& path : activePaths) {
      const TextureGeneratorPtr oldGenerator = activeGenerators.value(path);
      if (pendingGenerators.contains(path)) {
         const TextureGeneratorPtr newGenerator = pendingGenerators.take(path);
         auto* oldJs = dynamic_cast<JsTexGen*>(oldGenerator.data());
         auto* newJs = dynamic_cast<JsTexGen*>(newGenerator.data());
         if (oldJs != nullptr && newJs != nullptr &&
             oldJs->contentRevision() == newJs->contentRevision()) {
            continue;
         }
         const TextureGeneratorPtr collision =
             project->getGenerators().value(newGenerator->getName());
         if (!collision.isNull() && collision != oldGenerator) {
            pendingDiagnostics.append(collisionDiagnostic(collision, newGenerator));
            continue;
         }
         if (project->replaceGenerator(oldGenerator, newGenerator)) {
            activeGenerators.insert(path, newGenerator);
         }
      } else if (!rejectedPaths.contains(path) && !encounteredPaths.contains(path)) {
         project->removeGenerator(oldGenerator);
         activeGenerators.remove(path);
      }
   }

   for (auto iterator = pendingGenerators.cbegin(); iterator != pendingGenerators.cend();
        ++iterator) {
      const TextureGeneratorPtr& generator = iterator.value();
      const TextureGeneratorPtr collision = project->getGenerators().value(generator->getName());
      if (!collision.isNull()) {
         pendingDiagnostics.append(collisionDiagnostic(collision, generator));
         continue;
      }
      project->addGenerator(generator);
      activeGenerators.insert(iterator.key(), generator);
   }
}

void JsTexGenManager::clearPending() {
   pendingGenerators.clear();
   rejectedPaths.clear();
   pendingDiagnostics.clear();
}

void JsTexGenManager::removeCustomGenerators() {
   for (const TextureGeneratorPtr& generator : std::as_const(activeGenerators)) {
      project->removeGenerator(generator);
   }
   activeGenerators.clear();
}

QString JsTexGenManager::collisionDiagnostic(const TextureGeneratorPtr& oldGenerator,
                                             const TextureGeneratorPtr& newGenerator) const {
   return QStringLiteral("Duplicate generator '%1': %2 conflicts with %3")
       .arg(newGenerator->getName(), generatorSource(newGenerator), generatorSource(oldGenerator));
}

#include "jstexgenmanager.moc"
