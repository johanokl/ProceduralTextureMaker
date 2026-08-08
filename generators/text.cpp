
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "text.h"
#include <QPainter>
#include <cmath>

TextTextureGenerator::TextTextureGenerator() {
   TextureGeneratorSetting colorsetting;
   colorsetting.name = "Colour";
   colorsetting.description = "Colour used to render the text.";
   colorsetting.defaultvalue = QVariant(QColor(255, 0, 0));
   colorsetting.id = "color";
   configurables.append(colorsetting);

   TextureGeneratorSetting text;
   text.name = "Text";
   text.description = "Text content to render; multiple lines are supported.";
   text.defaultvalue = QVariant(QString("Text"));
   text.multiline = true;
   text.id = "text";
   configurables.append(text);

   TextureGeneratorSetting alignment;
   QStringList alignments;
   alignments.append("Left");
   alignments.append("Center");
   alignments.append("Right");
   alignment.name = "Alignment";
   alignment.description = "Sets the horizontal alignment of the text within the texture.";
   alignment.defaultvalue = QVariant(alignments);
   alignment.defaultindex = 1;
   alignment.id = "alignment";
   configurables.append(alignment);

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
   font.name = "Font category";
   font.description = "Generic font category used to select a suitable installed typeface.";
   font.defaultvalue = QVariant(fonts);
   font.id = "fontname";
   configurables.append(font);

   TextureGeneratorSetting fontsize;
   fontsize.name = "Font size (%)";
   fontsize.description = "Text size as a percentage of the texture height.";
   fontsize.defaultvalue = QVariant((double)20);
   fontsize.min = QVariant(1);
   fontsize.max = QVariant(200);
   fontsize.id = "fontsize";
   configurables.append(fontsize);

   TextureGeneratorSetting rotation;
   rotation.name = "Rotation (°)";
   rotation.description = "Rotates the text around the texture centre.";
   rotation.defaultvalue = QVariant((double)0);
   rotation.min = QVariant(-360);
   rotation.max = QVariant(360);
   rotation.id = "rotation";
   configurables.append(rotation);

   TextureGeneratorSetting offsetLeft;
   offsetLeft.name = "Horizontal offset (%)";
   offsetLeft.description = "Moves the text horizontally from its aligned position.";
   offsetLeft.defaultvalue = QVariant((double)0);
   offsetLeft.min = QVariant(-200);
   offsetLeft.max = QVariant(200);
   offsetLeft.id = "offsetleft";
   configurables.append(offsetLeft);

   TextureGeneratorSetting offsetTop;
   offsetTop.name = "Vertical offset (%)";
   offsetTop.description = "Moves the text vertically from its aligned position.";
   offsetTop.defaultvalue = QVariant((double)0);
   offsetTop.min = QVariant(-200);
   offsetTop.max = QVariant(200);
   offsetTop.id = "offsettop";
   configurables.append(offsetTop);

   TextureGeneratorSetting antialiasing;
   antialiasing.defaultvalue = QVariant((bool)true);
   antialiasing.name = "Antialiasing";
   antialiasing.description = "Smooths the edges of rendered glyphs.";
   antialiasing.id = "antialiasing";
   configurables.append(antialiasing);
}
void TextTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                    const QMap<QString, TextureImagePtr>& sourceimages,
                                    const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }

   QColor color = settings.value("color").value<QColor>();
   QString fontname = settings.value("fontname").toString();
   QString text = settings.value("text").toString();
   QString alignment = settings.value("alignment").toString();
   double fontsize = settings.value("fontsize").toDouble() * size.height() / 100;
   double rotation = settings.value("rotation").toDouble();
   int offsetLeft = settings.value("offsetleft").toDouble() * size.width() / 100;
   int offsetTop = settings.value("offsettop").toDouble() * size.height() / 100;
   bool antialiasing = settings.value("antialiasing").toBool();

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

   if (sourceimages.contains(QStringLiteral("Canvas"))) {
      memcpy(destimage, sourceimages.value(QStringLiteral("Canvas"))->getData(),
             size.width() * size.height() * sizeof(TexturePixel));
   } else {
      memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));
   }

   QImage tempimage = makeTextureImageView(size, destimage);

   offsetLeft += (double)50 * size.width() / 100;
   offsetTop += (double)50 * size.height() / 100;

   QFont font;
   font.setPixelSize(fontsize);
   font.setStyleHint(styleHint);
   font.setFamily(font.defaultFamily());
   QFontMetrics fm(font);

   const QStringList lines = text.split('\n');
   int textWidth = 1;
   for (const QString& line : lines) {
      textWidth = qMax(textWidth, fm.horizontalAdvance(line));
   }
   const int textHeight = fm.height() + qMax(0, lines.size() - 1) * fm.lineSpacing();
   const QRect textRect(-textWidth / 2, -textHeight / 2, textWidth, textHeight);
   Qt::Alignment textAlignment = Qt::AlignHCenter | Qt::AlignTop;
   if (alignment == "Left") {
      textAlignment = Qt::AlignLeft | Qt::AlignTop;
   } else if (alignment == "Right") {
      textAlignment = Qt::AlignRight | Qt::AlignTop;
   }

   QPainter painter(&tempimage);
   painter.translate(offsetLeft, offsetTop);
   painter.rotate(rotation);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.setRenderHint(QPainter::Antialiasing, antialiasing);
   painter.setFont(font);
   painter.setPen(color);
   painter.drawText(textRect, textAlignment, text);
}
