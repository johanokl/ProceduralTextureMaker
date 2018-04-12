/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "texturegenerator.h"
#include <QString>

/**
 * @brief TextureGenerator::getSlotName
 * @param id Slot id
 * @return The name
 * Default name for slots.
 */
QString TextureGenerator::getSlotName(int id)
{
   return QString("Slot %1").arg(id + 1);
}
