
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureimage.h"
#include "global.h"
#include <QSize>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace {

/// @brief Validates image dimensions and computes their pixel count without overflowing.
/// @param size The image width and height in pixels.
/// @return The number of pixels required by the image.
/// @throws std::invalid_argument if either dimension is not positive.
/// @throws std::length_error if the required pixel storage cannot be represented.
std::size_t checkedPixelCount(const QSize size) {
   if (size.width() <= 0 || size.height() <= 0) {
      throw std::invalid_argument("Texture image dimensions must be positive");
   }
   const auto width = static_cast<std::size_t>(size.width());
   const auto height = static_cast<std::size_t>(size.height());
   constexpr auto maxPixelCount = std::numeric_limits<std::size_t>::max() / sizeof(TexturePixel);
   if (width > maxPixelCount / height) {
      throw std::length_error("Texture image dimensions exceed addressable memory");
   }
   return width * height;
}

/// @brief Computes the tightly packed RGBA row stride without overflowing Qt's size type.
/// @param size Validated image dimensions.
/// @return Bytes in one image row.
/// @throws std::length_error if the stride cannot be represented.
qsizetype checkedBytesPerLine(const QSize size) {
   constexpr qsizetype bytesPerPixel = sizeof(TexturePixel);
   if constexpr (sizeof(qsizetype) <= sizeof(int)) {
      if (size.width() > std::numeric_limits<qsizetype>::max() / bytesPerPixel) {
         throw std::length_error("Texture image row stride exceeds Qt image capacity");
      }
   }
   return static_cast<qsizetype>(size.width()) * bytesPerPixel;
}

}  // namespace

QImage makeTextureImageView(const QSize size, TexturePixel* pixels) {
   static_cast<void>(checkedPixelCount(size));
   if (pixels == nullptr) {
      throw std::invalid_argument("Texture image pixels must not be null");
   }
   return QImage(reinterpret_cast<uchar*>(pixels), size.width(), size.height(),
                 checkedBytesPerLine(size), QImage::Format_RGBA8888);
}

QImage makeTextureImageView(const QSize size, const TexturePixel* pixels) {
   static_cast<void>(checkedPixelCount(size));
   if (pixels == nullptr) {
      throw std::invalid_argument("Texture image pixels must not be null");
   }
   return QImage(reinterpret_cast<const uchar*>(pixels), size.width(), size.height(),
                 checkedBytesPerLine(size), QImage::Format_RGBA8888);
}

QImage copyTextureImage(const QSize size, const TexturePixel* pixels) {
   const QImage view = makeTextureImageView(size, pixels);
   QImage copy(size, QImage::Format_RGBA8888);
   if (copy.isNull()) {
      throw std::bad_alloc();
   }
   std::memcpy(copy.bits(), view.constBits(), static_cast<std::size_t>(copy.sizeInBytes()));
   return copy;
}

TextureImage::TextureImage(QSize size) : size(size), pixels(checkedPixelCount(size)) {}

TextureImagePtr TextureImage::create(QSize size) { return TextureImagePtr::create(size); }
