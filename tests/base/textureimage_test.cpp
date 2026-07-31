#include "base/textureexporter.h"
#include "base/textureimage.h"
#include <QTest>
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
   QCOMPARE(bytes[0], static_cast<unsigned char>(0x33));
   QCOMPARE(bytes[1], static_cast<unsigned char>(0x22));
   QCOMPARE(bytes[2], static_cast<unsigned char>(0x11));
   QCOMPARE(bytes[3], static_cast<unsigned char>(0x44));
   QCOMPARE(image.data()[0].toRGBA(), quint32(0x11223344));

   QString error;
   const QImage converted = TextureExporter::toQImage(image, &error);
   QVERIFY2(!converted.isNull(), qPrintable(error));
   QCOMPARE(converted.pixelColor(0, 0), QColor(0x11, 0x22, 0x33, 0x44));

   TextureImage moved(std::move(image));
   QCOMPARE(moved.getSize(), QSize(3, 2));
}

QTEST_APPLESS_MAIN(TextureImageTest)
#include "textureimage_test.moc"
