/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "textureimage.h"
#include <cstdlib>
#include <exception>

/**
 * @brief TextureImage::TextureImage
 * @param size Image pixel dimensions
 * @param data TexturePixels, size must be at least width*height
 */
TextureImage::TextureImage(QSize size, TexturePixel* data)
{
   this->size = size;
   this->data = data;
}

/**
 * @brief TextureImage::~TextureImage
 */
TextureImage::~TextureImage()
{
   if (data != nullptr) {
      delete data;
   }
}
