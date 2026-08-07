// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef BACKGROUNDBRUSHSTYLES_H
#define BACKGROUNDBRUSHSTYLES_H

#include <QtCore/qnamespace.h>
#include <array>

namespace BackgroundBrushStyles {

/// @brief User-selectable background brush style and its display label.
struct Option {
   const char* label;
   Qt::BrushStyle style;
};

/// @brief Brush styles supported by settings storage and the settings panel.
inline constexpr std::array<Option, 15> options{{
    {"None", Qt::NoBrush},
    {"Solid", Qt::SolidPattern},
    {"Horizontal", Qt::HorPattern},
    {"Vertical", Qt::VerPattern},
    {"Cross", Qt::CrossPattern},
    {"Backward Diagonal", Qt::BDiagPattern},
    {"Forward Diagonal", Qt::FDiagPattern},
    {"Diagonal Cross", Qt::DiagCrossPattern},
    {"Dense 1", Qt::Dense1Pattern},
    {"Dense 2", Qt::Dense2Pattern},
    {"Dense 3", Qt::Dense3Pattern},
    {"Dense 4", Qt::Dense4Pattern},
    {"Dense 5", Qt::Dense5Pattern},
    {"Dense 6", Qt::Dense6Pattern},
    {"Dense 7", Qt::Dense7Pattern},
}};

/// @brief Checks whether an integer represents a user-selectable brush style.
constexpr bool isSupported(const int value) {
   for (const Option& option : options) {
      if (value == static_cast<int>(option.style)) {
         return true;
      }
   }
   return false;
}

}  // namespace BackgroundBrushStyles

#endif  // BACKGROUNDBRUSHSTYLES_H
