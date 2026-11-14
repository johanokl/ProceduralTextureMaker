#include "base/settingsmanager.h"
#include "base/editmanager.h"
#include "base/jstexgen.h"
#include "base/textureproject.h"
#include "gui/addnodepanel.h"
#include "gui/applicationtheme.h"
#include "gui/cubewidget.h"
#include "gui/generatorinfowidget.h"
#include "gui/iteminfopanel.h"
#include "gui/mainwindow.h"
#include "gui/nodesettingswidget.h"
#include "gui/qdoubleslider.h"
#include "gui/previewimagepanel.h"
#include "gui/texturebackground.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodescene.h"
#include "sceneview/viewnodeview.h"
#include "texgenapplication.h"
#include <QGraphicsSceneDragDropEvent>
#include <QEvent>
#include <QHelpEvent>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QMimeData>
#include <QPushButton>
#include <QProgressDialog>
#include <QTimer>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include <QScopeGuard>
#include <QStyle>
#include <QStyleFactory>
#include <QToolBar>
#include <QAction>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QVBoxLayout>
#include <QWheelEvent>
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

/// @brief Exposes wheel handling for focused scene-view zoom tests.
class TestViewNodeView : public ViewNodeView {
public:
   using ViewNodeView::wheelEvent;
};

/// @brief Records widgets shown as standalone windows, including brief startup flashes.
class StartupWindowObserver : public QObject {
public:
   /// @brief Class names captured when top-level widgets are shown.
   QStringList shownWindows;

protected:
   /// @brief Records show events without changing normal event handling.
   bool eventFilter(QObject* watched, QEvent* event) override {
      if (event->type() == QEvent::Show) {
         const auto* widget = qobject_cast<QWidget*>(watched);
         if (widget != nullptr && widget->isWindow()) {
            shownWindows.append(QString::fromLatin1(widget->metaObject()->className()));
         }
      }
      return false;
   }
};

/// @brief Performs a minimal offscreen smoke test of the graphical application.
class UiSmokeTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Checks empty and duplicate IDs produce distinct warning messages.
   void explainsInvalidNodeIds();
   /// @brief Checks tooltip pointer tracking and rapid node re-entry.
   void keepsGraphTooltipVisibleAfterReentry();
   /// @brief Verifies child widgets are embedded before any startup window is shown.
   void showsOnlyMainWindowAtStartup();
   /// @brief Supplies filters with matching channel checkboxes.
   void editsChannelFiltersWithCheckboxes_data();
   /// @brief Checks boolean channel controls with RGB checked and alpha unchecked.
   void editsChannelFiltersWithCheckboxes();
   /// @brief Checks that stuck exports can be cancelled while the window remains responsive.
   void cancelsImageExportAndClosesSafely();
   /// @brief Verifies failures, blocked inputs, stopped work, and recovery in graph and previews.
   void displaysRenderFailuresAndStoppedWork();
   /// @brief Verifies the theme draws disabled menu text differently from enabled text.
   void distinguishesDisabledMenuItems();
   /// @brief Verifies Stop and Restart are menu-only actions with matching enabled states.
   void stopsAndRestartsRenderingFromMenu();
   /// @brief Isolates application settings and initializes the test environment.
   void initTestCase();
   /// @brief Verifies transparent texture backgrounds are composited consistently.
   void compositesTextureBackground();
   /// @brief Verifies that a tracked project loads into a scene-backed window.
   void loadsProjectIntoSceneBackedWindow();
   /// @brief Verifies opening a project reuses only clean, empty windows.
   void opensProjectWithoutDiscardingChanges();
   /// @brief Verifies ownership of temporary connection and drag items.
   void managesTemporarySceneItems();
   /// @brief Verifies that node removal clears scene item observers.
   void removesConnectedNodeItems();
   /// @brief Verifies named endpoint labels follow node hover and selection state.
   void showsLabelsForHighlightedOrSelectedNodes();
   /// @brief Verifies loaded list settings select their persisted combo-box values.
   void restoresPersistedComboBoxSelection();
   /// @brief Verifies unmodified mouse-wheel scrolling zooms the scene view.
   void zoomsSceneWithMouseWheel();
   /// @brief Verifies custom generator origins are routed to the three dedicated palette groups.
   void groupsCustomJavaScriptGenerators();
   /// @brief Verifies clicking a palette button displays its generator metadata.
   void selectsGeneratorFromAddNodePanel();
   /// @brief Verifies generator and graph selections clear one another.
   void keepsGeneratorAndGraphSelectionsExclusive();
   /// @brief Verifies preview ordering, node locking, and the optional 3D view.
   void arrangesPreviewPanelViews();

private:
   /// @brief Owns the isolated settings directory for the duration of the test.
   std::unique_ptr<QTemporaryDir> settingsDirectory;
};

void UiSmokeTest::explainsInvalidNodeIds() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   MainWindow window(application);
   auto* project = window.getTextureProject();
   const auto node = project->newNode(1, project->getGenerator("Fill"));
   const auto other = project->newNode(2, project->getGenerator("Fill"));
   QVERIFY(other->setIdentifier("taken"));
   auto* panel = window.findChild<ItemInfoPanel*>();
   panel->setActiveNode(node->getId());
   auto* editor = panel->findChild<QLineEdit*>("nodeIdentifier");
   QVERIFY(editor != nullptr);
   for (const QString& value : {QString("   "), QString(" taken ")}) {
      QString message;
      QTimer::singleShot(0, &window, [&message] {
         if (auto* dialog = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            message = dialog->text();
            dialog->accept();
         }
      });
      editor->setText(value);
      QVERIFY(QMetaObject::invokeMethod(editor, "editingFinished"));
      QCOMPARE(message, value.trimmed().isEmpty()
                            ? QString("The node ID cannot be empty.")
                            : QString("Another node already uses the ID \"taken\"."));
      QCOMPARE(editor->text(), node->getIdentifier());
   }
}

