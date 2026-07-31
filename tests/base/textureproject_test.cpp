#include "base/projectfileservice.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "support/testgenerators.h"
#include <QTemporaryDir>
#include <QTest>

/// @brief Verifies graph ownership, rendering caches, copying, and saved state.
class TextureProjectTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies node identifiers, connections, disconnections, and removal.
   void maintainsGraphAndIds();
   /// @brief Verifies synchronous rendering caches and downstream invalidation.
   void cachesAndInvalidatesRenders();
   /// @brief Verifies clipboard-style copies and project saved-state tracking.
   void copiesAndTracksSavedState();
};

void TextureProjectTest::maintainsGraphAndIds() {
   TextureProject project(false);
   auto generator = TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Slots"), 2));
   project.addGenerator(generator);
   project.addGenerator(TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Slots"), 2)));
   QCOMPARE(project.getGenerator(QStringLiteral("Slots")), generator);

   const TextureNodePtr one = project.newNode(0, generator);
   const TextureNodePtr explicitNode = project.newNode(7, generator);
   const TextureNodePtr two = project.newNode(0, generator);
   QCOMPARE(project.getNodeIds(), QList<int>({1, 2, 7}));
   QCOMPARE(project.newNode(7, generator), explicitNode);

   QVERIFY(two->setSourceSlot(0, one->getId()));
   QVERIFY(two->setSourceSlot(1, one->getId()));
   QCOMPARE(one->getNumReceivers(), 1);
   QVERIFY(explicitNode->setSourceSlot(0, two->getId()));
   QVERIFY(!one->setSourceSlot(0, explicitNode->getId()));
   QVERIFY(!two->setSourceSlot(3, one->getId()));
   QVERIFY(!two->setSourceSlot(0, 999));
   QVERIFY(!project.findLoops());

   project.removeNode(two->getId());
   QCOMPARE(one->getNumReceivers(), 0);
   QCOMPARE(explicitNode->getSources().value(0), 0);
   project.clear();
   QCOMPARE(project.newNode(0, generator)->getId(), 1);
}

void TextureProjectTest::cachesAndInvalidatesRenders() {
   TextureProject project(false);
   auto* sourceGenerator = new RecordingGenerator(QStringLiteral("Source"), 0, 25);
   auto* filterGenerator = new RecordingGenerator(QStringLiteral("Filter"), 1, 50);
   project.addGenerator(TextureGeneratorPtr(sourceGenerator));
   project.addGenerator(TextureGeneratorPtr(filterGenerator));
   const TextureNodePtr source = project.newNode(1, project.getGenerator(QStringLiteral("Source")));
   const TextureNodePtr output = project.newNode(2, project.getGenerator(QStringLiteral("Filter")));
   const TextureNodePtr unrelated =
       project.newNode(3, project.getGenerator(QStringLiteral("Source")));
   QVERIFY(output->setSourceSlot(0, source->getId()));

   const QSize size(5, 3);
   const TextureImagePtr first = output->renderImage(size);
   QCOMPARE(output->renderImage(size), first);
   QCOMPARE(sourceGenerator->callCount(), 1);
   QCOMPARE(filterGenerator->callCount(), 1);
   const TextureImagePtr unrelatedImage = unrelated->renderImage(size);

   TextureNodeSettings settings = source->getSettings();
   settings[QStringLiteral("value")] = 91;
   source->setSettings(settings);
   QVERIFY(source->cachedImage(size).isNull());
   QVERIFY(output->cachedImage(size).isNull());
   QCOMPARE(unrelated->cachedImage(size), unrelatedImage);
   QVERIFY(output->renderImage(size) != first);
}

void TextureProjectTest::copiesAndTracksSavedState() {
   TextureProject project(false);
   auto generator = TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Clone")));
   project.addGenerator(generator);
   const TextureNodePtr node = project.newNode(1, generator);
   node->setName(QStringLiteral("Copied node"));
   node->setPos(QPointF(3.5, 8.0));

   QCOMPARE(project.pasteNodes(project.serializeNode(1)), 1);
   QCOMPARE(project.getNumNodes(), 2);
   QCOMPARE(project.getNode(2)->getName(), QStringLiteral("Copied node"));
   QCOMPARE(project.pasteNodes(QStringLiteral("not xml")), 0);

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("project.txl"));
   QVERIFY(ProjectFileService::save(path, project, false));
   QVERIFY(!project.isModified());
   node->setName(QStringLiteral("Renamed"));
   QCOMPARE(ProjectFileService::save(path, project, false).error, ProjectFileError::OutputExists);
   QVERIFY(project.isModified());
   QVERIFY(ProjectFileService::save(path, project, true));
   QVERIFY(!project.isModified());
}

QTEST_APPLESS_MAIN(TextureProjectTest)
#include "textureproject_test.moc"
