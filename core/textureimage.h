/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include <QSize>
#include <QSharedPointer>
#include "global.h"

/**
 * @brief The TextureImage class
 *
 * Holds the data for an image
 */
class TextureImage
{
public:
   TextureImage(QSize size, TexturePixel* data);
   virtual ~TextureImage();
   QSize getSize() const { return size; }
   TexturePixel* getData() const { return data; }
private:
   QSize size;
   TexturePixel* data;
};

/**
 * @brief TextureImagePtr
 *
 * Thread-safe smart pointer for TextureImage
 */
typedef QSharedPointer<TextureImage> TextureImagePtr;


#endif // TEXTUREIMAGE_H
