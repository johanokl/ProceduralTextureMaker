/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SETCHANNELSTEXTUREGENERATOR_H
#define SETCHANNELSTEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The SetChannelsTextureGenerator class
 */
class SetChannelsTextureGenerator : public TextureGenerator
{
public:
   enum Channels {
      none = 1,
      fill = 2,
      node1red = 3,
      node1green = 4,
      node1blue = 5,
      node1alpha = 6,
      node2red = 7,
      node2green = 8,
      node2blue = 9,
      node2alpha = 10
   };

   SetChannelsTextureGenerator();
   ~SetChannelsTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   int getNumSourceSlots() const override { return 2; }
   QString getName() const override { return QString("Set channels"); }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return QString(""); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
   Channels getChannelFromName(const QString& name) const;
   quint8 getColorFromChannel(const TexturePixel& firstColor, const TexturePixel& secondColor,
                                     Channels channel) const;
};

#endif // SETCHANNELSTEXTUREGENERATOR_H


