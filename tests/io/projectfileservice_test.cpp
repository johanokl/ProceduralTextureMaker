#include "base/projectfileservice.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/builtinregistry.h"
#include <QCryptographicHash>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

namespace {

/// @brief Counts all source connections in a project graph.
/// @param project Project whose graph is inspected.
/// @return Total number of connected source slots.
int edgeCount(const TextureProject& project) {
   int count = 0;
   for (const int id : project.getNodeIds()) {
      for (const int sourceId : project.getNode(id)->getSources()) {
         if (sourceId != 0) {
            ++count;
         }
      }
   }
   return count;
}

}  // namespace

/// @brief Verifies project-file compatibility, validation, rendering, and errors.
class ProjectFileServiceTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Supplies tracked examples and their expected graph sizes.
   void loadsTrackedExamples_data();

   /// @brief Verifies that tracked example projects load with intact graphs.
   void loadsTrackedExamples();

   /// @brief Verifies that saving and reloading preserves graph semantics.
   void roundTripsSemantically();

   /// @brief Supplies portable and platform-characterizing render hashes.
   void rendersStableRawHashes_data();

   /// @brief Verifies stable raw pixel output for representative projects.
   void rendersStableRawHashes();

   /// @brief Supplies malformed project documents rejected by validation.
   void rejectsMalformedDocumentsTransactionally_data();

   /// @brief Verifies invalid documents do not partially replace the active project.
   void rejectsMalformedDocumentsTransactionally();

   /// @brief Verifies input parsing and output-writing error reporting.
   void reportsFileErrors();
};

void ProjectFileServiceTest::loadsTrackedExamples_data() {
   QTest::addColumn<QString>("name");
   QTest::addColumn<int>("nodes");
   QTest::addColumn<int>("edges");
   QTest::addColumn<int>("sink");
   QTest::newRow("wall") << "wall.txl" << 11 << 10 << 13;
   QTest::newRow("space") << "space.txl" << 44 << 43 << 30;
   QTest::newRow("rose") << "rose.txl" << 19 << 19 << 32;
}

void ProjectFileServiceTest::loadsTrackedExamples() {
   QFETCH(QString, name);
   QFETCH(int, nodes);
   QFETCH(int, edges);
   QFETCH(int, sink);
   TextureProject project(false);
   registerBuiltInGenerators(project);
   project.newNode(999, project.getGenerator(QStringLiteral("Fill")));
   const QString path = QStringLiteral(PTM_SOURCE_DIR "/examples/") + name;
   const ProjectFileResult result = ProjectFileService::load(path, project);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   QCOMPARE(project.getNumNodes(), nodes);
   QVERIFY(project.getNode(999).isNull());
   QCOMPARE(edgeCount(project), edges);
   QCOMPARE(project.getSinkNodeIds(), QList<int>({sink}));
   QVERIFY(!project.findLoops());
   QVERIFY(!project.isModified());
}

void ProjectFileServiceTest::roundTripsSemantically() {
   TextureProject source(false);
   registerBuiltInGenerators(source);
   ProjectFileResult result =
       ProjectFileService::load(QStringLiteral(PTM_SOURCE_DIR "/examples/rose.txl"), source);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   const QByteArray before = source.saveAsXML().toByteArray();

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("round trip.txl"));
   result = ProjectFileService::save(path, source, false);
   QVERIFY2(result.succeeded(), qPrintable(result.message));

   TextureProject reloaded(false);
   registerBuiltInGenerators(reloaded);
   result = ProjectFileService::load(path, reloaded);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   QCOMPARE(reloaded.saveAsXML().toByteArray(), before);
}

void ProjectFileServiceTest::rendersStableRawHashes_data() {
   QTest::addColumn<QString>("path");
   QTest::addColumn<int>("sink");
   QTest::addColumn<QSize>("size");
   QTest::addColumn<QByteArray>("expectedDigest");
   QTest::newRow("portable-fill")
       << QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/minimal-fill.txl") << 1
       << QSize(17, 13)
       << QByteArray("7ed60ca9ab1418340d89ae074e6a5427ad490ff8547c74402857d0ea33359043");
#ifdef Q_OS_WIN
   const QByteArray wallDigest("4cf9451594d719c6bace41035604d54ff7979b4dee1c776356bebf621f153ab1");
#else
   const QByteArray wallDigest;
#endif
   QTest::newRow("wall") << QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl") << 13
                         << QSize(32, 32) << wallDigest;
}

