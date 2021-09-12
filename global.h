/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXGENGLOBAL_H
#define TEXGENGLOBAL_H

#include <QMap>
#include <QVariant>
#include <QtGlobal>

/**
 * @brief The TexturePixel class
 *
 * 32 bit RGBA pixel, stored as BGRA.
 */
class TexturePixel {
public:
   unsigned char b;
   unsigned char g;
   unsigned char r;
   unsigned char a;

   TexturePixel() = default;

   explicit
   TexturePixel(unsigned char red, unsigned char green,
                unsigned char blue, unsigned char alpha)
      : b(blue), g(green), r(red), a(alpha) {}

   TexturePixel(const TexturePixel& rhs) = default;

   /**
    * @brief TexturePixel::toRGBA
    * @return the pixel as RGBA
    */
   quint32 toRGBA() const {
      quint32 ret = r;
      ret = ret << 8;
      ret += g;
      ret = ret << 8;
      ret += b;
      ret = ret << 8;
      ret += a;
      return ret;
   }

   TexturePixel& operator=(const TexturePixel& rhs) = default;

   TexturePixel& operator+=(const TexturePixel& rhs) {
      r = static_cast<unsigned char>(qMin(static_cast<int>(rhs.r) + static_cast<int>(r), 255));
      g = static_cast<unsigned char>(qMin(static_cast<int>(rhs.g) + static_cast<int>(g), 255));
      b = static_cast<unsigned char>(qMin(static_cast<int>(rhs.b) + static_cast<int>(b), 255));
      a = static_cast<unsigned char>(qMin(static_cast<int>(rhs.a) + static_cast<int>(a), 255));
      return *this;
   }

   TexturePixel operator+(const TexturePixel& rhs) const {
       TexturePixel ret(*this);
       ret += rhs;
       return ret;
   }

   /**
    * @brief TexturePixel::intensity
    * @return the intensity of pixel, from 0 - 1
    */
   double intensity() const {
      return ((static_cast<double>(this->r) +
               static_cast<double>(this->g) +
               static_cast<double>(this->b))
              / 3.0) / 255.0;
   }

   /**
    * @brief TexturePixel::intensity
    * @return the intensity of pixel, from 0 - 1
    */
   double intensityWithAlpha() const {
      return ((static_cast<double>(this->r) +
               static_cast<double>(this->g) +
               static_cast<double>(this->b))
              / 3.0) * this->a / 255.0;
   }

};


struct TextureGeneratorSetting {
   QVariant defaultvalue;
   QString name;
   QString description = "";
   int order = 0;
   int defaultindex = 0;  // For QStringList
   QVariant min;
   QVariant max;
   QString group = "";
   QString enabler = "";
};

typedef QMap<QString, TextureGeneratorSetting> TextureGeneratorSettings;

typedef QMap<QString, QVariant> TextureNodeSettings;

#define ERROR_MSG(X) printf("ERROR: %s:%i: %s\n", __FUNCTION__, __LINE__, qPrintable(QString(X)))
#define INFO_MSG(X) printf("INFO: %s: %s\n", __FUNCTION__, qPrintable(QString(X)))

#endif // TEXGENGLOBAL_H
