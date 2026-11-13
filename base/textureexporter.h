// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREEXPORTER_H
#define TEXTUREEXPORTER_H

#include <QImage>
#include <QObject>
#include <atomic>
#include <memory>
#include <QSize>
#include <QString>

class QThread;
struct TextureGraphSnapshot;
class TextureImage;
class TextureProject;

/// @brief Identifies the stage at which an image export operation failed.
enum class TextureExportError {
   /// @brief The operation completed successfully.
   None,
   /// @brief The requested node does not exist.
   InvalidNode,
   /// @brief The requested dimensions are invalid or exceed the export limit.
   InvalidSize,
   /// @brief The output path exists and overwriting was disabled.
   OutputExists,
   /// @brief The output file could not be opened.
   OutputOpen,
   /// @brief Rendering or conversion to a Qt image failed.
   Render,
   /// @brief The rendered image could not be encoded as PNG.
   Encode,
   /// @brief The temporary output could not be committed atomically.
   OutputCommit,
   /// @brief The user cancelled the export.
   Cancelled
};

/// @brief Reports the outcome of an image export operation.
struct TextureExportResult {
   /// @brief Operation status code.
   TextureExportError error = TextureExportError::None;
   /// @brief Human-readable failure description, or an empty string on success.
   QString message;

   /// @brief Returns whether the operation completed successfully.
   [[nodiscard]] bool succeeded() const noexcept { return error == TextureExportError::None; }

   /// @brief Converts the result to true when the operation succeeded.
   explicit operator bool() const noexcept { return succeeded(); }
};

Q_DECLARE_METATYPE(TextureExportResult)

/// @brief Renders a graph snapshot and writes a PNG in the background.
/// @details Each instance runs one export. Cancellation interrupts JavaScript;
/// an active native generator finishes before the worker exits.
class TextureExporter final : public QObject {
   Q_OBJECT

public:
   /// @brief Creates an idle export job.
   explicit TextureExporter(QObject* parent = nullptr);

   /// @brief Cancels the export and waits for its worker to finish.
   ~TextureExporter() override;

   /// @brief Maximum number of pixels accepted by an export operation.
   static constexpr qsizetype MaximumPixelCount = 64 * 1024 * 1024;

   /// @brief Copies a texture image into a compatible Qt image.
   /// @param image Texture image whose pixel data is copied.
   /// @param error Optional destination for a failure description.
   /// @return Converted image, or a null image when conversion fails.
   [[nodiscard]] static QImage toQImage(const TextureImage& image, QString* error = nullptr);

   /// @brief Copies the selected graph and starts its export. Call once on the project thread.
   /// @param project Project containing the selected node.
   /// @param nodeId Node to export together with its inputs.
   /// @param size Output image dimensions.
   /// @param path Destination PNG path.
   /// @param overwrite Whether an existing file may be replaced.
   void start(TextureProject& project, int nodeId, QSize size, const QString& path, bool overwrite);

   /// @brief Requests cancellation without waiting on the caller's thread.
   void cancel();

signals:
   /// @brief Reports the number of rendered nodes and the total needed for export.
   void progress(int completed, int total);

   /// @brief Reports success, cancellation, or failure after the worker has finished.
   void finished(TextureExportResult result);

private:
   /// @brief Renders the snapshot and writes the result on the export worker.
   TextureExportResult exportPng(TextureGraphSnapshot snapshot, int nodeId, const QString& path);

   /// @brief Worker that owns rendering and file writing.
   std::unique_ptr<QThread> worker;
   /// @brief Cancellation request shared with the worker.
   std::atomic<bool> cancelled{false};
};

#endif  // TEXTUREEXPORTER_H
