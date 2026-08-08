// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXGENGLOBAL_H
#define TEXGENGLOBAL_H

#include <QList>
#include <QMap>
#include <QVariant>
#include <QtGlobal>
#include <climits>
#include <cstddef>
#include <type_traits>

/// @brief Stores one straight-alpha pixel as four byte-ordered RGBA channels.
class TexturePixel {
public:
   /// @brief Red channel from 0 to 255.
   quint8 r{0};
   /// @brief Green channel from 0 to 255.
   quint8 g{0};
   /// @brief Blue channel from 0 to 255.
   quint8 b{0};
   /// @brief Straight alpha channel from transparent 0 to opaque 255.
   quint8 a{0};

   /// @brief Constructs a transparent black pixel.
   constexpr TexturePixel() noexcept = default;

   /// @brief Constructs a pixel from straight RGBA channels.
   /// @param red Red channel from 0 to 255.
   /// @param green Green channel from 0 to 255.
   /// @param blue Blue channel from 0 to 255.
   /// @param alpha Straight alpha channel from 0 to 255.
   constexpr explicit TexturePixel(const quint8 red, const quint8 green, const quint8 blue,
                                   const quint8 alpha) noexcept
       : r(red), g(green), b(blue), a(alpha) {}

   /// @brief Copies another pixel without changing its channel layout.
   /// @param rhs Pixel to copy.
   TexturePixel(const TexturePixel& rhs) = default;

   /// @brief Packs the channels into the numeric value `0xRRGGBBAA`.
   /// @return A numeric RGBA value independent of native byte order.
   [[nodiscard]] constexpr quint32 toRGBA() const noexcept {
      return (static_cast<quint32>(r) << 24U) | (static_cast<quint32>(g) << 16U) |
             (static_cast<quint32>(b) << 8U) | static_cast<quint32>(a);
   }

   /// @brief Replaces this pixel with another pixel.
   /// @param rhs Pixel to copy.
   /// @return This pixel after assignment.
   TexturePixel& operator=(const TexturePixel& rhs) = default;

   /// @brief Adds another pixel channel by channel with saturation at 255.
   /// @param rhs Pixel whose channels are added.
   /// @return This pixel after addition.
   TexturePixel& operator+=(const TexturePixel& rhs) noexcept {
      r = static_cast<quint8>(qMin(static_cast<int>(rhs.r) + static_cast<int>(r), 255));
      g = static_cast<quint8>(qMin(static_cast<int>(rhs.g) + static_cast<int>(g), 255));
      b = static_cast<quint8>(qMin(static_cast<int>(rhs.b) + static_cast<int>(b), 255));
      a = static_cast<quint8>(qMin(static_cast<int>(rhs.a) + static_cast<int>(a), 255));
      return *this;
   }

   /// @brief Adds two pixels channel by channel with saturation at 255.
   /// @param rhs Pixel whose channels are added.
   /// @return The saturated channel sum.
   [[nodiscard]] TexturePixel operator+(const TexturePixel& rhs) const noexcept {
      TexturePixel ret(*this);
      ret += rhs;
      return ret;
   }

   /// @brief Calculates the arithmetic mean of the RGB channels.
   /// @return Color intensity from 0.0 to 1.0 without applying alpha.
   [[nodiscard]] constexpr double intensity() const noexcept {
      return (static_cast<double>(r) + static_cast<double>(g) + static_cast<double>(b)) /
             (3.0 * 255.0);
   }

   /// @brief Calculates RGB intensity weighted by straight alpha.
   /// @return Alpha-weighted intensity from 0.0 to 1.0.
   [[nodiscard]] constexpr double intensityWithAlpha() const noexcept {
      return intensity() * (static_cast<double>(a) / 255.0);
   }
};

static_assert(CHAR_BIT == 8, "Texture pixels require eight-bit bytes");
static_assert(sizeof(TexturePixel) == 4, "Texture pixels must contain exactly four bytes");
static_assert(std::is_standard_layout_v<TexturePixel>, "Texture pixels must use standard layout");
static_assert(std::is_trivially_copyable_v<TexturePixel>,
              "Texture pixels must support contiguous byte copies");
static_assert(offsetof(TexturePixel, r) == 0, "Red must be the first texture byte");
static_assert(offsetof(TexturePixel, g) == 1, "Green must be the second texture byte");
static_assert(offsetof(TexturePixel, b) == 2, "Blue must be the third texture byte");
static_assert(offsetof(TexturePixel, a) == 3, "Alpha must be the fourth texture byte");

/// @brief Describes one user-configurable texture-generator setting.
struct TextureGeneratorSetting {
   /// @brief Stable, case-sensitive ID used by persistence and runtime setting values.
   QString id;
   /// @brief Default value and setting type used when creating a node.
   QVariant defaultvalue;
   /// @brief User-facing setting name.
   QString name;
   /// @brief Optional user-facing setting description.
   QString description;
   /// @brief Default selected index when defaultvalue contains a QStringList.
   int defaultindex = 0;
   /// @brief Optional inclusive minimum for numeric settings.
   QVariant min;
   /// @brief Optional inclusive maximum for numeric settings.
   QVariant max;
   /// @brief Optional presentation group name.
   QString group;
   /// @brief Optional boolean setting ID controlling whether this setting is enabled.
   QString enabler;
   /// @brief Whether a string setting should use a multiline editor.
   bool multiline = false;
};

/// @brief Generator setting definitions stored in presentation order.
using TextureGeneratorSettings = QList<TextureGeneratorSetting>;

/// @brief Current node setting values keyed by stable setting ID.
using TextureNodeSettings = QMap<QString, QVariant>;

#endif  // TEXGENGLOBAL_H
