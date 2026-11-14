#include "base/projectfileservice.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/builtinregistry.h"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include "base/editmanager.h"
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
   /// @brief Checks RGBA colour storage and invalid colour rejection.
   void storesRgbaColours();
   /// @brief Checks saved field order and JSON escaping in files and clipboard copies.
   void savesOrderedNodeFields();
   /// @brief Checks editable IDs, undo, clipboard copies, and saved references.
   void editsStringIdsAndPreservesReferences();
   /// @brief Supplies tracked examples and their expected graph sizes.
   void loadsTrackedExamples_data();

   /// @brief Verifies that tracked example projects load with intact graphs.
   void loadsTrackedExamples();

   /// @brief Verifies that saving and reloading preserves graph semantics.
   void roundTripsSemantically();

   /// @brief Verifies connections are saved with canonical input names.
   void savesCanonicalSlotNames();

   /// @brief Supplies portable and platform-characterizing render hashes.
   void rendersStableRawHashes_data();

   /// @brief Verifies stable raw pixel output for representative projects.
   void rendersStableRawHashes();

   /// @brief Supplies malformed project documents rejected by validation.
   void rejectsMalformedDocumentsTransactionally_data();

   /// @brief Verifies invalid documents do not partially replace the active project.
   void rejectsMalformedDocumentsTransactionally();

   /// @brief Verifies missing generator errors identify the file and every generator.
   void reportsUnavailableGenerators();

   /// @brief Verifies input parsing and output-writing error reporting.
   void reportsFileErrors();
};

void ProjectFileServiceTest::storesRgbaColours() {
   const QColor colour(12, 34, 56, 78);
   QCOMPARE(ProjectFileService::parseColor("#0c22384e"), colour);
   QCOMPARE(ProjectFileService::parseColor("#0c2238"), QColor(12, 34, 56));
   QVERIFY(!ProjectFileService::parseColor("#gg22384e").isValid());
   QVERIFY(!ProjectFileService::parseColor("#12345").isValid());
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto node = project.newNode(0, project.getGenerator("Fill"));
   auto settings = node->getSettings();
   settings["color"] = colour;
   node->setSettings(settings);
   QCOMPARE(project.saveAsJSON()
                .array()
                .first()
                .toObject()
                .value("settings")
                .toObject()
                .value("color")
                .toString(),
            QString("#0c22384e"));
   QTemporaryDir directory;
   const QString path = directory.filePath("rgba.ptmg");
   QVERIFY(ProjectFileService::save(path, project, false));
   TextureProject loaded(false);
   registerBuiltInGenerators(loaded);
   QVERIFY(ProjectFileService::load(path, loaded));
   QCOMPARE(loaded.getNode(node->getIdentifier())->getSettings().value("color").value<QColor>(),
            colour);
}

void ProjectFileServiceTest::savesOrderedNodeFields() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto node = project.newNode(0, project.getGenerator("Fill"));
   node->setName(QString::fromUtf8("A \"quoted\" \\ node\nwith a newline and \xc3\xa5"));
   QTemporaryDir directory;
   const QString path = directory.filePath("ordered.ptmg");
   QVERIFY(ProjectFileService::save(path, project, false));
   QFile file(path);
   QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
   const auto json = file.readAll();
   qsizetype previous = -1;
   for (const auto& field : {"id", "name", "position", "generator", "settings", "slots"}) {
      const auto position = json.indexOf(QByteArray("\"") + field + "\":");
      QVERIFY(position > previous);
      previous = position;
   }
   QCOMPARE(QJsonDocument::fromJson(json), project.saveAsJSON());
   QCOMPARE(project.serializeNode(node->getId()).toUtf8(), json);
}

void ProjectFileServiceTest::editsStringIdsAndPreservesReferences() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto source = project.newNode(0, project.getGenerator("Fill"));
   const auto output = project.newNode(0, project.getGenerator("Blend"));
   QVERIFY(!source->getIdentifier().isEmpty());
   QVERIFY(source->getIdentifier() != output->getIdentifier());
   QVERIFY(source->setIdentifier("base"));
   QVERIFY(output->setIdentifier("result"));
   QVERIFY(output->setSourceSlot("Background", source->getId()));
   EditManager edits(project);
   edits.reset();
   QVERIFY(!edits.changeNodeIdentifier(source->getId(), "result"));
   QVERIFY(!edits.changeNodeIdentifier(source->getId(), ""));
   QVERIFY(!edits.changeNodeIdentifier(source->getId(), "   "));
   QVERIFY(!edits.changeNodeIdentifier(source->getId(), " result "));
   QVERIFY(edits.changeNodeIdentifier(source->getId(), " base "));
   QVERIFY(edits.isClean());
   QVERIFY(edits.changeNodeIdentifier(source->getId(), " background "));
   edits.stack().undo();
   QCOMPARE(source->getIdentifier(), QString("base"));
   edits.stack().redo();
   QCOMPARE(source->getIdentifier(), QString("background"));
   QVERIFY(edits.removeNode(source->getId()));
   edits.stack().undo();
   QCOMPARE(project.getNode("background")->getId(), source->getId());
   const QString clipboard = project.serializeNode(output->getId());
   QCOMPARE(project.pasteNodes(clipboard), 1);
   QCOMPARE(project.getNumNodes(), 3);
   QSet<QString> ids;
   for (int id : project.getNodeIds()) {
      ids.insert(project.getNode(id)->getIdentifier());
   }
   QCOMPARE(ids.size(), 3);
   QTemporaryDir directory;
   const QString path = directory.filePath("ids.ptmg");
   QVERIFY(ProjectFileService::save(path, project, false));
   TextureProject loaded(false);
   registerBuiltInGenerators(loaded);
   QVERIFY(ProjectFileService::load(path, loaded));
   const auto reloaded = loaded.getNode("result");
   QVERIFY(reloaded);
   QCOMPARE(loaded.getNode(reloaded->getSources().value("Background"))->getIdentifier(),
            QString("background"));
   QVERIFY(!loaded.getNode("base"));
}

