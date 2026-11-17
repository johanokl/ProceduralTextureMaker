#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/editmanager.h"
#include "base/jstexgen.h"
#include "base/texturerendermanager.h"
#include "support/testgenerators.h"
#include <QColor>
#include <QtTest>

namespace {

/// @brief Builds small real JavaScript schemas for history migration tests.
TextureGeneratorPtr scriptGenerator(const QString& name, const QString& settings,
                                    const QString& inputs = "[]") {
   return TextureGeneratorPtr(new JsTexGen(
       QString("const generator={apiVersion:1,name:'%1',type:'combiner',inputs:%3,settings:%2,"
               "generate(size,settings,output){output.data.fill(88);}};")
           .arg(name, settings, inputs),
       name + ".js"));
}

}  // namespace

class EditManagerTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Saves preserve both history branches and keep worker definitions immutable.
   void keepsHistoryAcrossGeneratorChanges();
   /// @brief Deleted nodes follow renamed generators without losing save warnings.
   void restoresDeletedNodesWithCurrentGenerator();
   /// @brief Live and restored values share migration rules; new inputs survive old swaps.
   void migratesHistoricalSettingsAndConnections();
   void groupsMoveAndSettingChanges();
   void keepsColorChangesSeparate();
   void restoresConnectionsAndRemovedNode();
   void addsRenamesAndTracksCleanState();
   void clearsAndRestoresTheWholeGraph();
   void connectsAndSwapsNamedSources();
};

void EditManagerTest::keepsHistoryAcrossGeneratorChanges() {
   TextureProject project(false);
   EditManager edits(project);
   TextureGeneratorPtr old(new RecordingGenerator("Original"));
   project.addGenerator(old);
   const int id = edits.addNode(old, {});
   edits.changeSetting(id, "value", 29);
   edits.setClean();
   edits.stack().undo();
   const auto render = project.createTextureGraphSnapshot(QSize(1, 1));
   TextureGeneratorPtr replacement(new RecordingGenerator("Renamed", 0, 88));
   QVERIFY(project.replaceGenerator(old, replacement));
   QCOMPARE(edits.stack().count(), 2);
   QCOMPARE(edits.stack().index(), 1);
   QVERIFY(edits.stack().canRedo());
   QCOMPARE(render.nodes.front().generator, old);
   edits.stack().redo();
   QCOMPARE(project.getNode(id)->getSettings().value("value").toInt(), 29);
   QVERIFY(project.isModified());
   edits.changeSetting(id, "value", 30);
   QCOMPARE(edits.stack().count(), 3);  // Do not merge across a source save.
   edits.stack().setIndex(0);
   QVERIFY(project.getNode(id).isNull());
   edits.stack().setIndex(3);
   QCOMPARE(project.getNode(id)->getGenerator(), replacement);
   QCOMPARE(project.getNode(id)->getSettings().value("value").toInt(), 30);
   QVERIFY(project.isModified());
}

void EditManagerTest::restoresDeletedNodesWithCurrentGenerator() {
   TextureProject project(false);
   EditManager edits(project);
   TextureGeneratorPtr old(new RecordingGenerator("Original"));
   project.addGenerator(old);
   const int id = edits.addNode(old, {});
   QVERIFY(edits.removeNode(id));
   edits.setClean();
   TextureGeneratorPtr replacement(new RecordingGenerator("Renamed"));
   QVERIFY(project.replaceGenerator(old, replacement));
   QCOMPARE(edits.stack().count(), 2);
   QVERIFY(project.isModified());
   edits.stack().undo();
   QCOMPARE(project.getNode(id)->getGenerator(), replacement);
   QVERIFY(project.isModified());
   edits.stack().redo();
   QVERIFY(project.isModified());  // Even the old clean index is no longer a saved state.
   project.removeGenerator(replacement);
   QSignalSpy warnings(&project, &TextureProject::historyReplayAdjusted);
   edits.stack().undo();
   QVERIFY(project.getNode(id).isNull());
   QVERIFY(!warnings.isEmpty());
   project.addGenerator(replacement);
   edits.stack().redo();
   edits.stack().undo();
   QCOMPARE(project.getNode(id)->getGenerator(), replacement);
}

