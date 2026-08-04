#include "base/settingsmanager.h"
#include "base/jstexgen.h"
#include "base/textureproject.h"
#include "gui/addnodepanel.h"
#include "gui/cubewidget.h"
#include "gui/iteminfopanel.h"
#include "gui/mainwindow.h"
#include "gui/nodesettingswidget.h"
#include "gui/previewimagepanel.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodescene.h"
#include "texgenapplication.h"
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QVBoxLayout>
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
   /// @brief Verifies named endpoint labels follow node hover and selection state.
   void showsLabelsForHighlightedOrSelectedNodes();
   /// @brief Verifies loaded list settings select their persisted combo-box values.
   void restoresPersistedComboBoxSelection();
   /// @brief Verifies custom generator origins are routed to the three dedicated palette groups.
   void groupsCustomJavaScriptGenerators();
   /// @brief Verifies preview ordering, node locking, and the optional 3D view.
   void arrangesPreviewPanelViews();

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

void UiSmokeTest::showsLabelsForHighlightedOrSelectedNodes() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl"));
   auto* view = window.findChild<QGraphicsView*>();
   QVERIFY(view != nullptr);
   auto* scene = dynamic_cast<ViewNodeScene*>(view->scene());
   QVERIFY(scene != nullptr);

   ViewNodeLine* connection = nullptr;
   for (QGraphicsItem* item : scene->items()) {
      connection = dynamic_cast<ViewNodeLine*>(item);
      if (connection != nullptr) {
         break;
      }
   }
   QVERIFY(connection != nullptr);
   QVERIFY(!connection->endpointLabelsVisible());

   const TextureNodePtr source = window.getTextureProject()->getNode(connection->getStartItemId());
   const TextureNodePtr receiver = window.getTextureProject()->getNode(connection->getEndItemId());
   QVERIFY(!source.isNull());
   QVERIFY(!receiver.isNull());
   ViewNodeItem* sourceItem = scene->getItem(source->getId());
   QVERIFY(sourceItem != nullptr);
   sourceItem->setSelected(true);
   QVERIFY(connection->endpointLabelsVisible());
   QVERIFY(connection->sourceNameLabelVisible());
   QVERIFY(!connection->receiverNameLabelVisible());
   QVERIFY(connection->zValue() > sourceItem->zValue());

   SettingsManager* settingsManager = window.getTextureProject()->getSettingsManager();
   QVERIFY(settingsManager != nullptr);
   settingsManager->setConnectionLabelSize(18);
   settingsManager->setDisplayReceiverNames(true);
   QCOMPARE(connection->getLabelFontSize(), 18);
   QVERIFY(connection->sourceNameLabelVisible());
   QVERIFY(connection->receiverNameLabelVisible());

   sourceItem->setSelected(false);
   QVERIFY(!connection->endpointLabelsVisible());

   connection->setHighlighted(true);
   QVERIFY(connection->endpointLabelsVisible());
   QCOMPARE(connection->getSourceLabelText(),
            QStringLiteral("Output (%1)").arg(receiver->getName()));
   QCOMPARE(connection->getReceiverLabelText(),
            QStringLiteral("%1 (%2)").arg(connection->getSlot(), source->getName()));

   source->setName(QStringLiteral("Renamed source"));
   QCOMPARE(connection->getReceiverLabelText(),
            QStringLiteral("%1 (Renamed source)").arg(connection->getSlot()));
   connection->setHighlighted(false);
   QVERIFY(!connection->endpointLabelsVisible());
}

void UiSmokeTest::restoresPersistedComboBoxSelection() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/rose.txl"));

   auto* infoPanel = window.findChild<ItemInfoPanel*>();
   QVERIFY(infoPanel != nullptr);
   infoPanel->setActiveNode(20);
   auto* nodeSettings =
       infoPanel->findChild<NodeSettingsWidget*>(QStringLiteral("nodeSettingsInspector"));
   QVERIFY(nodeSettings != nullptr);

   const QList<QComboBox*> comboBoxes = nodeSettings->findChildren<QComboBox*>();
   QCOMPARE(comboBoxes.size(), 4);
   QStringList selections;
   for (const QComboBox* comboBox : comboBoxes) {
      selections.append(comboBox->currentText());
   }
   QVERIFY(selections.contains(QStringLiteral("Second's alpha")));
}