void ProjectFileServiceTest::loadsTrackedExamples_data() {
   QTest::addColumn<QString>("name");
   QTest::addColumn<int>("nodes");
   QTest::addColumn<int>("edges");
   QTest::addColumn<int>("sink");
   QTest::newRow("wall") << "wall.ptmg" << 11 << 10 << 13;
   QTest::newRow("space") << "space.ptmg" << 44 << 43 << 30;
   QTest::newRow("rose") << "rose.ptmg" << 19 << 19 << 32;
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
       ProjectFileService::load(QStringLiteral(PTM_SOURCE_DIR "/examples/rose.ptmg"), source);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   const QByteArray before = source.saveAsJSON().toJson();

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("round trip.ptmg"));
   result = ProjectFileService::save(path, source, false);
   QVERIFY2(result.succeeded(), qPrintable(result.message));

   TextureProject reloaded(false);
   registerBuiltInGenerators(reloaded);
   result = ProjectFileService::load(path, reloaded);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   QCOMPARE(reloaded.saveAsJSON().toJson(), before);
}

void ProjectFileServiceTest::savesCanonicalSlotNames() {
   const QString json = QStringLiteral(
       R"([
  {
    "id": "1",
    "name": "source",
    "generator": "Fill"
  },
  {
    "id": "2",
    "name": "displaced",
    "generator": "Displacement",
    "slots": {
      "Source image": "1",
      "Map": "1"
    }
  }
])");

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString inputPath = directory.filePath(QStringLiteral("input.ptmg"));
   QFile inputFile(inputPath);
   QVERIFY(inputFile.open(QIODevice::WriteOnly));
   QCOMPARE(inputFile.write(json.toUtf8()), qint64(json.toUtf8().size()));
   inputFile.close();

   TextureProject project(false);
   registerBuiltInGenerators(project);
   ProjectFileResult result = ProjectFileService::load(inputPath, project);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   const TextureNodePtr displaced = project.getNode(2);
   QCOMPARE(displaced->getSources().value(QStringLiteral("Source image")), 1);
   QCOMPARE(displaced->getSources().value(QStringLiteral("Map")), 1);

   const QString namedPath = directory.filePath(QStringLiteral("named.ptmg"));
   result = ProjectFileService::save(namedPath, project, false);
   QVERIFY2(result.succeeded(), qPrintable(result.message));
   QFile namedFile(namedPath);
   QVERIFY(namedFile.open(QIODevice::ReadOnly));
   const QString saved = QString::fromUtf8(namedFile.readAll());
   QVERIFY(saved.contains(QStringLiteral("\"Source image\"")));
   QVERIFY(saved.contains(QStringLiteral("\"Map\"")));
   QVERIFY(!saved.contains(QStringLiteral("\"0\"")));
   QVERIFY(!saved.contains(QStringLiteral("\"Slot 2\"")));
}

