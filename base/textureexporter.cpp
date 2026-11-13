// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureexporter.h"
#include "textureimage.h"
#include "texturerendermanager.h"
#include "textureproject.h"
#include <QFileInfo>
#include <QImageWriter>
#include <QSaveFile>
#include <QThread>
#include <condition_variable>
#include <chrono>
#include <mutex>
#include <exception>
#include <utility>

namespace {

/// @brief Creates a failed texture-export operation result.
/// @param error Failure category.
/// @param message Human-readable failure description.
/// @return Result containing the supplied error and message.
TextureExportResult failure(const TextureExportError error, QString message) {
   return TextureExportResult{error, std::move(message)};
}

/// @brief Checks that image dimensions are positive and within the export limit.
/// @param size Image dimensions to validate.
/// @return True when the dimensions are valid for export.
bool validExportSize(const QSize size) {
   if (size.width() <= 0 || size.height() <= 0) {
      return false;
   }
   const qint64 pixelCount = static_cast<qint64>(size.width()) * size.height();
   return pixelCount > 0 && pixelCount <= TextureExporter::MaximumPixelCount;
}

}  // namespace

TextureExporter::TextureExporter(QObject* parent) : QObject(parent) {
   qRegisterMetaType<TextureExportResult>();
}

TextureExporter::~TextureExporter() {
   cancel();
   if (worker) {
      worker->wait();
   }
}

void TextureExporter::cancel() { cancelled.store(true); }

QImage TextureExporter::toQImage(const TextureImage& image, QString* error) {
   const QSize size = image.getSize();
   if (!validExportSize(size)) {
      if (error) {
         *error = QStringLiteral("The texture dimensions are invalid or exceed the export limit");
      }
      return {};
   }
   QImage result;
   try {
      result = image.toQImageCopy();
   } catch (const std::exception&) {
      if (error) {
         *error = QStringLiteral("Could not allocate a compatible Qt image buffer");
      }
      return {};
   }
   if (result.sizeInBytes() != static_cast<qsizetype>(image.byteSize())) {
      if (error) {
         *error = QStringLiteral("Could not create a compatible Qt image buffer");
      }
      return {};
   }
   return result;
}

void TextureExporter::start(TextureProject& project, const int nodeId, const QSize size,
                            const QString& path, const bool overwrite) {
   if (worker) {
      return;
   }
   TextureExportResult validation;
   if (project.getNode(nodeId).isNull()) {
      validation = failure(TextureExportError::InvalidNode,
                           QStringLiteral("The project has no node with id %1").arg(nodeId));
   } else if (!validExportSize(size)) {
      validation =
          failure(TextureExportError::InvalidSize,
                  QStringLiteral("The export size must be positive and contain at most %1 pixels")
                      .arg(MaximumPixelCount));
   } else if (QFileInfo::exists(path) && !overwrite) {
      validation = failure(TextureExportError::OutputExists,
                           QStringLiteral("The destination '%1' already exists").arg(path));
   }
   if (!validation) {
      QMetaObject::invokeMethod(
          this, [this, validation] { emit finished(validation); }, Qt::QueuedConnection);
      return;
   }

   auto snapshot = project.createTextureGraphSnapshot(size, nodeId, false);
   emit progress(0, static_cast<int>(snapshot.nodes.size()));
   worker.reset(QThread::create([this, snapshot = std::move(snapshot), nodeId, path]() mutable {
      TextureExportResult result;
      try {
         result = exportPng(std::move(snapshot), nodeId, path);
      } catch (const std::exception& error) {
         result = failure(TextureExportError::Render, QString::fromUtf8(error.what()));
      } catch (...) {
         result = failure(TextureExportError::Render, QStringLiteral("Unknown image export error"));
      }
      QMetaObject::invokeMethod(
          this, [this, result] { emit finished(result); }, Qt::QueuedConnection);
   }));
   worker->start();
}

TextureExportResult TextureExporter::exportPng(TextureGraphSnapshot snapshot, const int nodeId,
                                               const QString& path) {
   const int total = static_cast<int>(snapshot.nodes.size());
   TextureImagePtr rendered;
   TextureExportResult result;
   std::mutex mutex;
   std::condition_variable changed;
   bool done = false;
   int completed = 0;
   {
      // One render worker keeps memory use predictable for large exports.
      TextureRenderManager manager(
          [&](TextureRenderResult image) {
             std::lock_guard lock(mutex);
             if (done) {
                return;
             }
             ++completed;
             emit progress(completed, total);
             if (image.nodeId == nodeId) {
                rendered = std::move(image.image);
                done = true;
                changed.notify_one();
             }
          },
          [&](TextureRenderFailure error) {
             std::lock_guard lock(mutex);
             if (done) {
                return;
             }
             result = failure(TextureExportError::Render, error.message);
             done = true;
             changed.notify_one();
          },
          1);
      manager.render(std::move(snapshot));
      std::unique_lock lock(mutex);
      while (!done) {
         if (cancelled.load()) {
            result =
                failure(TextureExportError::Cancelled, QStringLiteral("Image export cancelled"));
            break;
         }
         changed.wait_for(lock, std::chrono::milliseconds(50));
      }
      done = true;
      lock.unlock();
      // Joining here leaves the GUI free while an active native node finishes.
      manager.cancel();
   }
   if (!result) {
      return result;
   }
   if (cancelled.load()) {
      return failure(TextureExportError::Cancelled, QStringLiteral("Image export cancelled"));
   }
   if (rendered.isNull()) {
      return failure(TextureExportError::Render,
                     QStringLiteral("The texture generator returned no image"));
   }
   QString conversionError;
   const QImage outputImage = toQImage(*rendered, &conversionError);
   if (outputImage.isNull()) {
      return failure(TextureExportError::Render, conversionError);
   }

   QSaveFile output(path);
   if (!output.open(QIODevice::WriteOnly)) {
      return failure(TextureExportError::OutputOpen,
                     QStringLiteral("Could not open '%1': %2").arg(path, output.errorString()));
   }
   QImageWriter writer(&output, "png");
   writer.setQuality(100);
   if (!writer.write(outputImage)) {
      output.cancelWriting();
      return failure(
          TextureExportError::Encode,
          QStringLiteral("Could not encode PNG '%1': %2").arg(path, writer.errorString()));
   }
   if (cancelled.load()) {
      output.cancelWriting();
      return failure(TextureExportError::Cancelled, QStringLiteral("Image export cancelled"));
   }
   if (!output.commit()) {
      return failure(TextureExportError::OutputCommit,
                     QStringLiteral("Could not commit '%1': %2").arg(path, output.errorString()));
   }
   return {};
}
