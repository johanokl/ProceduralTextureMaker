#include "base/textureexporter.h"
#include "base/textureimage.h"
#include <QColor>
#include <QTest>
#include <cmath>
#include <type_traits>

/// @brief Verifies texture-image storage, ownership, and pixel conversion behavior.
class TextureImageTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies storage traits, dimensions, pixel layout, and QImage conversion.
   void storageAndPixelLayout();
};

void TextureImageTest::storageAndPixelLayout() {
   static_assert(!std::is_copy_constructible_v<TextureImage>);
   static_assert(std::is_nothrow_move_constructible_v<TextureImage>);
   QVERIFY_EXCEPTION_THROWN(TextureImage(QSize(0, 1)), std::invalid_argument);
   QVERIFY_EXCEPTION_THROWN(TextureImage(QSize(-1, 3)), std::invalid_argument);

   static_assert(sizeof(TexturePixel) == 4);
   TextureImage image(QSize(3, 2));
   QCOMPARE(image.pixelCount(), std::size_t(6));
   QCOMPARE(image.byteSize(), std::size_t(6 * sizeof(TexturePixel)));
   image.data()[0] = TexturePixel(0x11, 0x22, 0x33, 0x44);
   const auto* bytes = reinterpret_cast<const unsigned char*>(image.data());
   QCOMPARE(bytes[0], static_cast<unsigned char>(0x11));
   QCOMPARE(bytes[1], static_cast<unsigned char>(0x22));
   QCOMPARE(bytes[2], static_cast<unsigned char>(0x33));
   QCOMPARE(bytes[3], static_cast<unsigned char>(0x44));
   QCOMPARE(image.data()[0].toRGBA(), quint32(0x11223344));

   QImage mutableView = image.toQImageView();
   QCOMPARE(mutableView.format(), QImage::Format_RGBA8888);
   QCOMPARE(mutableView.size(), QSize(3, 2));
   QCOMPARE(mutableView.bytesPerLine(), 3 * static_cast<qsizetype>(sizeof(TexturePixel)));
   QCOMPARE(mutableView.sizeInBytes(), 6 * static_cast<qsizetype>(sizeof(TexturePixel)));
   QCOMPARE(mutableView.constBits(), reinterpret_cast<const uchar*>(image.data()));
   mutableView.setPixelColor(1, 0, QColor(0x55, 0x66, 0x77, 0x88));
   QCOMPARE(image.data()[1].toRGBA(), quint32(0x55667788));

   const TextureImage& constImage = image;
   const QImage readOnlyView = constImage.toQImageView();
   QCOMPARE(readOnlyView.constBits(), reinterpret_cast<const uchar*>(image.data()));

   QImage owningCopy = image.toQImageCopy();
   QCOMPARE(owningCopy.format(), QImage::Format_RGBA8888);
   QVERIFY(owningCopy.constBits() != reinterpret_cast<const uchar*>(image.data()));
   image.data()[0] = TexturePixel(1, 2, 3, 4);
   QCOMPARE(owningCopy.pixelColor(0, 0), QColor(0x11, 0x22, 0x33, 0x44));

   const TexturePixel transparent;
   QCOMPARE(transparent.toRGBA(), quint32(0));
   QCOMPARE(TexturePixel(255, 255, 255, 255).intensity(), 1.0);
   QCOMPARE(TexturePixel(255, 255, 255, 0).intensityWithAlpha(), 0.0);
   QVERIFY(std::abs(TexturePixel(255, 255, 255, 128).intensityWithAlpha() - (128.0 / 255.0)) <
           1.0e-12);

   QString error;
   const QImage converted = TextureExporter::toQImage(image, &error);
   QVERIFY2(!converted.isNull(), qPrintable(error));
   QCOMPARE(converted.pixelColor(0, 0), QColor(1, 2, 3, 4));

   TextureImage moved(std::move(image));
   QCOMPARE(moved.getSize(), QSize(3, 2));
}

QTEST_APPLESS_MAIN(TextureImageTest)
#include "textureimage_test.moc"
