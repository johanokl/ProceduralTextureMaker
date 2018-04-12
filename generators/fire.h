/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef FIRETEXTUREGENERATOR_H
#define FIRETEXTUREGENERATOR_H

#include "texturegenerator.h"

/**
 * @brief The FireTextureGenerator class
 *
 * The algorithm is taken from Lode's Computer Graphics Tutorial
 * http://lodev.org/cgtutor/fire.html
 */
class FireTextureGenerator : public TextureGenerator
{
public:
   FireTextureGenerator();
   ~FireTextureGenerator() override = default;
   void generate(QSize size,
                 TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   int getNumSourceSlots() const override { return 1; }
   QString getName() const override { return "Fire"; }
   QString getDescription() const override { return QString("Early 90's fire effect"); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   double noise(double x, double y) const;
   double findnoise2(double x,double y) const;
   double interpolate(double a, double b, double x) const;

   TextureGeneratorSettings configurables;
};

#endif // FIRETEXTUREGENERATOR_H