void ProjectFileServiceTest::rendersStableRawHashes() {
   QFETCH(QString, path);
   QFETCH(int, sink);
   QFETCH(QSize, size);
   QFETCH(QByteArray, expectedDigest);
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const ProjectFileResult result = ProjectFileService::load(path, project);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   const TextureImagePtr image = project.getNode(sink)->renderImage(size);
   const QByteArray bytes(reinterpret_cast<const char*>(image->data()),
                          static_cast<qsizetype>(image->byteSize()));
   const QByteArray digest = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
   QCOMPARE(digest.size(), qsizetype(64));
   if (!expectedDigest.isEmpty()) {
      QCOMPARE(digest, expectedDigest);
   }
}

void ProjectFileServiceTest::rejectsMalformedDocumentsTransactionally_data() {
   QTest::addColumn<QString>("xml");
   QTest::newRow("wrong-root") << "<Wrong><Nodes/></Wrong>";
   QTest::newRow("duplicate-id")
       << "<TextureSet><Nodes><Node id='1' name='a'><generator name='Fill'/></Node>"
          "<Node id='1' name='b'><generator name='Fill'/></Node></Nodes></TextureSet>";
   QTest::newRow("unknown-generator")
       << "<TextureSet><Nodes><Node id='1' name='a'><generator name='Missing'/></Node>"
          "</Nodes></TextureSet>";
   QTest::newRow("missing-source")
       << "<TextureSet><Nodes><Node id='1' name='a'><generator name='Blending'/>"
          "<Sources><source slot='0' source='2'/></Sources></Node></Nodes></TextureSet>";
   QTest::newRow("cycle")
       << "<TextureSet><Nodes><Node id='1' name='a'><generator name='Blending'/>"
          "<Sources><source slot='0' source='2'/></Sources></Node>"
          "<Node id='2' name='b'><generator name='Blending'/>"
          "<Sources><source slot='0' source='1'/></Sources></Node></Nodes></TextureSet>";
}

void ProjectFileServiceTest::rejectsMalformedDocumentsTransactionally() {
   QFETCH(QString, xml);
   TextureProject project(false);
   registerBuiltInGenerators(project);
   project.newNode(9, project.getGenerator(QStringLiteral("Fill")));
   const TextureImagePtr cached = project.getNode(9)->renderImage(QSize(2, 2));
   const bool dirtyBefore = project.isModified();

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("invalid.txl")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   const QByteArray data = xml.toUtf8();
   QCOMPARE(file.write(data), qint64(data.size()));
   file.close();
   const ProjectFileResult result = ProjectFileService::load(file.fileName(), project);
   QCOMPARE(result.error, ProjectFileError::Validation);
   QCOMPARE(project.getNodeIds(), QList<int>({9}));
   QCOMPARE(project.isModified(), dirtyBefore);
   QCOMPARE(project.getNode(9)->cachedImage(QSize(2, 2)), cached);
}

void ProjectFileServiceTest::reportsFileErrors() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("broken.txl")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   QCOMPARE(file.write("<TextureSet><Nodes>"), qint64(19));
   file.close();

   TextureProject project(false);
   registerBuiltInGenerators(project);
   const ProjectFileResult result = ProjectFileService::load(file.fileName(), project);
   QCOMPARE(result.error, ProjectFileError::XmlParse);
   QVERIFY(result.message.contains(QStringLiteral("1:")));

   project.newNode(1, project.getGenerator(QStringLiteral("Fill")));
   const ProjectFileResult saveResult = ProjectFileService::save(
       directory.filePath(QStringLiteral("missing/child.txl")), project, false);
   QCOMPARE(saveResult.error, ProjectFileError::OutputOpen);
   QVERIFY(project.isModified());
}

QTEST_MAIN(ProjectFileServiceTest)
#include "projectfileservice_test.moc"
