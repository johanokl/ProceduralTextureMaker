
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "cutout.h"

CutoutTextureGenerator::CutoutTextureGenerator() {
   QStringList ordering;
   ordering.append("Mask cut out from Image");
   ordering.append("Image cut out from Mask");
   TextureGeneratorSetting order;
   order.name = "Order";
   order.defaultvalue = QVariant(ordering);
   order.defaultindex = 0;
   order.order = 1;
   configurables.insert("order", order);

   TextureGeneratorSetting factor;
   factor.name = "Factor";
   factor.defaultvalue = QVariant((int)1);
   factor.min = 1;
   factor.max = 255;
   factor.defaultindex = 0;
   factor.order = 2;
   configurables.insert("factor", factor);
}
void CutoutTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                      const QMap<QString, TextureImagePtr>& sourceimages,
                                      const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   QString order = settings.value("order").toString();
   int factor = settings.value("factor").toInt();

   QString first = QStringLiteral("Image");
   QString second = QStringLiteral("Mask");
   if (order == QStringLiteral("Image cut out from Mask") ||
       order == QStringLiteral("Slot 1 cut out from Slot 2")) {
      first = QStringLiteral("Mask");
      second = QStringLiteral("Image");
   }
   TexturePixel* originSource = nullptr;
   TexturePixel* subtractSource = nullptr;
   if (sourceimages.contains(first)) {
      originSource = sourceimages.value(first).data()->getData();
   }
   if (sourceimages.contains(second)) {
      subtractSource = sourceimages.value(second).data()->getData();
   }
   int numPixels = size.width() * size.height();
   if (originSource && subtractSource) {
      memcpy(destimage, originSource, numPixels * sizeof(TexturePixel));
      for (int i = 0; i < numPixels; i++) {
         if (destimage[i].a > (factor * subtractSource[i].a)) {
            destimage[i].a -= subtractSource[i].a;
         } else {
            destimage[i].a = 0;
         }
      }
   } else if (originSource) {
      memcpy(destimage, originSource, numPixels * sizeof(TexturePixel));
   } else if (subtractSource) {
      memcpy(destimage, subtractSource, numPixels * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, numPixels * sizeof(TexturePixel));
   }
}