void UiSmokeTest::keepsGraphTooltipVisibleAfterReentry() {
   QGraphicsScene scene;
   scene.setSceneRect(0, 0, 300, 300);
   auto* node = scene.addRect(40, 40, 100, 100);
   node->setToolTip("Test node (Fill)");
   ViewNodeView view;
   view.resize(320, 320);
   view.setNodeScene(&scene);
   view.show();
   QCoreApplication::processEvents();
   auto* tooltip = view.findChild<QLabel*>(QStringLiteral("graphTooltip"));
   QVERIFY(tooltip != nullptr);
   QVERIFY(tooltip->windowFlags().testFlag(Qt::WindowTransparentForInput));
   const QPoint inside = view.mapFromScene(80, 80);
   const QPoint outside = view.mapFromScene(220, 220);
   QMouseEvent first(QEvent::MouseMove, inside, view.viewport()->mapToGlobal(inside), Qt::NoButton,
                     Qt::NoButton, Qt::NoModifier);
   QCoreApplication::sendEvent(view.viewport(), &first);
   QHelpEvent nativeRequest(QEvent::ToolTip, inside, view.viewport()->mapToGlobal(inside));
   QCoreApplication::sendEvent(view.viewport(), &nativeRequest);
   QTRY_VERIFY(tooltip->isVisible());
   QCOMPARE(tooltip->text(), node->toolTip());
   const QPoint firstPosition = tooltip->pos();
   const QPoint movedInside = inside + QPoint(10, 10);
   QMouseEvent follow(QEvent::MouseMove, movedInside, view.viewport()->mapToGlobal(movedInside),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);
   QCoreApplication::sendEvent(view.viewport(), &follow);
   QCOMPARE(tooltip->pos(), firstPosition + QPoint(10, 10));
   QMouseEvent leaveNode(QEvent::MouseMove, outside, view.viewport()->mapToGlobal(outside),
                         Qt::NoButton, Qt::NoButton, Qt::NoModifier);
   QCoreApplication::sendEvent(view.viewport(), &leaveNode);
   QVERIFY(!tooltip->isVisible());
   QMouseEvent reentry(QEvent::MouseMove, inside, view.viewport()->mapToGlobal(inside),
                       Qt::NoButton, Qt::NoButton, Qt::NoModifier);
   QCoreApplication::sendEvent(view.viewport(), &reentry);
   QVERIFY(!tooltip->isVisible());
   QTRY_VERIFY(tooltip->isVisible());
   QTest::qWait(350);
   QVERIFY(tooltip->isVisible());
   view.setNodeScene(nullptr);
   QVERIFY(!tooltip->isVisible());
}

void UiSmokeTest::showsOnlyMainWindowAtStartup() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   StartupWindowObserver observer;
   application->installEventFilter(&observer);
   const auto removeFilter = qScopeGuard([&] { application->removeEventFilter(&observer); });
   MainWindow window(application);
   QVERIFY2(observer.shownWindows.isEmpty(), qPrintable(observer.shownWindows.join(", ")));
   window.show();
   QCoreApplication::processEvents();
   QCOMPARE(observer.shownWindows, QStringList({QStringLiteral("MainWindow")}));
   auto* view = window.findChild<QGraphicsView*>();
   QVERIFY(view != nullptr);
   QVERIFY(view->isVisible());
}

void UiSmokeTest::editsChannelFiltersWithCheckboxes_data() {
   QTest::addColumn<QString>("generatorName");
   QTest::addColumn<QString>("settingPrefix");
   QTest::newRow("invert") << QStringLiteral("Invert") << QStringLiteral("invert");
   QTest::newRow("adjust") << QStringLiteral("Adjust") << QStringLiteral("channel");
}

