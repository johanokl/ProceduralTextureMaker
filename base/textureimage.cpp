
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "textureimage.h"
#include <cstdlib>
#include <exception>

/// @brief Constructor for the TextureImage class.
/// @details The TextureImage class is a simple wrapper for a 2D array of TexturePixel objects. It
/// stores the size of the image and a pointer to the pixel data. The pixel data is expected to be
/// allocated by the caller and will be deleted in the destructor of this class.
/// @param size Image pixel dimensions
/// @param data TexturePixels, size must be at least width*height
TextureImage::TextureImage(QSize size)
    : size(size), data(std::make_unique<TexturePixel[]>(size.width() * size.height())) {}

/// @brief Creates a shared texture image with an owned pixel buffer.
TextureImagePtr TextureImage::create(QSize size) { return TextureImagePtr::create(size); }
