/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPoint>
#include <math.h>
#include "lens.h"

using namespace std;

LensTextureGenerator::LensTextureGenerator()
{
   TextureGeneratorSetting offsetleft;
   offsetleft.defaultvalue = QVariant((int) 0);
   offsetleft.name = "Offset left";
   offsetleft.min = QVariant(-100);
   offsetleft.max = QVariant(100);
   offsetleft.order = 0;
   configurables.insert("offsetleft", offsetleft);

   TextureGeneratorSetting offsettop;
   offsettop.defaultvalue = QVariant((int) 0);
   offsettop.name = "Offset top";
   offsettop.min = QVariant(-100);
   offsettop.max = QVariant(100);
   offsettop.order = 1;
   configurables.insert("offsettop", offsettop);

   TextureGeneratorSetting size;
   size.defaultvalue = QVariant((double) 50);
   size.name = "Size";
   size.min = QVariant(0);
   size.max = QVariant(300);
   size.order = 2;
   configurables.insert("size", size);

   TextureGeneratorSetting strength;
   strength.defaultvalue = QVariant((double) 200);
   strength.name = "strength";
   strength.min = QVariant(0);
   strength.max = QVariant(300);
   strength.order = 3;
   configurables.insert("strength", strength);
}


void LensTextureGenerator::generate(QSize size,
                                    TexturePixel* destimage,
                                    QMap<int, TextureImagePtr> sourceimages,
                                    TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }
   int offsetleft = settings->value("offsetleft").toDouble() * size.width() / 100;
   int offsettop = settings->value("offsettop").toDouble() * size.height() / 100;
   int lenssize = settings->value("size").toDouble() * size.height() / 100;
   double strength = (300 - settings->value("strength").toDouble()) * size.width() / 100;
   if (!sourceimages.contains(0)) {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
      return;
   }
   TexturePixel* sourceimage = sourceimages.value(0).data()->getData();

   memcpy(destimage, sourceimage, size.width() * size.height() * sizeof(TexturePixel));

   if (lenssize % 2) {
      lenssize += 1;
   }
   QPoint* lens = new QPoint[lenssize * lenssize];
   int r = lenssize / 2;

   for (int y = 0; y < (lenssize >> 1); y++) {
      for (int x = 0; x < (lenssize >> 1); x++) {
         int ix = 0;
         int iy = 0;
         if ((x * x + y * y) < (r * r)) {
            double shift = (double) strength /
                  sqrt(strength * strength - (x * x + y * y - r * r));
            ix = x * shift - x;
            iy = y * shift - y;
         }
         lens[lenssize * (lenssize / 2 - y) + lenssize / 2 - x] = QPoint(-ix, -iy);
         lens[lenssize * (lenssize / 2 + y) + lenssize / 2 + x] = QPoint(ix, iy);
         lens[lenssize * (lenssize / 2 + y) + lenssize / 2 - x] = QPoint(-ix, iy);
         lens[lenssize * (lenssize / 2 - y) + lenssize / 2 + x] = QPoint(ix, -iy);
      }
   }
   for (int y = 0; y < lenssize; y++) {
      int ypos = y + size.height() / 2 + offsettop - lenssize / 2;
      for (int x = 0; x < lenssize; x++) {
         int xpos = x + size.width() / 2 + offsetleft - lenssize / 2;
         int destpos = ypos * size.width() + xpos;
         if (xpos >= 0 && xpos < size.width() && ypos >= 0 && ypos < size.height()) {
            int sourcex = xpos + lens[y * lenssize + x].x();
            if (sourcex < 0) {
               sourcex += size.width();
            } else {
               sourcex = sourcex % size.width();
            }
            int sourcey = ypos + lens[y * lenssize + x].y();
            if (sourcey < 0) {
               sourcey += size.height();
            } else {
               sourcey = sourcey % size.height();
            }
            destimage[destpos] = sourceimage[sourcey * size.width() + sourcex];
         }
      }
   }
   delete[] lens;
}
