#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/editmanager.h"
#include "support/testgenerators.h"
#include <QColor>
#include <QtTest>

class EditManagerTest : public QObject {
   Q_OBJECT

private slots:
   void groupsMoveAndSettingChanges();
   void keepsColorChangesSeparate();
   void restoresConnectionsAndRemovedNode();
   void addsRenamesAndTracksCleanState();
   void clearsAndRestoresTheWholeGraph();
};

void EditManagerTest::groupsMoveAndSettingChanges() {
   TextureProject project(false);
   const TextureGeneratorPtr generator(new RecordingGenerator(QStringLiteral("Source"), 0, 10));
   project.addGenerator(generator);
   const TextureNodePtr node = project.newNode(1, generator);
   EditManager editManager(project);
   editManager.reset();

   const QPointF start(4.0, 8.0);
   node->setPos(start);
   editManager.reset();
   node->setPos(QPointF(5.0, 9.0));
   node->setPos(QPointF(10.0, 12.0));
   const QPointF end = node->getPos();
   editManager.moveNode(node->getId(), start, end);

   QCOMPARE(editManager.stack().count(), 1);
   editManager.stack().undo();
   QCOMPARE(node->getPos(), start);
   editManager.stack().redo();
   QCOMPARE(node->getPos(), end);

   editManager.changeSetting(node->getId(), QStringLiteral("value"), 11);
   editManager.changeSetting(node->getId(), QStringLiteral("value"), 12);
   editManager.changeSetting(node->getId(), QStringLiteral("value"), 13);
   QCOMPARE(editManager.stack().count(), 2);
   QCOMPARE(node->getSettings().value(QStringLiteral("value")).toInt(), 13);
   editManager.stack().undo();
   QCOMPARE(node->getSettings().value(QStringLiteral("value")).toInt(), 10);
}

void EditManagerTest::keepsColorChangesSeparate() {
   TextureProject project(false);
   const TextureGeneratorPtr generator(new RecordingGenerator);
   const TextureNodePtr node = project.newNode(1, generator);
   TextureNodeSettings settings = node->getSettings();
   settings[QStringLiteral("color")] = QColor(10, 20, 30, 40);
   node->setSettings(settings);
   EditManager editManager(project);
   editManager.reset();

   const QColor second(50, 60, 70, 80);
   const QColor third(90, 100, 110, 120);
   editManager.changeSetting(node->getId(), QStringLiteral("color"), second, false);
   editManager.changeSetting(node->getId(), QStringLiteral("color"), third, false);
   QCOMPARE(editManager.stack().count(), 2);

   editManager.stack().undo();
   QCOMPARE(node->getSettings().value(QStringLiteral("color")).value<QColor>(), second);
   editManager.stack().undo();
   QCOMPARE(node->getSettings().value(QStringLiteral("color")).value<QColor>(),
            QColor(10, 20, 30, 40));
}

void EditManagerTest::restoresConnectionsAndRemovedNode() {
   TextureProject project(false);
   const TextureGeneratorPtr sourceGenerator(new RecordingGenerator(QStringLiteral("Source"), 0));
   const TextureGeneratorPtr filterGenerator(new RecordingGenerator(QStringLiteral("Filter"), 2));
   const TextureNodePtr upstream = project.newNode(1, sourceGenerator);
   const TextureNodePtr middle = project.newNode(2, filterGenerator);
   const TextureNodePtr receiver = project.newNode(3, filterGenerator);
   QVERIFY(middle->setSourceSlot(0, upstream->getId()));
   QVERIFY(receiver->setSourceSlot(1, middle->getId()));
   middle->setName(QStringLiteral("Middle"));
   middle->setPos(QPointF(25.0, 50.0));

   EditManager editManager(project);
   editManager.reset();
   QVERIFY(editManager.removeNode(middle->getId()));
   QVERIFY(project.getNode(2).isNull());
   QCOMPARE(receiver->getSources().value(1), 0);

   editManager.stack().undo();
   const TextureNodePtr restored = project.getNode(2);
   QVERIFY(!restored.isNull());
   QCOMPARE(restored->getName(), QStringLiteral("Middle"));
   QCOMPARE(restored->getPos(), QPointF(25.0, 50.0));
   QCOMPARE(restored->getSources().value(0), upstream->getId());
   QCOMPARE(receiver->getSources().value(1), restored->getId());

   editManager.stack().redo();
   QVERIFY(project.getNode(2).isNull());
   editManager.stack().undo();
   QVERIFY(editManager.setConnection(receiver->getId(), 1, 0));
   QCOMPARE(receiver->getSources().value(1), 0);
   editManager.stack().undo();
   QCOMPARE(receiver->getSources().value(1), 2);
}

void EditManagerTest::addsRenamesAndTracksCleanState() {
   TextureProject project(false);
   const TextureGeneratorPtr generator(new RecordingGenerator(QStringLiteral("Source"), 0));
   EditManager editManager(project);
   editManager.reset();
   QVERIFY(!project.isModified());

   const int nodeId = editManager.addNode(generator, QPointF(7.0, 9.0));
   QVERIFY(nodeId != 0);
   QVERIFY(project.isModified());
   QCOMPARE(project.getNode(nodeId)->getPos(), QPointF(7.0, 9.0));

   editManager.renameNode(nodeId, QStringLiteral("Renamed"));
   QCOMPARE(project.getNode(nodeId)->getName(), QStringLiteral("Renamed"));
   editManager.stack().undo();
   QVERIFY(project.getNode(nodeId)->getName() != QStringLiteral("Renamed"));
   editManager.stack().undo();
   QVERIFY(project.getNode(nodeId).isNull());
   QVERIFY(!project.isModified());

   editManager.stack().redo();
   QCOMPARE(project.getNode(nodeId)->getPos(), QPointF(7.0, 9.0));
   editManager.setClean();
   QVERIFY(!project.isModified());
   editManager.renameNode(nodeId, QStringLiteral("After save"));
   QVERIFY(project.isModified());
   editManager.stack().undo();
   QVERIFY(!project.isModified());
}

void EditManagerTest::clearsAndRestoresTheWholeGraph() {
   TextureProject project(false);
   const TextureGeneratorPtr sourceGenerator(new RecordingGenerator(QStringLiteral("Source"), 0));
   const TextureGeneratorPtr filterGenerator(new RecordingGenerator(QStringLiteral("Filter"), 1));
   const TextureNodePtr source = project.newNode(4, sourceGenerator);
   const TextureNodePtr receiver = project.newNode(9, filterGenerator);
   source->setName(QStringLiteral("Restored source"));
   source->setPos(QPointF(30.0, 40.0));
   QVERIFY(receiver->setSourceSlot(0, source->getId()));

   EditManager editManager(project);
   editManager.reset();
   QVERIFY(editManager.clearProject());
   QCOMPARE(project.getNumNodes(), 0);
   QCOMPARE(editManager.stack().count(), 1);
   QVERIFY(project.isModified());

   editManager.stack().undo();
   QCOMPARE(project.getNumNodes(), 2);
   QCOMPARE(project.getNode(4)->getName(), QStringLiteral("Restored source"));
   QCOMPARE(project.getNode(4)->getPos(), QPointF(30.0, 40.0));
   QCOMPARE(project.getNode(9)->getSources().value(0), 4);
   QVERIFY(!project.isModified());

   editManager.stack().redo();
   QCOMPARE(project.getNumNodes(), 0);
}

QTEST_GUILESS_MAIN(EditManagerTest)
#include "editmanager_test.moc"
