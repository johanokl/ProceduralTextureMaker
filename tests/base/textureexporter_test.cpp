#include "base/jstexgen.h"
#include "base/textureexporter.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "support/testgenerators.h"
#include <QFile>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

namespace {

/// @brief Creates a generator that can only finish through cancellation.
TextureGeneratorPtr infiniteGenerator() {
   return TextureGeneratorPtr(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'Infinite export',type:'generator',inputs:[],"
       "generate(){while(true){}}};")));
}

/// @brief Reads the single completion result recorded by a signal spy.
TextureExportResult outcome(const QSignalSpy& spy) {
   return qvariant_cast<TextureExportResult>(spy.first().first());
}

}  // namespace

/// @brief Verifies asynchronous export, cancellation and snapshot isolation.
class TextureExporterTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Checks output pixels and ensures unrelated nodes and model caches are untouched.
   void exportsOnlyRequiredNodes();
   /// @brief Checks that editing a live node does not change an export already started.
   void usesCapturedSettings();
   /// @brief Checks cancellation stays responsive while an active native node finishes.
   void cancelsNativeRenderAfterNodeFinishes();
   /// @brief Checks cancellation leaves an existing destination unchanged and events responsive.
   void cancelsStuckJavaScript();
   /// @brief Checks render errors and output errors reach the caller.
   void reportsFailures();
   /// @brief Checks a long dependency chain is rendered without recursive evaluation.
   void rendersDeepGraph();
};

void TextureExporterTest::exportsOnlyRequiredNodes() {
   TextureProject project(false);
   const TextureGeneratorPtr source(new RecordingGenerator(QStringLiteral("Source"), 0, 42));
   const TextureGeneratorPtr filter(new NamedInputGenerator());
   const auto first = project.newNode(1, source);
   const auto second = project.newNode(2, filter);
   QVERIFY(second->setSourceSlot(QStringLiteral("Zulu"), 1));
   project.newNode(3, infiniteGenerator());
   const QSize size(7, 5);
   const QImage expected = TextureExporter::toQImage(*second->renderImage(size));
   // Use a fresh size so this assertion also detects export cache pollution.
   const QSize exportSize(11, 9);
   QTemporaryDir directory;
   const QString path = directory.filePath(QStringLiteral("image.png"));
   TextureExporter exporter;
   QSignalSpy finished(&exporter, &TextureExporter::finished);
   QSignalSpy progress(&exporter, &TextureExporter::progress);
   exporter.start(project, 2, exportSize, path, false);
   QTRY_COMPARE(finished.count(), 1);
   QVERIFY2(outcome(finished).succeeded(), qPrintable(outcome(finished).message));
   const QImage actual(path);
   QCOMPARE(actual.size(), exportSize);
   QCOMPARE(actual.pixelColor(0, 0), expected.pixelColor(0, 0));
   QCOMPARE(progress.count(), 3);
   QVERIFY(first->cachedImage(exportSize).isNull());
   QVERIFY(second->cachedImage(exportSize).isNull());
}

void TextureExporterTest::usesCapturedSettings() {
   TextureProject project(false);
   auto generator = QSharedPointer<RecordingGenerator>::create();
   const auto node = project.newNode(1, generator);
   node->setSettings({{QStringLiteral("value"), 22}});
   generator->block();
   QTemporaryDir directory;
   const QString path = directory.filePath(QStringLiteral("image.png"));
   TextureExporter exporter;
   QSignalSpy finished(&exporter, &TextureExporter::finished);
   exporter.start(project, 1, QSize(2, 2), path, false);
   const auto release = qScopeGuard([&] { generator->release(); });
   const bool started = generator->waitUntilStarted();
   node->setSettings({{QStringLiteral("value"), 99}});
   generator->release();
   QVERIFY(started);
   QTRY_COMPARE(finished.count(), 1);
   QVERIFY(outcome(finished).succeeded());
   QCOMPARE(QImage(path).pixelColor(0, 0).red(), 22);
}