void UiSmokeTest::editsChannelFiltersWithCheckboxes() {
   QFETCH(QString, generatorName);
   QFETCH(QString, settingPrefix);
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   MainWindow window(application);
   auto* project = window.getTextureProject();
   const auto node = project->newNode(1, project->getGenerator(generatorName));
   auto* infoPanel = window.findChild<ItemInfoPanel*>();
   QVERIFY(infoPanel != nullptr);
   infoPanel->setActiveNode(1);
   auto* inspector =
       infoPanel->findChild<NodeSettingsWidget*>(QStringLiteral("nodeSettingsInspector"));
   QVERIFY(inspector != nullptr);
   auto* idEditor = inspector->findChild<QLineEdit*>(QStringLiteral("nodeIdentifier"));
   QVERIFY(idEditor != nullptr);
   QCOMPARE(idEditor->text(), node->getIdentifier());
   idEditor->setText(" filter ");
   QVERIFY(QMetaObject::invokeMethod(idEditor, "editingFinished"));
   QCOMPARE(project->getNode("filter"), node);
   QCOMPARE(idEditor->text(), QString("filter"));
   window.getEditManager().stack().undo();
   QCOMPARE(idEditor->text(), node->getIdentifier());
   QVERIFY(!project->getNode("filter"));
   const auto comboBoxes = inspector->findChildren<QComboBox*>();
   QCOMPARE(comboBoxes.size(), generatorName == QStringLiteral("Invert") ? 0 : 1);
   if (!comboBoxes.isEmpty()) {
      QCOMPARE(comboBoxes.first()->currentText(), QStringLiteral("Multiply"));
      QCOMPARE(comboBoxes.first()->count(), 3);
      QCOMPARE(comboBoxes.first()->itemText(2), QStringLiteral("Subtract"));
      auto* amount = inspector->findChild<QDoubleSpinBox*>();
      QVERIFY(amount != nullptr);
      QCOMPARE(amount->minimum(), 0.0);
      QCOMPARE(amount->maximum(), 255.0);
      auto* slider = inspector->findChild<QDoubleSlider*>();
      QVERIFY(slider != nullptr);
      QCOMPARE(slider->minimum(), 0);
      // QDoubleSlider stores values in hundredths.
      QCOMPARE(slider->maximum(), 25500);
      comboBoxes.first()->setCurrentText(QStringLiteral("Subtract"));
      amount->setValue(30.0);
   }
   const auto checkboxes = inspector->findChildren<QCheckBox*>();
   QCOMPARE(checkboxes.size(), 4);
   for (int index = 0; index < checkboxes.size(); ++index) {
      QCOMPARE(checkboxes[index]->isChecked(), index < 3);
   }
   checkboxes[0]->setChecked(false);
   checkboxes[3]->setChecked(true);
   const auto settings = node->getSettings();
   QCOMPARE(settings.value(settingPrefix + QStringLiteral("Red")).typeId(), QMetaType::Bool);
   QCOMPARE(settings.value(settingPrefix + QStringLiteral("Red")).toBool(), false);
   QCOMPARE(settings.value(settingPrefix + QStringLiteral("Green")).toBool(), true);
   QCOMPARE(settings.value(settingPrefix + QStringLiteral("Blue")).toBool(), true);
   QCOMPARE(settings.value(settingPrefix + QStringLiteral("Alpha")).toBool(), true);
   if (generatorName == QStringLiteral("Adjust")) {
      QCOMPARE(settings.value(QStringLiteral("mode")).toString(), QStringLiteral("Subtract"));
      QCOMPARE(settings.value(QStringLiteral("level")).toDouble(), 30.0);
   }
}

void UiSmokeTest::cancelsImageExportAndClosesSafely() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   auto window = std::make_unique<MainWindow>(application);
   auto* project = window->getTextureProject();
   project->stopRendering();
   const TextureGeneratorPtr generator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'Stuck export',type:'generator',inputs:[],"
       "generate(){while(true){}}};")));
   project->newNode(1, generator);
   QAction* save = nullptr;
   for (auto* action : window->findChildren<QAction*>()) {
      if (action->text() == QStringLiteral("Save selected image")) {
         save = action;
      }
   }
   QVERIFY(save != nullptr);
   QTemporaryDir directory;
   const QString path = directory.filePath(QStringLiteral("image.png"));
   int ticks = 0;
   QTimer heartbeat;
   connect(&heartbeat, &QTimer::timeout, this, [&] { ++ticks; });
   heartbeat.start(10);
   window->exportImage(1, path);
   QVERIFY(!save->isEnabled());
   auto* progress = window->findChild<QProgressDialog*>();
   QVERIFY(progress != nullptr);
   auto* cancel = progress->findChild<QPushButton*>();
   QVERIFY(cancel != nullptr);
   QTRY_VERIFY(ticks >= 3);
   cancel->click();
   QTRY_VERIFY(save->isEnabled());
   QVERIFY(!QFile::exists(path));
   QVERIFY(project->isRenderingStopped());

   window->exportImage(1, path);
   QTRY_VERIFY(ticks >= 6);
   window.reset();
   QCoreApplication::processEvents();
   QVERIFY(!QFile::exists(path));
}

