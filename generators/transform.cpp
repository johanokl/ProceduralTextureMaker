
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "transform.h"
#include <QPainter>

TransformTextureGenerator::TransformTextureGenerator() {
   TextureGeneratorSetting scaleX;
   scaleX.name = "Horizontal scale (%)";
   scaleX.description = "Scales the input image horizontally around the texture centre.";
   scaleX.defaultvalue = QVariant((double)100);
   scaleX.min = QVariant(0);
   scaleX.max = QVariant(500);
   scaleX.group = "scale";
   scaleX.id = "xscale";
   configurables.append(scaleX);

   TextureGeneratorSetting scaleY;
   scaleY.name = "Vertical scale (%)";
   scaleY.description = "Scales the input image vertically around the texture centre.";
   scaleY.defaultvalue = QVariant((double)100);
   scaleY.min = QVariant(0);
   scaleY.max = QVariant(500);
   scaleY.group = "scale";
   scaleY.id = "yscale";
   configurables.append(scaleY);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation (°)";
   rotation.description = "Rotates the input image around the texture centre.";
   rotation.defaultvalue = QVariant((double)0);
   rotation.min = QVariant(-360);
   rotation.max = QVariant(360);
   rotation.id = "rotation";
   configurables.append(rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the input image horizontally across the texture.";
   offsetLeft.defaultvalue = QVariant((double)0);
   offsetLeft.min = QVariant(-200);
   offsetLeft.max = QVariant(200);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the input image vertically across the texture.";
   offsetTop.defaultvalue = QVariant((double)0);
   offsetTop.min = QVariant(-200);
   offsetTop.max = QVariant(200);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);

   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Background colour";
   colorsetting.description = "Colour used where the transformed image leaves empty space.";
   colorsetting.defaultvalue = QVariant(QColor(0, 0, 0, 0));
   colorsetting.id = "backgroundcolor";
   configurables.append(colorsetting);

   TextureGeneratorSetting firstXtiles;
   firstXtiles.name = "First-pass horizontal tiles";
   firstXtiles.description = "Number of horizontal copies created before transformation.";
   firstXtiles.defaultvalue = QVariant((int)1);
   firstXtiles.min = QVariant(1);
   firstXtiles.max = QVariant(5);
   firstXtiles.group = "first tiles";
   firstXtiles.id = "firstXtiles";
   configurables.append(firstXtiles);

   TextureGeneratorSetting firstYtiles;
   firstYtiles.name = "First-pass vertical tiles";
   firstYtiles.description = "Number of vertical copies created before transformation.";
   firstYtiles.defaultvalue = QVariant((int)1);
   firstYtiles.min = QVariant(1);
   firstYtiles.max = QVariant(5);
   firstYtiles.group = "first tiles";
   firstYtiles.id = "firstYtiles";
   configurables.append(firstYtiles);

   TextureGeneratorSetting secondXtiles;
   secondXtiles.name = "Second-pass horizontal tiles";
   secondXtiles.description = "Number of horizontal copies created after transformation.";
   secondXtiles.defaultvalue = QVariant((int)1);
   secondXtiles.min = QVariant(1);
   secondXtiles.max = QVariant(50);
   secondXtiles.group = "second tiles";
   secondXtiles.id = "secondXtiles";
   configurables.append(secondXtiles);

   TextureGeneratorSetting secondYtiles;
   secondYtiles.name = "Second-pass vertical tiles";
   secondYtiles.description = "Number of vertical copies created after transformation.";
   secondYtiles.defaultvalue = QVariant((int)1);
   secondYtiles.min = QVariant(1);
   secondYtiles.max = QVariant(50);
   secondYtiles.group = "second tiles";
   secondYtiles.id = "secondYtiles";
   configurables.append(secondYtiles);
}
void TransformTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                         const QMap<QString, TextureImagePtr>& sourceimages,
                                         const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }

   double scaleX = settings.value("xscale").toDouble() / 100;
   double scaleY = settings.value("yscale").toDouble() / 100;
   double rotation = settings.value("rotation").toDouble();
   int offsetLeft = settings.value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings.value("offsettop").toDouble() * size.height() / 100;
   int firstXtiles = settings.value("firstXtiles").toInt();
   int firstYtiles = settings.value("firstYtiles").toInt();
   int secondXtiles = settings.value("secondXtiles").toInt();
   int secondYtiles = settings.value("secondYtiles").toInt();
   QColor backgroundcolor = settings.value("backgroundcolor").value<QColor>();

   QImage tempimage = makeTextureImageView(size, destimage);

   if (sourceimages.contains(QStringLiteral("Image"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Image"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   TextureImage tiledStorage(size);
   QImage tiledimage = tiledStorage.toQImageView();
   {
      QPainter firstpainter(&tiledimage);
      firstpainter.scale(static_cast<double>(1) / secondXtiles,
                         static_cast<double>(1) / secondYtiles);
      firstpainter.fillRect(0, 0, secondXtiles * tempimage.width(),
                            secondYtiles * tempimage.height(), QBrush(tempimage));
   }

   TextureImage destinationStorage(QSize(firstXtiles * size.width(), firstYtiles * size.height()));
   QImage destBuffer = destinationStorage.toQImageView();
   TexturePixel col(backgroundcolor.red(), backgroundcolor.green(), backgroundcolor.blue(),
                    backgroundcolor.alpha());
   for (std::size_t index = 0; index < destinationStorage.pixelCount(); ++index) {
      destinationStorage.data()[index] = col;
   }

   {
      QPainter painter(&destBuffer);
      painter.translate(offsetLeft, offsetTop);
      painter.translate(size.width() / 2, size.height() / 2);
      painter.rotate(rotation);
      painter.scale(scaleX, scaleY);
      painter.translate(-firstXtiles * size.width() / 2, -firstYtiles * size.height() / 2);
      painter.fillRect(0, 0, destBuffer.width(), destBuffer.height(), QBrush(tiledimage));
   }

   memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   QPainter finalpainter(&tempimage);
   finalpainter.drawImage(QRectF(0, 0, size.width(), size.height()), destBuffer,
                          QRectF(0, 0, size.width(), size.height()));
}
