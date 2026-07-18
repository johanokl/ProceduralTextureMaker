
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include "global.h"
#include <QSharedPointer>
#include <QSize>
#include <memory>

class TextureImage;
using TextureImagePtr = QSharedPointer<TextureImage>;

/// @brief The TextureImage class
///
/// Holds the data for an image
class TextureImage {
public:
   explicit TextureImage(QSize size);
   ~TextureImage() = default;
   static TextureImagePtr create(QSize size);
   QSize getSize() const { return size; }
   TexturePixel* getData() const { return data.get(); }

private:
   QSize size;
   std::unique_ptr<TexturePixel[]> data;
};

#endif  // TEXTUREIMAGE_H