void UiSmokeTest::groupsCustomJavaScriptGenerators() {
   TextureProject project(false);
   AddNodePanel panel(project);
   const auto addScript = [&project](const QString& name, const QString& type,
                                     const TextureGenerator::Origin origin) {
      const QString script = QStringLiteral(
                                 "const generator={apiVersion:1,name:'%1',type:'%2',inputs:[],"
                                 "settings:{},generate(size,settings,output){void size;void "
                                 "settings;output.data.fill(0);}};")
                                 .arg(name, type);
      auto* generator = new JsTexGen(script, QStringLiteral("test.js"), origin);
      QVERIFY(generator->isValid());
      project.addGenerator(TextureGeneratorPtr(generator));
   };
   addScript(QStringLiteral("Custom source"), QStringLiteral("generator"),
             TextureGenerator::Origin::Custom);
   addScript(QStringLiteral("Custom effect"), QStringLiteral("filter"),
             TextureGenerator::Origin::Custom);
   addScript(QStringLiteral("Custom blend"), QStringLiteral("combiner"),
             TextureGenerator::Origin::Custom);
   addScript(QStringLiteral("Bundled effect"), QStringLiteral("filter"),
             TextureGenerator::Origin::BuiltIn);

   QMap<QString, QGroupBox*> groups;
   for (QGroupBox* group : panel.findChildren<QGroupBox*>()) {
      groups.insert(group->title(), group);
   }
   QVERIFY(groups.contains(QStringLiteral("Custom Generators")));
   QVERIFY(groups.contains(QStringLiteral("Custom Filters")));
   QVERIFY(groups.contains(QStringLiteral("Custom Combiners")));
   QVERIFY(!groups.value(QStringLiteral("Custom Generators"))->isHidden());
   QVERIFY(!groups.value(QStringLiteral("Custom Filters"))->isHidden());
   QVERIFY(!groups.value(QStringLiteral("Custom Combiners"))->isHidden());
   QCOMPARE(groups.value(QStringLiteral("Custom Generators"))->findChildren<QPushButton*>().size(),
            1);
   QCOMPARE(groups.value(QStringLiteral("Custom Filters"))->findChildren<QPushButton*>().size(), 1);
   QCOMPARE(groups.value(QStringLiteral("Custom Combiners"))->findChildren<QPushButton*>().size(),
            1);
   QCOMPARE(groups.value(QStringLiteral("Filters"))->findChildren<QPushButton*>().size(), 1);

   for (QGroupBox* group : groups) {
      auto* groupLayout = qobject_cast<QGridLayout*>(group->layout());
      QVERIFY(groupLayout != nullptr);
      for (int index = 0; index < groupLayout->count(); ++index) {
         QVERIFY(groupLayout->itemAt(index)->alignment().testFlag(Qt::AlignLeft));
      }
   }
}