void TextureExporterTest::cancelsNativeRenderAfterNodeFinishes() {
   TextureProject project(false);
   auto generator = QSharedPointer<RecordingGenerator>::create();
   project.newNode(1, generator);
   generator->block();
   QTemporaryDir directory;
   const QString path = directory.filePath(QStringLiteral("image.png"));
   TextureExporter exporter;
   const auto release = qScopeGuard([&] { generator->release(); });
   QSignalSpy finished(&exporter, &TextureExporter::finished);
   exporter.start(project, 1, QSize(2, 2), path, false);
   const bool started = generator->waitUntilStarted();
   exporter.cancel();
   bool eventDelivered = false;
   QTimer::singleShot(100, this, [&] { eventDelivered = true; });
   QTRY_VERIFY(eventDelivered);
   QCOMPARE(finished.count(), 0);
   generator->release();
   QVERIFY(started);
   QTRY_COMPARE(finished.count(), 1);
   QCOMPARE(outcome(finished).error, TextureExportError::Cancelled);
   QVERIFY(!QFile::exists(path));
}

void TextureExporterTest::cancelsStuckJavaScript() {
   TextureProject project(false);
   project.newNode(1, infiniteGenerator());
   QTemporaryDir directory;
   const QString path = directory.filePath(QStringLiteral("image.png"));
   QFile existing(path);
   QVERIFY(existing.open(QIODevice::WriteOnly));
   existing.write("previous image");
   existing.close();
   TextureExporter exporter;
   QSignalSpy finished(&exporter, &TextureExporter::finished);
   int ticks = 0;
   QTimer heartbeat;
   connect(&heartbeat, &QTimer::timeout, this, [&] { ++ticks; });
   heartbeat.start(10);
   exporter.start(project, 1, QSize(2, 2), path, true);
   QTRY_VERIFY(ticks >= 3);
   QCOMPARE(finished.count(), 0);
   exporter.cancel();
   QTRY_COMPARE(finished.count(), 1);
   QCOMPARE(outcome(finished).error, TextureExportError::Cancelled);
   QVERIFY(existing.open(QIODevice::ReadOnly));
   QCOMPARE(existing.readAll(), QByteArray("previous image"));
}

void TextureExporterTest::reportsFailures() {
   TextureProject project(false);
   const TextureGeneratorPtr generator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'Broken export',type:'generator',inputs:[],"
       "generate(){throw Error('export test failure');}};")));
   project.newNode(1, generator);
   QTemporaryDir directory;
   TextureExporter broken;
   QSignalSpy failed(&broken, &TextureExporter::finished);
   const QString path = directory.filePath(QStringLiteral("image.png"));
   broken.start(project, 1, QSize(2, 2), path, false);
   QTRY_COMPARE(failed.count(), 1);
   QCOMPARE(outcome(failed).error, TextureExportError::Render);
   QVERIFY(outcome(failed).message.contains(QStringLiteral("export test failure")));
   QVERIFY(!QFile::exists(path));

   project.newNode(2, TextureGeneratorPtr(new RecordingGenerator()));
   TextureExporter unwritable;
   QSignalSpy writeFailed(&unwritable, &TextureExporter::finished);
   unwritable.start(project, 2, QSize(2, 2), directory.filePath("missing/image.png"), false);
   QTRY_COMPARE(writeFailed.count(), 1);
   QCOMPARE(outcome(writeFailed).error, TextureExportError::OutputOpen);
}

void TextureExporterTest::rendersDeepGraph() {
   TextureProject project(false);
   project.newNode(1, TextureGeneratorPtr(new RecordingGenerator()));
   const TextureGeneratorPtr filter(new NamedInputGenerator());
   constexpr int depth = 128;
   for (int id = 2; id <= depth; ++id) {
      const auto node = project.newNode(id, filter);
      QVERIFY(node->setSourceSlot(QStringLiteral("Zulu"), id - 1));
   }
   QTemporaryDir directory;
   TextureExporter exporter;
   QSignalSpy finished(&exporter, &TextureExporter::finished);
   const QString path = directory.filePath(QStringLiteral("deep.png"));
   exporter.start(project, depth, QSize(1, 1), path, false);
   QTRY_COMPARE(finished.count(), 1);
   QVERIFY2(outcome(finished).succeeded(), qPrintable(outcome(finished).message));
   QCOMPARE(QImage(path).pixelColor(0, 0).red(), 17);
}

QTEST_GUILESS_MAIN(TextureExporterTest)
#include "textureexporter_test.moc"