void UiSmokeTest::displaysRenderFailuresAndStoppedWork() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   MainWindow window(application);
   auto* project = window.getTextureProject();
   const TextureGeneratorPtr generator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'RenderStatus',type:'generator',inputs:[],"
       "settings:[{id:'mode',type:'integer',default:0,min:0,max:2}],"
       "generate(size,settings,output){if(settings.mode===1)throw Error('test failure');"
       "while(settings.mode===2){}output.data.fill(255);}};")));
   const TextureGeneratorPtr filter(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'StatusInput',type:'filter',inputs:['Source'],"
       "generate(size,settings,output,inputs){output.data.set(inputs.Source.data);}};")));
   project->addGenerator(generator);
   project->addGenerator(filter);
   const auto root = project->newNode(1, generator);
   const auto child = project->newNode(2, filter);
   QVERIFY(child->setSourceSlot(QStringLiteral("Source"), 1));
   QTRY_VERIFY(!child->cachedImage(project->getThumbnailSize()).isNull());

   auto* view = window.findChild<QGraphicsView*>();
   QVERIFY(view != nullptr);
   auto* scene = dynamic_cast<ViewNodeScene*>(view->scene());
   QVERIFY(scene != nullptr);
   auto* panel = window.findChild<PreviewImagePanel*>();
   auto* selected = panel->findChild<ImageLabel*>(QStringLiteral("selectedNodeImage"));
   auto* locked = panel->findChild<ImageLabel*>(QStringLiteral("lockedNodeImage"));
   window.show();
   panel->show();
   panel->setActiveNode(1);
   auto* lock = panel->findChild<QPushButton*>(QStringLiteral("lockNodeButton"));
   lock->click();
   QCOMPARE(selected->renderState(), TextureRenderState::Ready);

   const auto independent = project->newNode(3, generator);
   independent->setSettings({{QStringLiteral("mode"), 2}});
   root->setSettings({{QStringLiteral("mode"), 1}});
   QTRY_COMPARE(project->thumbnailRenderStatus(1).state, TextureRenderState::Failed);
   QCOMPARE(scene->getItem(1)->renderState(), TextureRenderState::Failed);
   QCOMPARE(scene->getItem(2)->renderState(), TextureRenderState::Blocked);
   QCOMPARE(scene->getItem(3)->renderState(), TextureRenderState::Cancelled);
   QVERIFY(scene->getItem(2)->toolTip().contains(QStringLiteral("test failure")));
   QCOMPARE(selected->renderState(), TextureRenderState::Failed);
   QCOMPARE(locked->renderState(), TextureRenderState::Failed);
   QVERIFY(selected->toolTip().contains(QStringLiteral("test failure")));
   const auto* image = selected->findChild<QLabel*>(QStringLiteral("previewImage"));
   QVERIFY(!image->pixmap().isNull());
   QCOMPARE(image->pixmap().toImage().pixelColor(0, 0), QColor(Qt::white));
   const auto failedOverlay = selected->pixmapWithStatusOverlay().toImage();
   const QColor failureTint = failedOverlay.pixelColor(0, 0);
   QVERIFY(failureTint.red() > failureTint.green());
   panel->hide();
   panel->show();
   QCOMPARE(selected->renderState(), TextureRenderState::Failed);
   panel->setActiveNode(2);
   QCOMPARE(selected->renderState(), TextureRenderState::Blocked);

   project->removeNode(3);
   root->setSettings({{QStringLiteral("mode"), 2}});
   QTRY_VERIFY(project->canStopRendering());
   project->stopRendering();
   QCOMPARE(scene->getItem(1)->renderState(), TextureRenderState::Stopped);
   QCOMPARE(scene->getItem(2)->renderState(), TextureRenderState::Stopped);
   QCOMPARE(selected->renderState(), TextureRenderState::Stopped);
   QCOMPARE(locked->renderState(), TextureRenderState::Stopped);
   const auto stoppedOverlay = locked->pixmapWithStatusOverlay().toImage();
   QVERIFY(stoppedOverlay != failedOverlay);
   locked->setRenderStatus({TextureRenderState::Rendering, {}});
   QVERIFY(locked->pixmapWithStatusOverlay().toImage() == stoppedOverlay);
   locked->setRenderStatus(project->thumbnailRenderStatus(1));

   root->setSettings({{QStringLiteral("mode"), 0}});
   project->restartRendering();
   QTRY_COMPARE(project->thumbnailRenderStatus(2).state, TextureRenderState::Ready);
   QCOMPARE(selected->renderState(), TextureRenderState::Ready);
   QCOMPARE(locked->renderState(), TextureRenderState::Ready);
   QVERIFY(locked->toolTip().isEmpty());
   QVERIFY(!scene->getItem(1)->toolTip().contains(QStringLiteral("test failure")));
}

void UiSmokeTest::distinguishesDisabledMenuItems() {
   auto* application = qobject_cast<QApplication*>(QCoreApplication::instance());
   const QString originalStyle = application->style()->objectName();
   const QString originalSheet = application->styleSheet();
   const QPalette originalPalette = application->palette();
   const QFont originalFont = application->font();
   const auto restoreTheme = qScopeGuard([=] {
      application->setStyleSheet(originalSheet);
      application->setStyle(QStyleFactory::create(originalStyle));
      application->setPalette(originalPalette);
      application->setFont(originalFont);
   });
   ApplicationTheme::apply(*application);
   QMenu menu;
   auto* enabled = menu.addAction(QStringLiteral("Rendering"));
   auto* disabled = menu.addAction(QStringLiteral("Rendering"));
   disabled->setEnabled(false);
   menu.show();
   menu.setActiveAction(nullptr);
   QCoreApplication::processEvents();
   const QImage enabledImage = menu.grab(menu.actionGeometry(enabled)).toImage();
   const QImage disabledImage = menu.grab(menu.actionGeometry(disabled)).toImage();
   QVERIFY(!enabledImage.isNull());
   QCOMPARE(enabledImage.size(), disabledImage.size());
   QVERIFY(enabledImage != disabledImage);
}

