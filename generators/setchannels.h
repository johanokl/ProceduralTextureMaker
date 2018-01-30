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
   virtual ~SetChannelsTextureGenerator() {}
   virtual void generate(QSize size,
                         TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;
   virtual int getNumSourceSlots() const { return 2; }
   virtual QString getName() const { return QString("Set channels"); }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return QString(""); }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Combiner; }

private:
   TextureGeneratorSettings configurables;
   Channels getChannelFromName(QString name) const;
   unsigned char getColorFromChannel(TexturePixel &firstColor, TexturePixel &secondColor,
                                     Channels channel) const;
};

#endif // SETCHANNELSTEXTUREGENERATOR_H


