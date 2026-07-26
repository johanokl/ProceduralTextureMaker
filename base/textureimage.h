
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include "global.h"
#include <QSharedPointer>
#include <QSize>
#include <cstddef>
#include <vector>

class TextureImage;
using TextureImagePtr = QSharedPointer<TextureImage>;

/// @brief Owns a contiguous buffer of texture pixels.
class TextureImage {
public:
   /// @brief Constructs an image with an owned, contiguous pixel buffer.
   /// @param size The image width and height in pixels; both dimensions must be positive.
   /// @throws std::invalid_argument if either dimension is not positive.
   /// @throws std::length_error if the required pixel storage cannot be represented.
   explicit TextureImage(QSize size);

   /// @brief Destroys the image and releases its pixel buffer.
   ~TextureImage() = default;

   /// @brief Disables copy construction to prevent accidental duplication of the pixel buffer.
   TextureImage(const TextureImage&) = delete;

   /// @brief Disables copy assignment to prevent accidental duplication of the pixel buffer.
   TextureImage& operator=(const TextureImage&) = delete;

   /// @brief Moves an image without copying its pixel buffer.
   TextureImage(TextureImage&&) noexcept = default;

   /// @brief Move-assigns an image without copying its pixel buffer.
   TextureImage& operator=(TextureImage&&) noexcept = default;

   /// @brief Creates a shared texture image with an owned pixel buffer.
   /// @param size The image width and height in pixels; both dimensions must be positive.
   /// @return A shared pointer to the newly allocated image.
   /// @throws std::invalid_argument if either dimension is not positive.
   /// @throws std::length_error if the required pixel storage cannot be represented.
   static TextureImagePtr create(QSize size);

   /// @brief Returns the image dimensions in pixels.
   QSize getSize() const noexcept { return size; }

   /// @brief Returns the number of pixels in the image buffer.
   std::size_t pixelCount() const noexcept { return pixels.size(); }

   /// @brief Returns the size of the image buffer in bytes.
   std::size_t byteSize() const noexcept { return pixels.size() * sizeof(TexturePixel); }

   /// @brief Returns a mutable pointer to the contiguous pixel buffer.
   TexturePixel* data() noexcept { return pixels.data(); }

   /// @brief Returns a read-only pointer to the contiguous pixel buffer.
   const TexturePixel* data() const noexcept { return pixels.data(); }

   /// @brief Returns a mutable pointer through the legacy compatibility API.
   TexturePixel* getData() noexcept { return data(); }

   /// @brief Returns a read-only pointer through the legacy compatibility API.
   const TexturePixel* getData() const noexcept { return data(); }

private:
   /// @brief Image width and height in pixels.
   QSize size;
   /// @brief Contiguous storage for the image pixels.
   std::vector<TexturePixel> pixels;
};

#endif  // TEXTUREIMAGE_H
