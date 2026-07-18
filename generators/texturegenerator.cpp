
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "texturegenerator.h"
#include <QString>

/// @brief Get the slot name for a given slot id.
/// @details The default implementation returns "Slot X" where X is the slot id + 1. This can be
/// overridden in derived classes to provide more descriptive names.
/// @param id Slot id
/// @return The name
QString TextureGenerator::getSlotName(int id) { return QString("Slot %1").arg(id + 1); }