void UiSmokeTest::stopsAndRestartsRenderingFromMenu() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   MainWindow window(application);
   QMenu* viewMenu = nullptr;
   for (auto* action : window.menuBar()->actions()) {
      QVERIFY(action->text() != QStringLiteral("&Render"));
      if (action->text() == QStringLiteral("&View")) {
         viewMenu = action->menu();
      }
   }
   QVERIFY(viewMenu != nullptr);
   QAction* stop = nullptr;
   QAction* restart = nullptr;
   for (auto* action : viewMenu->actions()) {
      if (action->text() == QStringLiteral("Stop rendering")) {
         stop = action;
      } else if (action->text() == QStringLiteral("Restart rendering")) {
         restart = action;
      }
   }
   QVERIFY(stop != nullptr);
   QVERIFY(restart != nullptr);

   for (auto* toolbar : window.findChildren<QToolBar*>()) {
      QVERIFY(!toolbar->actions().contains(stop));
      QVERIFY(!toolbar->actions().contains(restart));
   }
   QVERIFY(stop->isVisible());
   QVERIFY(restart->isVisible());
   QVERIFY(!stop->isEnabled());
   QVERIFY(!restart->isEnabled());
   auto* project = window.getTextureProject();
   stop->trigger();
   QVERIFY(!project->isRenderingStopped());

   const TextureGeneratorPtr generator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'MenuState',type:'generator',inputs:[],"
       "settings:[{id:'stuck',type:'boolean',default:true}],"
       "generate(size,settings,output){while(settings.stuck){}output.data.fill(255);}};")));
   project->addGenerator(generator);
   const auto node = project->newNode(1, generator);
   QTRY_VERIFY(stop->isEnabled());
   QVERIFY(!restart->isEnabled());
   stop->trigger();
   QVERIFY(project->isRenderingStopped());
   QVERIFY(!stop->isEnabled());
   QVERIFY(restart->isEnabled());

   restart->trigger();
   QVERIFY(!project->isRenderingStopped());
   QTRY_VERIFY(stop->isEnabled());
   QVERIFY(!restart->isEnabled());
   stop->trigger();
   node->setSettings({{QStringLiteral("stuck"), false}});
   restart->trigger();
   QTRY_VERIFY(!node->cachedImage(project->getThumbnailSize()).isNull());
   QVERIFY(!stop->isEnabled());
   QVERIFY(!restart->isEnabled());

   node->setSettings({{QStringLiteral("stuck"), true}});
   QTRY_VERIFY(stop->isEnabled());
   stop->trigger();
   QVERIFY(restart->isEnabled());
   project->removeNode(node->getId());
   QVERIFY(!stop->isEnabled());
   QVERIFY(!restart->isEnabled());
}

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

void UiSmokeTest::zoomsSceneWithMouseWheel() {
   TestViewNodeView view;
   view.resize(200, 200);
   const QPointF wheelPosition(100, 100);
   QWheelEvent zoomInEvent(wheelPosition, wheelPosition, QPoint(), QPoint(0, 120), Qt::NoButton,
                           Qt::NoModifier, Qt::NoScrollPhase, false);
   view.wheelEvent(&zoomInEvent);
   QVERIFY(zoomInEvent.isAccepted());
   QTRY_VERIFY(view.transform().m11() > 1.0);

   const qreal zoomedScale = view.transform().m11();
   QWheelEvent zoomOutEvent(wheelPosition, wheelPosition, QPoint(), QPoint(0, -120), Qt::NoButton,
                            Qt::NoModifier, Qt::NoScrollPhase, false);
   view.wheelEvent(&zoomOutEvent);
   QVERIFY(zoomOutEvent.isAccepted());
   QTRY_VERIFY(view.transform().m11() < zoomedScale);
}

void UiSmokeTest::compositesTextureBackground() {
   QImage textureImage(16, 16, QImage::Format_ARGB32);
   textureImage.fill(Qt::transparent);
   textureImage.setPixelColor(0, 0, Qt::red);
   const QPixmap texture = QPixmap::fromImage(textureImage);

   const QPixmap withoutOverlay = TextureBackground::composite(
       texture, QColor(QStringLiteral("#ffffff")), QColor(QStringLiteral("#dedede")), Qt::NoBrush);
   const QImage withoutOverlayImage = withoutOverlay.toImage();
   QCOMPARE(withoutOverlayImage.pixelColor(0, 0), QColor(Qt::red));
   QCOMPARE(withoutOverlayImage.pixelColor(1, 1), QColor(Qt::white));

   const QPixmap withOverlay =
       TextureBackground::composite(texture, QColor(QStringLiteral("#ffffff")),
                                    QColor(QStringLiteral("#dedede")), Qt::CrossPattern);
   QVERIFY(withOverlay.toImage() != withoutOverlayImage);
}

void UiSmokeTest::loadsProjectIntoSceneBackedWindow() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
   QCOMPARE(window.getTextureProject()->getNumNodes(), 11);
   QCOMPARE(window.getTextureProject()->getSinkNodeIds(), QList<int>({13}));
   for (int i = 0; i < 5; ++i) {
      window.reloadSceneView();
   }
   QCOMPARE(window.getTextureProject()->getNumNodes(), 11);
}