void ProjectFileServiceTest::rendersStableRawHashes_data() {
   QTest::addColumn<QString>("path");
   QTest::addColumn<int>("sink");
   QTest::addColumn<QSize>("size");
   QTest::addColumn<QByteArray>("expectedDigest");
   QTest::newRow("portable-fill")
       << QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/minimal-fill.ptmg") << 1
       << QSize(17, 13)
       << QByteArray("88570e6e206362c8ff826f3a441691ac49ab3b970bb62b490e4a3437d0416ae1");
#ifdef Q_OS_WIN
   const QByteArray wallDigest("d47c98c22144a02409d13685f39102c2442a90e190f330982fa40913fc69f59b");
#else
   const QByteArray wallDigest;
#endif
   QTest::newRow("wall") << QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg") << 13
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
   QTest::addColumn<QString>("json");
   QTest::newRow("wrong-root") << "{}";
   QTest::newRow("duplicate-id") << R"([
  {
    "id": "1",
    "name": "a",
    "generator": "Fill"
  },
  {
    "id": "1",
    "name": "b",
    "generator": "Fill"
  }
])";
   QTest::newRow("unknown-generator") << R"([
  {
    "id": "1",
    "name": "a",
    "generator": "Missing"
  }
])";
   QTest::newRow("missing-source") << R"([
  {
    "id": "1",
    "name": "a",
    "generator": "Blend",
    "slots": {
      "Background": "2"
    }
  }
])";
   QTest::newRow("cycle") << R"([
  {
    "id": "1",
    "name": "a",
    "generator": "Blend",
    "slots": {
      "Background": "2"
    }
  },
  {
    "id": "2",
    "name": "b",
    "generator": "Blend",
    "slots": {
      "Background": "1"
    }
  }
])";
   QTest::newRow("unknown-named-slot") << R"([
  {
    "id": "1",
    "name": "a",
    "generator": "Blend",
    "slots": {
      "Missing": "2"
    }
  },
  {
    "id": "2",
    "name": "b",
    "generator": "Fill"
  }
])";
   QTest::newRow("non-string-reference") << R"([
  {
    "id": "node",
    "name": "Node",
    "generator": "Blend",
    "slots": {
      "Background": 2
    }
  }
])";
   QTest::newRow("numeric-id") << R"([
  {
    "id": 1,
    "name": "Node",
    "generator": "Fill"
  }
])";
   QTest::newRow("invalid-settings") << R"([
  {
    "id": "node",
    "name": "Node",
    "generator": "Fill",
    "settings": {
      "color": 123
    }
  }
])";
   QTest::newRow("invalid-position") << R"([
  {
    "id": "node",
    "name": "Node",
    "position": {
      "x": "left",
      "y": 0
    },
    "generator": "Fill"
  }
])";
}

void ProjectFileServiceTest::rejectsMalformedDocumentsTransactionally() {
   QFETCH(QString, json);
   TextureProject project(false);
   registerBuiltInGenerators(project);
   project.newNode(9, project.getGenerator(QStringLiteral("Fill")));
   const TextureImagePtr cached = project.getNode(9)->renderImage(QSize(2, 2));
   const bool dirtyBefore = project.isModified();

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("invalid.ptmg")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   const QByteArray data = json.toUtf8();
   QCOMPARE(file.write(data), qint64(data.size()));
   file.close();
   const ProjectFileResult result = ProjectFileService::load(file.fileName(), project);
   QCOMPARE(result.error, ProjectFileError::Validation);
   QCOMPARE(project.getNodeIds(), QList<int>({9}));
   QCOMPARE(project.isModified(), dirtyBefore);
   QCOMPARE(project.getNode(9)->cachedImage(QSize(2, 2)), cached);
}

void ProjectFileServiceTest::reportsUnavailableGenerators() {
   const QString json = QStringLiteral(
       R"([
  {
    "id": "1",
    "name": "",
    "generator": "Missing B"
  },
  {
    "id": "2",
    "name": "",
    "generator": "Missing A"
  },
  {
    "id": "3",
    "name": "",
    "generator": "Missing B"
  }
])");
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("missing generators.ptmg"));
   QFile file(path);
   QVERIFY(file.open(QIODevice::WriteOnly));
   QCOMPARE(file.write(json.toUtf8()), qint64(json.toUtf8().size()));
   file.close();

   TextureProject project(false);
   registerBuiltInGenerators(project);
   const ProjectFileResult result = ProjectFileService::load(path, project);
   QCOMPARE(result.error, ProjectFileError::Validation);
   QCOMPARE(
       result.message,
       QStringLiteral("The project file '%1' could not be loaded because it references texture "
                      "generators that are not available: 'Missing A', 'Missing B'.")
           .arg(path));

   const QString singleJson = QStringLiteral(
       R"([
  {
    "id": "1",
    "name": "",
    "generator": "Missing"
  }
])");
   QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
   QCOMPARE(file.write(singleJson.toUtf8()), qint64(singleJson.toUtf8().size()));
   file.close();
   const ProjectFileResult singleResult = ProjectFileService::load(path, project);
   QCOMPARE(singleResult.error, ProjectFileError::Validation);
   QCOMPARE(singleResult.message,
            QStringLiteral("The project file '%1' could not be loaded because it references a "
                           "texture generator that is not available: 'Missing'.")
                .arg(path));
}

void ProjectFileServiceTest::reportsFileErrors() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("broken.ptmg")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   QCOMPARE(file.write("["), qint64(1));
   file.close();

   TextureProject project(false);
   registerBuiltInGenerators(project);
   const ProjectFileResult result = ProjectFileService::load(file.fileName(), project);
   QCOMPARE(result.error, ProjectFileError::JsonParse);
   QVERIFY(result.message.contains(QStringLiteral("byte")));

   project.newNode(1, project.getGenerator(QStringLiteral("Fill")));
   const ProjectFileResult saveResult = ProjectFileService::save(
       directory.filePath(QStringLiteral("missing/child.ptmg")), project, false);
   QCOMPARE(saveResult.error, ProjectFileError::OutputOpen);
   QVERIFY(project.isModified());
}

QTEST_MAIN(ProjectFileServiceTest)
#include "projectfileservice_test.moc"
