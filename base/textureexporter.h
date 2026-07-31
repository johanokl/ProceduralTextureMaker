// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREEXPORTER_H
#define TEXTUREEXPORTER_H

#include <QImage>
#include <QSize>
#include <QString>

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
   OutputCommit
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

/// @brief Provides shared texture conversion and headless image-export operations.
class TextureExporter final {
public:
   /// @brief Maximum number of pixels accepted by an export operation.
   static constexpr qsizetype MaximumPixelCount = 64 * 1024 * 1024;

   /// @brief Copies a texture image into a compatible Qt image.
   /// @param image Texture image whose pixel data is copied.
   /// @param error Optional destination for a failure description.
   /// @return Converted image, or a null image when conversion fails.
   [[nodiscard]] static QImage toQImage(const TextureImage& image, QString* error = nullptr);

   /// @brief Renders a project node and writes it atomically as a PNG file.
   /// @param project Project containing the node to render.
   /// @param nodeId Identifier of the node to render.
   /// @param size Output image dimensions in pixels.
   /// @param path Destination path for the PNG file.
   /// @param overwrite Whether an existing destination may be replaced.
   /// @return The operation result, including an error message on failure.
   [[nodiscard]] static TextureExportResult exportPng(TextureProject& project, int nodeId,
                                                      QSize size, const QString& path,
                                                      bool overwrite);
};

#endif  // TEXTUREEXPORTER_H