void EditManagerTest::migratesHistoricalSettingsAndConnections() {
   TextureProject project(false);
   EditManager edits(project);
   const QString schema =
       "[{id:'value',type:'integer',default:17,max:100},"
       "{id:'mode',type:'choice',default:'A',values:['A','B']},"
       "{id:'old',type:'boolean',default:true},{id:'text',type:'string',default:'hello'}]";
   const QString changedSchema =
       "[{id:'value',type:'integer',default:3,min:0,max:40},"
       "{id:'mode',type:'choice',default:'C',values:['C']},"
       "{id:'text',type:'boolean',default:false},{id:'new',type:'boolean',default:true}]";
   const auto old = scriptGenerator("Original", schema, "['A','B']");
   const auto replacement = scriptGenerator("Renamed", changedSchema, "['A','C']");
   QVERIFY(static_cast<JsTexGen*>(old.data())->isValid());
   QVERIFY(static_cast<JsTexGen*>(replacement.data())->isValid());
   project.addGenerator(old);
   const TextureGeneratorPtr source(new RecordingGenerator);
   project.newNode(1, source);
   project.newNode(2, source);
   const auto node = project.newNode(3, old);
   QVERIFY(node->setSourceSlot("A", 1));
   QVERIFY(node->setSourceSlot("B", 2));
   edits.changeSetting(3, "value", 90);
   edits.changeSetting(3, "mode", "B");
   edits.changeSetting(3, "text", "changed");
   edits.changeSetting(3, "old", false);
   edits.swapSources(3);
   const auto live = project.newNode(4, old);
   live->setSettings(node->getSettings());
   QVERIFY(edits.removeNode(3));
   QVERIFY(project.replaceGenerator(old, replacement));
   QSignalSpy warnings(&project, &TextureProject::historyReplayAdjusted);
   edits.stack().undo();
   const auto restored = project.getNode(3);
   QCOMPARE(restored->getSettings(), live->getSettings());
   QCOMPARE(restored->getSettings().value("value").toInt(), 40);
   QCOMPARE(restored->getSettings().value("mode").toString(), QString("C"));
   QCOMPARE(restored->getSettings().value("text"), QVariant(false));
   QCOMPARE(restored->getSettings().value("new"), QVariant(true));
   QVERIFY(!restored->getSettings().contains("old"));
   QCOMPARE(restored->getSources().value("A"), 2);
   QVERIFY(!restored->getSources().contains("B"));
   QVERIFY(restored->setSourceSlot("C", 2));
   edits.stack().undo();  // Old swap must leave the new C input alone.
   QCOMPARE(restored->getSources().value("A"), 1);
   QCOMPARE(restored->getSources().value("C"), 2);
   edits.stack().setIndex(0);
   QVERIFY(!restored->getSettings().contains("old"));
   QCOMPARE(restored->getSettings().value("text"), QVariant(false));
   QCOMPARE(restored->getSettings().value("value").toInt(), 17);
   QVERIFY(!warnings.isEmpty());
   const auto widened =
       scriptGenerator("Renamed", QString(changedSchema).replace("max:40", "max:100"), "['A','C']");
   QVERIFY(project.replaceGenerator(replacement, widened));
   edits.stack().redo();
   QCOMPARE(restored->getSettings().value("value").toInt(), 90);
   const auto other = scriptGenerator("Another", "[{id:'value',type:'integer',default:3}]");
   QVERIFY(restored->setGenerator(other));
   edits.stack().undo();
   QCOMPARE(restored->getSettings().value("value").toInt(), 17);
   edits.stack().redo();
   QCOMPARE(restored->getSettings().value("value").toInt(), 90);
}

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
   const auto generator =
       scriptGenerator("Colour", "[{id:'color',type:'color',default:{r:0,g:0,b:0,a:255}}]");
   QVERIFY(static_cast<JsTexGen*>(generator.data())->isValid());
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
   QVERIFY(middle->setSourceSlot(QStringLiteral("Input 1"), upstream->getId()));
   QVERIFY(receiver->setSourceSlot(QStringLiteral("Input 2"), middle->getId()));
   middle->setName(QStringLiteral("Middle"));
   middle->setPos(QPointF(25.0, 50.0));

   EditManager editManager(project);
   editManager.reset();
   QVERIFY(editManager.removeNode(middle->getId()));
   QVERIFY(project.getNode(2).isNull());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), 0);

   editManager.stack().undo();
   const TextureNodePtr restored = project.getNode(2);
   QVERIFY(!restored.isNull());
   QCOMPARE(restored->getName(), QStringLiteral("Middle"));
   QCOMPARE(restored->getPos(), QPointF(25.0, 50.0));
   QCOMPARE(restored->getSources().value(QStringLiteral("Input 1")), upstream->getId());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), restored->getId());

   editManager.stack().redo();
   QVERIFY(project.getNode(2).isNull());
   editManager.stack().undo();
   QVERIFY(editManager.setConnection(receiver->getId(), QStringLiteral("Input 2"), 0));
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), 0);
   editManager.stack().undo();
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), 2);
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
   QVERIFY(receiver->setSourceSlot(QStringLiteral("Image"), source->getId()));

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
   QCOMPARE(project.getNode(9)->getSources().value(QStringLiteral("Image")), 4);
   QVERIFY(!project.isModified());

   editManager.stack().redo();
   QCOMPARE(project.getNumNodes(), 0);
}

void EditManagerTest::connectsAndSwapsNamedSources() {
   TextureProject project(false);
   const TextureGeneratorPtr sourceGenerator(new RecordingGenerator(QStringLiteral("Source"), 0));
   const TextureGeneratorPtr receiverGenerator(
       new RecordingGenerator(QStringLiteral("Receiver"), 2));
   const TextureNodePtr first = project.newNode(1, sourceGenerator);
   const TextureNodePtr second = project.newNode(2, sourceGenerator);
   const TextureNodePtr receiver = project.newNode(3, receiverGenerator);
   EditManager editManager(project);
   editManager.reset();
   QSignalSpy connected(&project, &TextureProject::nodesConnected);

   QVERIFY(editManager.setConnectionToFirstAvailable(receiver->getId(), first->getId()));
   QVERIFY(editManager.setConnectionToFirstAvailable(receiver->getId(), second->getId()));
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 1")), first->getId());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), second->getId());
   QCOMPARE(connected.size(), 2);
   QCOMPARE(connected.at(0).at(2).toString(), QStringLiteral("Input 1"));
   QCOMPARE(connected.at(1).at(2).toString(), QStringLiteral("Input 2"));
   QVERIFY(
       !editManager.setConnection(receiver->getId(), QStringLiteral("Missing"), first->getId()));

   editManager.swapSources(receiver->getId());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 1")), second->getId());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), first->getId());
   editManager.stack().undo();
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 1")), first->getId());
   QCOMPARE(receiver->getSources().value(QStringLiteral("Input 2")), second->getId());
}

QTEST_GUILESS_MAIN(EditManagerTest)
#include "editmanager_test.moc"