void UiSmokeTest::arrangesPreviewPanelViews() {
   TextureProject project(false);
   SettingsManager settingsManager;
   project.setSettingsManager(&settingsManager);
   PreviewImagePanel panel(project);

   auto* controls = panel.findChild<QWidget*>(QStringLiteral("previewControls"));
   auto* previewScrollArea = panel.findChild<QScrollArea*>(QStringLiteral("previewScrollArea"));
   auto* previewList = panel.findChild<QWidget*>(QStringLiteral("previewList"));
   auto* lockButton = panel.findChild<QPushButton*>(QStringLiteral("lockNodeButton"));
   auto* tileCount = panel.findChild<QComboBox*>(QStringLiteral("previewTileCount"));
   auto* threeDButton = panel.findChild<QPushButton*>(QStringLiteral("showThreeDButton"));
   auto* lockedPreview = panel.findChild<QGroupBox*>(QStringLiteral("lockedNodePreview"));
   auto* selectedPreview = panel.findChild<QGroupBox*>(QStringLiteral("selectedNodePreview"));
   auto* threeDPreview = panel.findChild<QGroupBox*>(QStringLiteral("threeDPreview"));
   auto* selectedImage = panel.findChild<ImageLabel*>(QStringLiteral("selectedNodeImage"));
   auto* lockedImage = panel.findChild<ImageLabel*>(QStringLiteral("lockedNodeImage"));
   auto* cube = panel.findChild<CubeWidget*>(QStringLiteral("previewCube"));
   auto* selectedRenderingOverlay =
       selectedImage->findChild<QWidget*>(QStringLiteral("renderingOverlay"));
   auto* lockedRenderingOverlay =
       lockedImage->findChild<QWidget*>(QStringLiteral("renderingOverlay"));
   QVERIFY(controls != nullptr);
   QVERIFY(previewScrollArea != nullptr);
   QVERIFY(previewList != nullptr);
   QVERIFY(lockButton != nullptr);
   QVERIFY(tileCount != nullptr);
   QVERIFY(threeDButton != nullptr);
   QVERIFY(lockedPreview != nullptr);
   QVERIFY(selectedPreview != nullptr);
   QVERIFY(threeDPreview != nullptr);
   QVERIFY(selectedImage != nullptr);
   QVERIFY(lockedImage != nullptr);
   QVERIFY(cube != nullptr);
   QVERIFY(selectedRenderingOverlay != nullptr);
   QVERIFY(lockedRenderingOverlay != nullptr);

   auto* layout = qobject_cast<QVBoxLayout*>(panel.layout());
   QVERIFY(layout != nullptr);
   QCOMPARE(layout->indexOf(controls), 0);
   QCOMPARE(layout->indexOf(previewScrollArea), 1);
   QCOMPARE(previewScrollArea->verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
   auto* previewLayout = qobject_cast<QVBoxLayout*>(previewList->layout());
   QVERIFY(previewLayout != nullptr);
   QCOMPARE(previewLayout->spacing(), 10);
   QVERIFY(previewLayout->indexOf(lockedPreview) < previewLayout->indexOf(selectedPreview));
   QVERIFY(previewLayout->indexOf(selectedPreview) < previewLayout->indexOf(threeDPreview));
   for (int index = 0; index < previewLayout->count(); ++index) {
      QVERIFY(previewLayout->itemAt(index)->spacerItem() == nullptr);
   }
   QCOMPARE(previewList->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
   QCOMPARE(selectedPreview->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
   QVERIFY(panel.maximumWidth() > 500);
   QCOMPARE(lockButton->text(), QStringLiteral("Lock node"));
   QCOMPARE(threeDButton->text(), QStringLiteral("3D"));
   QVERIFY(panel.minimumWidth() >= controls->minimumSizeHint().width());

   lockButton->setText(QStringLiteral("Unlock a node with a deliberately long label"));
   QCoreApplication::processEvents();
   const QMargins panelMargins = layout->contentsMargins();
   const int controlsMinimumWidth =
       controls->layout()->minimumSize().width() + panelMargins.left() + panelMargins.right();
   QVERIFY(panel.minimumWidth() >= controlsMinimumWidth);
   panel.resize(1, 900);
   QCOMPARE(panel.width(), panel.minimumWidth());
   QVERIFY(lockButton->geometry().right() <= controls->contentsRect().right());
   lockButton->setText(QStringLiteral("Lock node"));
   QCoreApplication::processEvents();

   panel.resize(700, 900);
   panel.show();
   QPixmap previewPixmap(100, 100);
   previewPixmap.fill(Qt::black);
   selectedImage->setPixmap(previewPixmap);
   selectedImage->show();
   QCoreApplication::processEvents();
   QVERIFY(selectedImage->width() > 256);
   const int leftGap = previewList->x();
   QCOMPARE(leftGap, 0);
   const int rightGap =
       previewScrollArea->viewport()->width() - previewList->geometry().right() - 1;
   QVERIFY(qAbs(rightGap - previewScrollArea->verticalScrollBar()->sizeHint().width()) <= 1);
   QVERIFY(lockedPreview->isHidden());
   QVERIFY(!selectedPreview->isHidden());
   QVERIFY(!threeDButton->isChecked());
   QVERIFY(threeDPreview->isHidden());

   project.newNode(1);
   project.newNode(2);
   panel.setActiveNode(1);
   const int lockedButtonWidth = lockButton->minimumSizeHint().width();
   const int lockStatePanelMinimumWidth = panel.minimumWidth();
   lockButton->click();
   QCoreApplication::processEvents();
   QVERIFY(lockButton->isChecked());
   QCOMPARE(lockButton->text(), QStringLiteral("Unlock node"));
   QCOMPARE(lockButton->minimumSizeHint().width(), lockedButtonWidth);
   QCOMPARE(panel.minimumWidth(), lockStatePanelMinimumWidth);
   QVERIFY(!lockedPreview->isHidden());
   QVERIFY(lockedPreview->geometry().bottom() < selectedPreview->geometry().top());

   selectedImage->setPixmap(previewPixmap);
   selectedImage->show();
   lockedImage->setPixmap(previewPixmap);
   lockedImage->show();
   panel.imageUpdated(1);
   QVERIFY(!selectedRenderingOverlay->isHidden());
   QVERIFY(!lockedRenderingOverlay->isHidden());
   selectedImage->setPixmap(previewPixmap);
   lockedImage->setPixmap(previewPixmap);
   QVERIFY(selectedRenderingOverlay->isHidden());
   QVERIFY(lockedRenderingOverlay->isHidden());

   panel.setActiveNode(2);
   project.removeNode(1);
   QVERIFY(!lockButton->isChecked());
   QCOMPARE(lockButton->text(), QStringLiteral("Lock node"));
   QVERIFY(lockedPreview->isHidden());

   threeDButton->click();
   QVERIFY(threeDButton->isChecked());
   QVERIFY(!threeDPreview->isHidden());

   selectedImage->setPixmap(previewPixmap);
   selectedImage->show();
   cube->setTexture(previewPixmap);
   cube->show();
   panel.imageUpdated(2);
   QVERIFY(!cube->isHidden());
   QVERIFY(selectedImage->pixmapWithRenderingOverlay().toImage() != previewPixmap.toImage());

   lockedPreview->show();
   lockedImage->setPixmap(previewPixmap);
   lockedImage->show();
   cube->show();
   panel.resize(700, 400);
   QCoreApplication::processEvents();
   QVERIFY(previewScrollArea->verticalScrollBar()->isVisible());
   QCOMPARE(previewList->x(), leftGap);
   QCOMPARE(cube->height(), cube->width());
   QCOMPARE(selectedImage->height(), selectedImage->width());
   QCOMPARE(lockedImage->height(), lockedImage->width());
   QVERIFY(threeDPreview->contentsRect().contains(cube->geometry()));
   QVERIFY(selectedPreview->contentsRect().contains(selectedImage->geometry()));
   QVERIFY(lockedPreview->contentsRect().contains(lockedImage->geometry()));
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
