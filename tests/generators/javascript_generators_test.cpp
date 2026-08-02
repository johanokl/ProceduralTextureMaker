#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/javascript.h"
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <stdexcept>
#include <utility>

namespace {

/// @brief Returns a compact valid JavaScript generator used by rendering tests.
QString packedScript() {
   return QStringLiteral(
       "var name='SolidJS';var numSlots=0;var separateColorChannels=false;"
       "function getSettings(){return {};}function generate(data){var a=JSON.parse(data);"
       "for(var i=0;i<a.imagewidth*a.imageheight;++i)dest[i]=0x11223344;return dest;}");
}

/// @brief Renders a JavaScript generator and returns its first output pixel.
/// @param script JavaScript source defining the generator.
/// @return The color produced for the first pixel.
QColor renderColor(const QString& script) {
   auto generator = TextureGeneratorPtr(new JsTexGen(script));
   TextureProject project(false);
   project.addGenerator(generator);
   const TexturePixel pixel = project.newNode(1, generator)->renderImage(QSize(2, 2))->data()[0];
   return QColor(pixel.r, pixel.g, pixel.b, pixel.a);
}

/// @brief Creates a one-pixel source image.
TextureImagePtr sourceImage(const QColor& color) {
   TextureImagePtr image = TextureImage::create(QSize(1, 1));
   image->data()[0] = TexturePixel(
       static_cast<unsigned char>(color.red()), static_cast<unsigned char>(color.green()),
       static_cast<unsigned char>(color.blue()), static_cast<unsigned char>(color.alpha()));
   return image;
}

/// @brief Runs a JavaScript generator directly with named source images.
QColor renderWithSources(const QString& script, QMap<QString, TextureImagePtr> sources) {
   JsTexGen generator(script);
   TexturePixel destination{};
   TextureNodeSettings settings;
   generator.generate(QSize(1, 1), &destination, std::move(sources), &settings);
   return QColor(destination.r, destination.g, destination.b, destination.a);
}

}  // namespace

/// @brief Verifies JavaScript generator execution, diagnostics, and discovery.
class JavaScriptGeneratorsTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies packed and channel-separated rendering and script errors.
   void rendersAndReportsErrors();

   /// @brief Verifies loading valid scripts from a directory while reporting invalid ones.
   void loadsDirectory();

   /// @brief Verifies named inputs and legacy positional input compatibility.
   void supportsNamedAndLegacyInputs();
};

void JavaScriptGeneratorsTest::rendersAndReportsErrors() {
   QCOMPARE(renderColor(packedScript()), QColor(0x11, 0x22, 0x33, 0x44));
   const QString channels = QStringLiteral(
       "var name='Channels';var numSlots=0;var separateColorChannels=true;"
       "function getSettings(){return {};}function generate(data){var a=JSON.parse(data);"
       "for(var i=0;i<a.imagewidth*a.imageheight;++i){dest[i*4]=3;dest[i*4+1]=2;"
       "dest[i*4+2]=1;dest[i*4+3]=4;}return dest;}");
   QCOMPARE(renderColor(channels), QColor(1, 2, 3, 4));
   QVERIFY(!JsTexGen(QStringLiteral("var name=;")).isValid());

   const QStringList failures{
       QStringLiteral("function generate(){throw new Error('planned');}"),
       QStringLiteral("function generate(){return [];}"),
   };
   for (const QString& generateFunction : failures) {
      const QString script = QStringLiteral(
                                 "var name='Failure';var numSlots=0;"
                                 "function getSettings(){return {};}") +
                             generateFunction;
      auto generator = TextureGeneratorPtr(new JsTexGen(script));
      TextureProject project(false);
      project.addGenerator(generator);
      const TextureNodePtr node = project.newNode(1, generator);
      QVERIFY_EXCEPTION_THROWN(static_cast<void>(node->renderImage(QSize(2, 2))),
                               std::runtime_error);
   }
}

void JavaScriptGeneratorsTest::loadsDirectory() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("solid.js")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   const QByteArray data = packedScript().toUtf8();
   QCOMPARE(file.write(data), qint64(data.size()));
   file.close();

   TextureProject project(false);
   QVERIFY(loadJavaScriptGenerators(project, directory.path()).isEmpty());
   QVERIFY(!project.getGenerator(QStringLiteral("SolidJS")).isNull());
   QVERIFY(
       !loadJavaScriptGenerators(project, directory.filePath(QStringLiteral("missing"))).isEmpty());
}

void JavaScriptGeneratorsTest::supportsNamedAndLegacyInputs() {
   const QString namedScript = QStringLiteral(
       "var name='Named';var separateColorChannels=false;"
       "function getSettings(){return {};}"
       "function getInputSlots(){return ['Left','Right'];}"
       "function generate(data,inputs){return inputs.Right;}");
   JsTexGen namedGenerator(namedScript);
   QVERIFY(namedGenerator.isValid());
   QCOMPARE(namedGenerator.getSourceSlots(),
            QStringList({QStringLiteral("Left"), QStringLiteral("Right")}));
   QCOMPARE(
       renderWithSources(namedScript, {{QStringLiteral("Left"), sourceImage(QColor(1, 2, 3, 4))},
                                       {QStringLiteral("Right"), sourceImage(QColor(9, 8, 7, 6))}}),
       QColor(9, 8, 7, 6));

   const QString legacyScript = QStringLiteral(
       "var name='Legacy';var numSlots=2;var separateColorChannels=false;"
       "function getSettings(){return {};}"
       "function generate(data,first,second){"
       "if(first!==undefined)throw new Error('missing placeholder');return second;}");
   JsTexGen legacyGenerator(legacyScript);
   QVERIFY(legacyGenerator.isValid());
   QCOMPARE(legacyGenerator.getSourceSlots(),
            QStringList({QStringLiteral("Input 1"), QStringLiteral("Input 2")}));
   QCOMPARE(renderWithSources(legacyScript,
                              {{QStringLiteral("Input 2"), sourceImage(QColor(20, 30, 40, 50))}}),
            QColor(20, 30, 40, 50));

   const QString duplicateScript = QStringLiteral(
       "var name='Duplicate';function getSettings(){return {};}"
       "function getInputSlots(){return ['Input','Input'];}function generate(){}");
   QVERIFY(!JsTexGen(duplicateScript).isValid());
}

QTEST_GUILESS_MAIN(JavaScriptGeneratorsTest)
#include "javascript_generators_test.moc"
