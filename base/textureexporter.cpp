// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureexporter.h"
#include "textureimage.h"
#include "texturenode.h"
#include "textureproject.h"
#include <QFileInfo>
#include <QImageWriter>
#include <QSaveFile>
#include <cstring>
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

/// @brief Copies a texture image into a compatible Qt image.
/// @param image Texture image whose pixel data is copied.
/// @param error Optional destination for a failure description.
/// @return Converted image, or a null image when conversion fails.
QImage TextureExporter::toQImage(const TextureImage& image, QString* error) {
   const QSize size = image.getSize();
   if (!validExportSize(size)) {
      if (error) {
         *error = QStringLiteral("The texture dimensions are invalid or exceed the export limit");
      }
      return {};
   }
   QImage result(size, QImage::Format_ARGB32);
   if (result.isNull() || result.sizeInBytes() != static_cast<qsizetype>(image.byteSize())) {
      if (error) {
         *error = QStringLiteral("Could not allocate a compatible Qt image buffer");
      }
      return {};
   }
   std::memcpy(result.bits(), image.data(), image.byteSize());
   return result;
}

/// @brief Renders a project node and writes it atomically as a PNG file.
/// @param project Project containing the node to render.
/// @param nodeId Identifier of the node to render.
/// @param size Output image dimensions in pixels.
/// @param path Destination path for the PNG file.
/// @param overwrite Whether an existing destination may be replaced.
/// @return The operation result, including an error message on failure.
TextureExportResult TextureExporter::exportPng(TextureProject& project, const int nodeId,
                                               const QSize size, const QString& path,
                                               const bool overwrite) {
   const TextureNodePtr node = project.getNode(nodeId);
   if (node.isNull()) {
      return failure(TextureExportError::InvalidNode,
                     QStringLiteral("The project has no node with id %1").arg(nodeId));
   }
   if (!validExportSize(size)) {
      return failure(
          TextureExportError::InvalidSize,
          QStringLiteral("The export size must be positive and contain at most %1 pixels")
              .arg(MaximumPixelCount));
   }
   if (QFileInfo::exists(path) && !overwrite) {
      return failure(TextureExportError::OutputExists,
                     QStringLiteral("The destination '%1' already exists").arg(path));
   }

   QImage outputImage;
   try {
      const TextureImagePtr rendered = node->renderImage(size);
      if (rendered.isNull()) {
         return failure(TextureExportError::Render,
                        QStringLiteral("The texture generator returned no image"));
      }
      QString conversionError;
      outputImage = toQImage(*rendered, &conversionError);
      if (outputImage.isNull()) {
         return failure(TextureExportError::Render, conversionError);
      }
   } catch (const std::exception& error) {
      return failure(TextureExportError::Render, QString::fromUtf8(error.what()));
   } catch (...) {
      return failure(TextureExportError::Render, QStringLiteral("Unknown texture rendering error"));
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
   if (!output.commit()) {
      return failure(TextureExportError::OutputCommit,
                     QStringLiteral("Could not commit '%1': %2").arg(path, output.errorString()));
   }
   return {};
}
