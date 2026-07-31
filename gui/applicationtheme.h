
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef APPLICATIONTHEME_H
#define APPLICATIONTHEME_H
class QApplication;

namespace ApplicationTheme {
/// @brief Applies the shared application style and color palette.
/// @param app Application to style.
void apply(QApplication& app);
}  // namespace ApplicationTheme

#endif  // APPLICATIONTHEME_H
