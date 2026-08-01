#include "base/textureproject.h"
#include "gui/mainwindow.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodescene.h"
#include "texgenapplication.h"
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

/// @brief Exposes protected drag handlers for focused scene lifetime tests.
class TestViewNodeScene : public ViewNodeScene {
public:
   /// @brief Creates a test scene for a main window.
   /// @param mainWindow Window that supplies the texture project.
   explicit TestViewNodeScene(MainWindow& mainWindow) : ViewNodeScene(mainWindow) {}
   using ViewNodeScene::dragEnterEvent;
   using ViewNodeScene::dragLeaveEvent;
   using ViewNodeScene::dropEvent;
};

/// @brief Performs a minimal offscreen smoke test of the graphical application.
class UiSmokeTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Isolates application settings and initializes the test environment.
   void initTestCase();
   /// @brief Verifies that a tracked project loads into a scene-backed window.
   void loadsProjectIntoSceneBackedWindow();
   /// @brief Verifies ownership of temporary connection and drag items.
   void managesTemporarySceneItems();
   /// @brief Verifies that node removal clears scene item observers.
   void removesConnectedNodeItems();

private:
   /// @brief Owns the isolated settings directory for the duration of the test.
   std::unique_ptr<QTemporaryDir> settingsDirectory;
};

void UiSmokeTest::initTestCase() {
   QStandardPaths::setTestModeEnabled(true);
   QCoreApplication::setOrganizationName(QStringLiteral("PTM tests"));
   QCoreApplication::setApplicationName(QStringLiteral("UI smoke"));
   settingsDirectory = std::make_unique<QTemporaryDir>();
   QVERIFY(settingsDirectory->isValid());
   QSettings::setDefaultFormat(QSettings::IniFormat);
   QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory->path());
   QSettings settings;
   settings.setValue(QStringLiteral("showhelpstartup"), false);
}

void UiSmokeTest::loadsProjectIntoSceneBackedWindow() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl"));
   QCOMPARE(window.getTextureProject()->getNumNodes(), 11);
   QCOMPARE(window.getTextureProject()->getSinkNodeIds(), QList<int>({13}));
   for (int i = 0; i < 5; ++i) {
      window.reloadSceneView();
   }
   QCOMPARE(window.getTextureProject()->getNumNodes(), 11);
}

void UiSmokeTest::managesTemporarySceneItems() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl"));
   TestViewNodeScene scene(window);
   scene.addNode(window.getTextureProject()->getNode(13));
   const qsizetype persistentItems = scene.items().size();

   scene.startLineDrawing(13);
   QCOMPARE(scene.items().size(), persistentItems + 1);
   scene.endLineDrawing(-1);
   QCOMPARE(scene.items().size(), persistentItems);

   QGraphicsSceneDragDropEvent dragEnter(QEvent::GraphicsSceneDragEnter);
   dragEnter.setScenePos(QPointF(20, 20));
   scene.dragEnterEvent(&dragEnter);
   QCOMPARE(scene.items().size(), persistentItems + 1);
   scene.settingsUpdated();
   QCOMPARE(scene.items().size(), persistentItems + 1);

   QGraphicsSceneDragDropEvent dragLeave(QEvent::GraphicsSceneDragLeave);
   scene.dragLeaveEvent(&dragLeave);
   QCOMPARE(scene.items().size(), persistentItems);

   QMimeData mimeData;
   mimeData.setText(QStringLiteral("missing-generator"));
   QGraphicsSceneDragDropEvent drop(QEvent::GraphicsSceneDrop);
   drop.setMimeData(&mimeData);
   drop.setScenePos(QPointF(20, 20));
   scene.dragEnterEvent(&dragEnter);
   scene.dropEvent(&drop);
   QCOMPARE(scene.items().size(), persistentItems);
}

void UiSmokeTest::removesConnectedNodeItems() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl"));
   auto* view = window.findChild<QGraphicsView*>();
   QVERIFY(view != nullptr);
   auto* scene = dynamic_cast<ViewNodeScene*>(view->scene());
   QVERIFY(scene != nullptr);

   const int nodeId = 13;
   QVERIFY(scene->getItem(nodeId) != nullptr);
   window.getTextureProject()->removeNode(nodeId);
   QVERIFY(scene->getItem(nodeId) == nullptr);
   for (QGraphicsItem* item : scene->items()) {
      auto* line = dynamic_cast<ViewNodeLine*>(item);
      QVERIFY(line == nullptr ||
              (line->getStartItemId() != nodeId && line->getEndItemId() != nodeId));
   }
}

/// @brief Runs the UI smoke test with the production application subclass.
/// @param argc Number of command-line arguments.
/// @param argv Command-line argument values.
/// @return Qt Test process exit code.
int main(int argc, char** argv) {
   TexGenApplication application(argc, argv);
   UiSmokeTest test;
   return QTest::qExec(&test, argc, argv);
}

#include "ui_smoke_test.moc"