void UiSmokeTest::opensProjectWithoutDiscardingChanges() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   const qsizetype initialWindowCount = application->mainwindows.size();

   MainWindow* reusableWindow = application->addWindow();
   MainWindow* reusedWindow = application->openProject(
       reusableWindow, QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
   QCOMPARE(reusedWindow, reusableWindow);
   QCOMPARE(application->mainwindows.size(), initialWindowCount + 1);
   QCOMPARE(reusableWindow->getTextureProject()->getNumNodes(), 11);
   delete reusableWindow;

   MainWindow* savedWindow = application->addWindow();
   savedWindow->openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
   QVERIFY(!savedWindow->getTextureProject()->isModified());
   MainWindow* savedProjectWindow =
       application->openProject(savedWindow, QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
   QVERIFY(savedProjectWindow != savedWindow);
   QCOMPARE(application->mainwindows.size(), initialWindowCount + 2);
   QCOMPARE(savedWindow->getTextureProject()->getNumNodes(), 11);
   QCOMPARE(savedProjectWindow->getTextureProject()->getNumNodes(), 11);
   delete savedProjectWindow;
   delete savedWindow;

   MainWindow* existingWindow = application->addWindow();
   const int nodeId = existingWindow->getTextureProject()->newNode(1)->getId();
   existingWindow->getTextureProject()->removeNode(nodeId);
   QCOMPARE(existingWindow->getTextureProject()->getNumNodes(), 0);
   QVERIFY(existingWindow->getTextureProject()->isModified());

   MainWindow* openedWindow = application->openProject(
       existingWindow, QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
   QVERIFY(openedWindow != nullptr);
   QVERIFY(openedWindow != existingWindow);
   QCOMPARE(application->mainwindows.size(), initialWindowCount + 2);
   QCOMPARE(existingWindow->getTextureProject()->getNumNodes(), 0);
   QVERIFY(existingWindow->getTextureProject()->isModified());
   QCOMPARE(openedWindow->getTextureProject()->getNumNodes(), 11);

   delete openedWindow;
   delete existingWindow;
   QCOMPARE(application->mainwindows.size(), initialWindowCount);
}

void UiSmokeTest::managesTemporarySceneItems() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
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
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
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
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));
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
   QVERIFY(!connection->endpointLabelsVisible());
   QVERIFY(!connection->sourceNameLabelVisible());
   QVERIFY(!connection->receiverNameLabelVisible());
   QVERIFY(connection->zValue() > sourceItem->zValue());

   SettingsManager* settingsManager = window.getTextureProject()->getSettingsManager();
   QVERIFY(settingsManager != nullptr);
   settingsManager->setConnectionLabelSize(18);
   settingsManager->setDisplaySourceNames(true);
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
   receiver->setName(QStringLiteral("Renamed receiver"));
   QCOMPARE(connection->toolTip(),
            QStringLiteral("Renamed source (%1) → Renamed receiver (%2)")
                .arg(source->getGeneratorName(), receiver->getGeneratorName()));
   QCOMPARE(connection->getReceiverLabelText(),
            QStringLiteral("%1 (Renamed source)").arg(connection->getSlot()));
   connection->setHighlighted(false);
   QVERIFY(!connection->endpointLabelsVisible());
}

void UiSmokeTest::restoresPersistedComboBoxSelection() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("channels.ptmg")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   const QByteArray json(
       R"([
  {
    "id": "20",
    "name": "Channels",
    "generator": "Set channels",
    "settings": {
      "channelAlpha": "Second's alpha"
    }
  }
])");
   QCOMPARE(file.write(json), json.size());
   file.close();
   MainWindow window(application);
   window.openFile(file.fileName());

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
                                 "settings:[],generate(size,settings,output){void size;void "
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

