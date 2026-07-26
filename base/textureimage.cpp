
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureimage.h"
#include "global.h"
#include <QSize>
#include <cstddef>
#include <limits>
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

}  // namespace

TextureImage::TextureImage(QSize size) : size(size), pixels(checkedPixelCount(size)) {}

TextureImagePtr TextureImage::create(QSize size) { return TextureImagePtr::create(size); }
