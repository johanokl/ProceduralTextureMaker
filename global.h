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

   TexturePixel(const TexturePixel &rhs)
      : b(rhs.b), g(rhs.g), r(rhs.r), a(rhs.a) {}

   TexturePixel(unsigned char red = 0, unsigned char green = 0,
                unsigned char blue = 0, unsigned char alpha = 255)
      : b(blue), g(green), r(red), a(alpha) {}

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

   TexturePixel &operator+=(const TexturePixel &rhs) {
      r = qMin((int) rhs.r + r, 255);
      g = qMin((int) rhs.g + g, 255);
      b = qMin((int) rhs.b + b, 255);
      a = qMin((int) rhs.a + a, 255);
      return *this;
   }

   TexturePixel operator+(const TexturePixel &rhs) {
       TexturePixel ret(*this);
       ret += rhs;
       return ret;
   }

   /**
    * @brief TexturePixel::intensity
    * @return the intensity of pixel, from 0 - 1
    */
   double intensity() const {
      return (((double) this->r +
               (double) this->g +
               (double) this->b)
              / 3.0) / 255.0;
   }

   /**
    * @brief TexturePixel::intensity
    * @return the intensity of pixel, from 0 - 1
    */
   double intensityWithAlpha() const {
      return (((double) this->r +
               (double) this->g +
               (double) this->b)
              / 3.0) * this->a / 255.0;
   }

};


typedef struct {
   QString name;
   QString description = "";
   int order = 0;
   QVariant defaultvalue;
   int defaultindex = 0;  // For QStringList
   QVariant min;
   QVariant max;
   QString group = "";
   QString enabler = "";
}
TextureGeneratorSetting;

typedef QMap<QString, TextureGeneratorSetting> TextureGeneratorSettings;

typedef QMap<QString, QVariant> TextureNodeSettings;

#define ERROR_MSG(X) printf("ERROR: %s:%i: %s\n", __FUNCTION__, __LINE__, qPrintable(QString(X)))
#define INFO_MSG(X) printf("INFO: %s: %s\n", __FUNCTION__, qPrintable(QString(X)))

#endif // TEXGENGLOBAL_H