void UiSmokeTest::selectsGeneratorFromAddNodePanel() {
   const QString description = QStringLiteral(
       "Generator details that are deliberately long enough to wrap across several lines in the "
       "information panel without being clipped or partially rendered.");
   const QString sourceIdentity = QStringLiteral(
       "C:/test-data/procedural-texture-maker/javascript-generators/long-path/inspectable.js");
   TextureProject project(false);
   EditManager editManager(project);
   AddNodePanel addNodePanel(project);
   ItemInfoPanel infoPanel(nullptr, &project, &editManager);
   QObject::connect(&addNodePanel, &AddNodePanel::generatorSelected, &infoPanel,
                    &ItemInfoPanel::setActiveGenerator);

   const QString script =
       QStringLiteral(
           "const generator={apiVersion:1,name:'Inspectable',description:'%1',"
           "type:'filter',inputs:['Image','Mask'],settings:["
           "{id:'zeta',type:'integer',name:'Zulu property',description:'First',default:4,min:0,"
           "max:10},{id:'alpha',type:'integer',name:'Alpha property',description:'Second',"
           "default:2,min:0,max:10}],"
           "generate(size,settings,output){void size;void settings;output.data.fill(0);}};")
           .arg(description);
   const TextureGeneratorPtr generator(
       new JsTexGen(script, sourceIdentity, TextureGenerator::Origin::Custom));
   QVERIFY(static_cast<JsTexGen*>(generator.data())->isValid());
   project.addGenerator(generator);

   addNodePanel.show();
   infoPanel.resize(340, 450);
   infoPanel.show();
   QApplication::processEvents();
   QPushButton* generatorButton = nullptr;
   for (QPushButton* button : addNodePanel.findChildren<QPushButton*>()) {
      if (button->text() == QStringLiteral("Inspectable")) {
         generatorButton = button;
         break;
      }
   }
   QVERIFY(generatorButton != nullptr);
   QCOMPARE(project.getNumNodes(), 0);
   QTest::mouseClick(generatorButton, Qt::LeftButton);
   QApplication::processEvents();
   QCOMPARE(project.getNumNodes(), 0);

   auto* generatorInfo =
       infoPanel.findChild<GeneratorInfoWidget*>(QStringLiteral("generatorInfoInspector"));
   QVERIFY(generatorInfo != nullptr);
   QVERIFY(!generatorInfo->isHidden());
   QCOMPARE(generatorInfo->getGenerator(), generator);
   auto* nameLabel = infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoName"));
   QCOMPARE(nameLabel->text(), QStringLiteral("Inspectable"));
   QVERIFY(qobject_cast<QGroupBox*>(nameLabel->parentWidget()) == nullptr);
   QVERIFY(nameLabel->font().bold());
   QVERIFY(nameLabel->font().pointSizeF() > QApplication::font().pointSizeF());
   QCOMPARE(nameLabel->alignment(), Qt::AlignLeft | Qt::AlignVCenter);
   QCOMPARE(nameLabel->contentsMargins().left(), 8);

   QMap<QString, QGroupBox*> infoGroups;
   for (QGroupBox* group : generatorInfo->findChildren<QGroupBox*>()) {
      infoGroups.insert(group->title(), group);
   }
   QCOMPARE(infoGroups.keys(),
            QStringList({QStringLiteral("Available properties"), QStringLiteral("Description"),
                         QStringLiteral("Details"), QStringLiteral("Inputs")}));
   QCOMPARE(infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoType"))->text(),
            QStringLiteral("Filter"));
   QCOMPARE(infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoOrigin"))->text(),
            QStringLiteral("Custom"));
   auto* timingLabel = infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoTiming"));
   QCOMPARE(infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoTimingKey"))->text(),
            QStringLiteral("Generation time:"));
   QCOMPARE(timingLabel->text(), QStringLiteral("N/A"));
   TexturePixel timedPixel;
   generator->generateWithTiming(QSize(1, 1), &timedPixel, {}, TextureNodeSettings());
   QTRY_VERIFY(timingLabel->text().endsWith(QStringLiteral("(avg. 1 run)")));
   generator->generateWithTiming(QSize(1, 1), &timedPixel, {}, TextureNodeSettings());
   QTRY_VERIFY(timingLabel->text().endsWith(QStringLiteral("(avg. 2 runs)")));
   for (int run = 0; run < 9; ++run) {
      generator->generateWithTiming(QSize(1, 1), &timedPixel, {}, TextureNodeSettings());
   }
   QTRY_VERIFY(timingLabel->text().endsWith(QStringLiteral("(avg. 10 runs)")));
   auto* descriptionLabel =
       infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoDescription"));
   QCOMPARE(descriptionLabel->text(), description);
   QVERIFY(descriptionLabel->wordWrap());
   QVERIFY(descriptionLabel->font().bold());
   QVERIFY(descriptionLabel->height() >=
           descriptionLabel->heightForWidth(descriptionLabel->width()));
   const QList<QLabel*> inputLabels =
       infoPanel.findChildren<QLabel*>(QStringLiteral("generatorInfoInput"));
   QCOMPARE(inputLabels.size(), 2);
   QCOMPARE(inputLabels.at(0)->text(), QStringLiteral("Image"));
   QCOMPARE(inputLabels.at(1)->text(), QStringLiteral("Mask"));
   QVERIFY(inputLabels.at(0)->font().bold());
   QVERIFY(inputLabels.at(1)->font().bold());
   QVERIFY(inputLabels.at(1)->geometry().top() > inputLabels.at(0)->geometry().bottom());
   const QList<QLabel*> settingInfoLabels =
       generatorInfo->findChildren<QLabel*>(QStringLiteral("generatorInfoSetting"));
   QCOMPARE(settingInfoLabels.size(), 2);
   QVERIFY(settingInfoLabels.at(0)->text().contains(QStringLiteral("Zulu property")));
   QVERIFY(settingInfoLabels.at(1)->text().contains(QStringLiteral("Alpha property")));
   QVERIFY(settingInfoLabels.at(1)->geometry().top() > settingInfoLabels.at(0)->geometry().top());
   auto* sourceLabel = infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoSource"));
   auto* sourceKeyLabel = infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoSourceKey"));
   QCOMPARE(sourceLabel->text(), sourceIdentity);
   QVERIFY(sourceLabel->wordWrap());
   auto* detailsGroup = infoPanel.findChild<QGroupBox*>(QStringLiteral("generatorInfoDetails"));
   auto* detailsLayout = qobject_cast<QFormLayout*>(detailsGroup->layout());
   int sourceKeyRow = -1;
   int sourceValueRow = -1;
   QFormLayout::ItemRole sourceKeyRole = QFormLayout::SpanningRole;
   QFormLayout::ItemRole sourceValueRole = QFormLayout::SpanningRole;
   detailsLayout->getWidgetPosition(sourceKeyLabel, &sourceKeyRow, &sourceKeyRole);
   detailsLayout->getWidgetPosition(sourceLabel, &sourceValueRow, &sourceValueRole);
   QCOMPARE(sourceKeyRow, sourceValueRow);
   QCOMPARE(sourceKeyRole, QFormLayout::LabelRole);
   QCOMPARE(sourceValueRole, QFormLayout::FieldRole);
   QCOMPARE(sourceKeyLabel->alignment(), Qt::AlignLeft | Qt::AlignTop);
   QCOMPARE(sourceLabel->geometry().left(),
            infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoType"))->geometry().left());
   QCOMPARE(sourceLabel->geometry().left(),
            infoPanel.findChild<QLabel*>(QStringLiteral("generatorInfoOrigin"))->geometry().left());
   QVERIFY2(sourceLabel->height() >= sourceLabel->heightForWidth(sourceLabel->width()),
            qPrintable(QStringLiteral("source size %1x%2 requires height %3 (minimum %4)")
                           .arg(sourceLabel->width())
                           .arg(sourceLabel->height())
                           .arg(sourceLabel->heightForWidth(sourceLabel->width()))
                           .arg(sourceLabel->minimumHeight())));

   infoPanel.resize(500, 450);
   QApplication::processEvents();
   infoPanel.resize(340, 450);
   QApplication::processEvents();
   QCOMPARE(sourceLabel->text(), sourceIdentity);
   QVERIFY(sourceLabel->height() >= sourceLabel->heightForWidth(sourceLabel->width()));

   const int wrappedSourceHeight = sourceLabel->height();
   const int wrappedDetailsHeight = detailsGroup->height();
   const TextureGeneratorPtr shortSourceGenerator(
       new JsTexGen(script, QString(), TextureGenerator::Origin::Custom));
   QVERIFY(static_cast<JsTexGen*>(shortSourceGenerator.data())->isValid());
   infoPanel.setActiveGenerator(shortSourceGenerator);
   QApplication::processEvents();
   QCOMPARE(sourceLabel->text(), QStringLiteral("Built-in C++"));
   QVERIFY(sourceLabel->height() < wrappedSourceHeight);
   QVERIFY(detailsGroup->height() < wrappedDetailsHeight);
   for (QLabel* label : generatorInfo->findChildren<QLabel*>()) {
      QVERIFY(!label->text().contains(QStringLiteral("Default:")));
   }

   const TextureNodePtr node = project.newNode(1, generator);
   QVERIFY(!node.isNull());
   infoPanel.setActiveNode(node->getId());
   QApplication::processEvents();
   auto* nodeSettings =
       infoPanel.findChild<NodeSettingsWidget*>(QStringLiteral("nodeSettingsInspector"));
   QVERIFY(nodeSettings != nullptr);
   const QList<QLabel*> settingLabels =
       nodeSettings->findChildren<QLabel*>(QStringLiteral("nodeSettingLabel"));
   QCOMPARE(settingLabels.size(), 2);
   QCOMPARE(settingLabels.at(0)->text(), QStringLiteral("Zulu property:"));
   QCOMPARE(settingLabels.at(1)->text(), QStringLiteral("Alpha property:"));
   QVERIFY(settingLabels.at(1)->parentWidget()->geometry().top() >
           settingLabels.at(0)->parentWidget()->geometry().top());
}

void UiSmokeTest::keepsGeneratorAndGraphSelectionsExclusive() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg"));

   auto* addNodePanel = window.findChild<AddNodePanel*>();
   auto* infoPanel = window.findChild<ItemInfoPanel*>();
   auto* view = window.findChild<QGraphicsView*>();
   QVERIFY(addNodePanel != nullptr);
   QVERIFY(infoPanel != nullptr);
   QVERIFY(view != nullptr);
   auto* scene = dynamic_cast<ViewNodeScene*>(view->scene());
   QVERIFY(scene != nullptr);

   QPushButton* generatorButton = nullptr;
   QPushButton* filterButton = nullptr;
   for (QPushButton* button : addNodePanel->findChildren<QPushButton*>()) {
      if (button->text() == QStringLiteral("Fill")) {
         generatorButton = button;
      } else if (button->text() == QStringLiteral("Box blur")) {
         filterButton = button;
      }
   }
   QVERIFY(generatorButton != nullptr);
   QVERIFY(filterButton != nullptr);

   generatorButton->click();
   QVERIFY(generatorButton->isChecked());
   filterButton->click();
   QVERIFY(filterButton->isChecked());
   QVERIFY(!generatorButton->isChecked());
   generatorButton->click();
   QVERIFY(generatorButton->isChecked());
   QVERIFY(!filterButton->isChecked());
   QVERIFY(scene->selectedItems().isEmpty());
   QCOMPARE(scene->getSelectedNode(), -1);
   QVERIFY(!infoPanel->findChild<GeneratorInfoWidget*>(QStringLiteral("generatorInfoInspector"))
                ->isHidden());

   ViewNodeItem* nodeItem = scene->getItem(13);
   QVERIFY(nodeItem != nullptr);
   nodeItem->setSelected(true);
   scene->setSelectedNode(nodeItem->getId());
   QVERIFY(!generatorButton->isChecked());

   generatorButton->click();
   QVERIFY(generatorButton->isChecked());
   QVERIFY(!nodeItem->isSelected());
   QCOMPARE(scene->getSelectedNode(), -1);

   ViewNodeLine* lineItem = nullptr;
   for (QGraphicsItem* item : scene->items()) {
      lineItem = dynamic_cast<ViewNodeLine*>(item);
      if (lineItem != nullptr) {
         break;
      }
   }
   QVERIFY(lineItem != nullptr);
   scene->clearSelection();
   lineItem->setSelected(true);
   scene->setSelectedLine(lineItem->getStartItemId(), lineItem->getEndItemId(),
                          lineItem->getSlot());
   QVERIFY(!generatorButton->isChecked());

   generatorButton->click();
   QVERIFY(generatorButton->isChecked());
   QVERIFY(!lineItem->isSelected());
   QCOMPARE(scene->getSelectedNode(), -1);

   auto* generatorInfo =
       infoPanel->findChild<GeneratorInfoWidget*>(QStringLiteral("generatorInfoInspector"));
   scene->selectSceneBackground();
   QVERIFY(!generatorButton->isChecked());
   QVERIFY(generatorInfo->isHidden());
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
   QVERIFY(selectedImage->hasSmoothFiltering());
   QVERIFY(lockedImage->hasSmoothFiltering());
   QVERIFY(cube->hasSmoothFiltering());

   settingsManager.setTextureFiltering(SettingsManager::TextureFiltering::Nearest);
   QVERIFY(!selectedImage->hasSmoothFiltering());
   QVERIFY(!lockedImage->hasSmoothFiltering());
   QVERIFY(!cube->hasSmoothFiltering());
   settingsManager.setTextureFiltering(SettingsManager::TextureFiltering::Smooth);

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
   selectedImage->setRenderStatus({TextureRenderState::Rendering, {}});
   lockedImage->setRenderStatus({TextureRenderState::Rendering, {}});
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
   selectedImage->setRenderStatus({TextureRenderState::Rendering, {}});
   cube->setTexture(selectedImage->pixmapWithStatusOverlay());
   QVERIFY(!cube->isHidden());
   QVERIFY(selectedImage->pixmapWithStatusOverlay().toImage() != previewPixmap.toImage());

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
