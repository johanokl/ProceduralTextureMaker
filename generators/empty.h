
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef EMPTYGENERATOR_H
#define EMPTYGENERATOR_H

#include "texturegenerator.h"

/// @brief The EmptyGenerator class
class EmptyGenerator : public TextureGenerator {
public:
   EmptyGenerator() = default;
   ~EmptyGenerator() override = default;
   void generate(QSize size, TexturePixel* destimage, QMap<QString, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;
   QStringList getSourceSlots() const override {
      return {QStringLiteral("Input 1"), QStringLiteral("Input 2"), QStringLiteral("Input 3")};
   }
   QString getName() const override { return QString("Empty"); }
   const TextureGeneratorSettings& getSettings() const override { return _settings; }
   QString getDescription() const override { return QString("Empty generator."); }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }

private:
   TextureGeneratorSettings _settings;
};

#endif  // EMPTYGENERATOR_H
