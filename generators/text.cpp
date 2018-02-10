/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */


#include <math.h>
#include <QPainter>
#include "text.h"

using namespace std;

TextTextureGenerator::TextTextureGenerator()
{
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Color";
   colorsetting.defaultvalue = QVariant(QColor(255, 0, 0));
   colorsetting.order = 1;
   configurables.insert("color", colorsetting);

   TextureGeneratorSetting text;
   text.name = "Text";
   text.description = "";
   text.defaultvalue = QVariant(QString("Text"));
   text.order = 2;
   configurables.insert("text", text);

   TextureGeneratorSetting font;
   QStringList fonts;
   fonts.append("AnyStyle");
   fonts.append("Helvetica");
   fonts.append("Times");
   fonts.append("Courier");
   fonts.append("OldEnglish");
   fonts.append("System");
   fonts.append("Cursive");
   fonts.append("Monospace");
   fonts.append("Fantasy");
   font.name = "Font";
   font.defaultvalue = QVariant(fonts);
   font.order = 3;
   configurables.insert("fontname", font);

   TextureGeneratorSetting fontsize;
   fontsize.name = "Size";
   fontsize.description = "";
   fontsize.defaultvalue = QVariant((double) 20);
   fontsize.min = QVariant((double) 0);
   fontsize.max = QVariant((double) 200);
   fontsize.order = 4;
   configurables.insert("fontsize", fontsize);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation";
   rotation.description = "";
   rotation.defaultvalue = QVariant((double) 0);
   rotation.min = QVariant((double) -360);
   rotation.max = QVariant((double) 360);
   rotation.order = 5;
   configurables.insert("rotation", rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Offset left";
   offsetLeft.description = "";
   offsetLeft.defaultvalue = QVariant((double) 0);
   offsetLeft.min = QVariant((double) -200);
   offsetLeft.max = QVariant((double) 200);
   offsetLeft.order = 6;
   configurables.insert("offsetleft", offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Offset top";
   offsetTop.description = "";
   offsetTop.defaultvalue = QVariant((double) 0);
   offsetTop.min = QVariant((double) -200);
   offsetTop.max = QVariant((double) 200);
   offsetTop.order = 7;
   configurables.insert("offsettop", offsetTop);
}


void TextTextureGenerator::generate(QSize size,
                                    TexturePixel* destimage,
                                    QMap<int, TextureImagePtr> sourceimages,
                                    TextureNodeSettings* settings) const
{
   if (!settings || !destimage || !size.isValid()) {
      return;
   }

   QColor color = settings->value("color").value<QColor>();
   QString fontname = settings->value("fontname").toString();
   QString text = settings->value("text").toString();
   double fontsize = settings->value("fontsize").toDouble() * size.height() / 100;
   double rotation = settings->value("rotation").toDouble();
   int offsetLeft = settings->value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings->value("offsettop").toDouble() * size.height() / 100;

   QFont::StyleHint styleHint = QFont::StyleHint::AnyStyle;
   if (fontname == "Times") {
      styleHint = QFont::StyleHint::Times;
   } else if (fontname == "Courier") {
      styleHint = QFont::StyleHint::Courier;
   } else if (fontname == "OldEnglish") {
      styleHint = QFont::StyleHint::OldEnglish;
   } else if (fontname == "System") {
      styleHint = QFont::StyleHint::System;
   } else if (fontname == "Cursive") {
      styleHint = QFont::StyleHint::Cursive;
   } else if (fontname == "Monospace") {
      styleHint = QFont::StyleHint::Monospace;
   } else if (fontname == "Fantasy") {
      styleHint = QFont::StyleHint::Fantasy;
   }

   if (sourceimages.contains(0)) {
      memcpy(destimage, sourceimages.value(0)->getData(), size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = QImage(size.width(), size.height(), QImage::Format_RGB32);
   memcpy(tempimage.bits(), destimage, size.width() * size.height() * sizeof(TexturePixel));

   offsetLeft += (double) 50 * size.width() / 100;
   offsetTop += (double) 50 * size.height() / 100;

   QFont font;
   font.setPixelSize(fontsize);
   font.setStyleHint(styleHint);
   font.setFamily(font.defaultFamily());
   QFontMetrics fm(font);

   QPainter painter(&tempimage);
   painter.translate(offsetLeft, offsetTop);
   painter.rotate(rotation);
   painter.translate(-fm.width(text) / 2, -fm.height() / 2);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setFont(font);
   painter.setPen(color);
   painter.drawText(QRect(0, 0, size.width() * 10, size.height() * 10), text);

   memcpy(destimage, tempimage.bits(), size.width() * size.height() * sizeof(TexturePixel));
}
