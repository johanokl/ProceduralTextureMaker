
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/settingsmanager.h"
#include "base/projectfileservice.h"
#include "base/textureexporter.h"
#include "base/textureimage.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/editmanager.h"
#include "generators/builtinregistry.h"
#include "base/jstexgenmanager.h"
#include "global.h"
#include "gui/addnodepanel.h"
#include "gui/clipboardoperations.h"
#include "gui/cubewidget.h"
#include "gui/iteminfopanel.h"
#include "gui/jstexgeneditor.h"
#include "gui/generatorinfowidget.h"
#include "base/jstexgen.h"
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include "gui/menuactions.h"
#include "gui/previewimagepanel.h"
#include "gui/settingspanel.h"
#include "mainwindow.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodescene.h"
#include "sceneview/viewnodeview.h"
#include "texgenapplication.h"
#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QPointer>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWindow>

MainWindow::MainWindow(TexGenApplication* parent) {
   parentapp = parent;
   project = std::make_unique<TextureProject>();
   editManager = std::make_unique<EditManager>(*project);
   settingsManager = std::make_unique<SettingsManager>();
   project->setSettingsManager(settingsManager.get());

   QObject::connect(project.get(), &TextureProject::generatorNameCollision, this,
                    &MainWindow::generatorNameCollision);

   iteminfopanel = new ItemInfoPanel(this, project.get(), editManager.get());
   iteminfopanel->hide();
   iteminfopanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   iteminfopanel->setMaximumWidth(560);
   iteminfopanel->setMinimumWidth(340);

   menuactions = std::make_unique<MenuActions>(this);
   addnodewidget = new AddNodePanel(*project);
   previewimagewidget = new PreviewImagePanel(*project);
   settingspanel = new SettingsPanel(this, settingsManager.get());
   addnodewidget->hide();
   previewimagewidget->hide();
   settingspanel->hide();

   view = new ViewNodeView();
   scene = createScene();

   registerBuiltInGenerators(*project);
   project->clear();
   editManager->reset();

   jstexgenManager = std::make_unique<JsTexGenManager>(project.get());
   QObject::connect(jstexgenManager.get(), &JsTexGenManager::diagnosticsAvailable, this,
                    [](const QStringList& diagnostics) {
                       for (const QString& diagnostic : diagnostics) {
                          qWarning().noquote() << diagnostic;
                       }
                    });

   auto* widget = new QSplitter(this);
   widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   widget->addWidget(iteminfopanel);
   widget->addWidget(view);
   widget->addWidget(addnodewidget);
   widget->addWidget(previewimagewidget);
   widget->addWidget(settingspanel);
   widget->setMinimumSize(1000, 200);
   widget->setChildrenCollapsible(false);
   widget->setCollapsible(0, false);
   widget->setCollapsible(1, false);
   widget->setStretchFactor(0, 0);
   widget->setStretchFactor(1, 100);
   auto* vertical = new QSplitter(Qt::Vertical, this);
   vertical->setMinimumSize(1000, 450);
   vertical->addWidget(widget);
   jsTexGenEditor = new JsTexGenEditor(*jstexgenManager, *settingsManager, vertical);
   connect(addnodewidget, &AddNodePanel::createCustomVersion, this,
           &MainWindow::createCustomVersion);
   jsTexGenEditor->hide();
   vertical->addWidget(jsTexGenEditor);
   vertical->setStretchFactor(0, 1);
   vertical->setStretchFactor(1, 1);
   setCentralWidget(vertical);
   auto* generatorInfo = iteminfopanel->findChild<GeneratorInfoWidget*>();
   connect(jsTexGenEditor, &JsTexGenEditor::generatorSaved, addnodewidget,
           &AddNodePanel::selectGenerator);
   connect(jsTexGenEditor, &JsTexGenEditor::draftMetadataChanged, generatorInfo,
           &GeneratorInfoWidget::setDraftMetadata);
   connect(generatorInfo, &GeneratorInfoWidget::createCustomVersion, this,
           &MainWindow::createCustomVersion);
   connect(generatorInfo, &GeneratorInfoWidget::metadataEdited, this,
           [this](const QString& property, const QString& value) {
              jsTexGenEditor->setMetadata(property, value);
              if (jsTexGenEditor->isHidden()) toggleJsTexGenEditor();
           });

   setGeometry(100, 100, 1000, 600);
   menuactions->setAddNodePanel(addnodewidget);
   menuactions->setPreviewImagePanel(previewimagewidget);
   menuactions->setSettingsPanel(settingspanel);
   menuactions->setItemInfoPanel(iteminfopanel);

   auto* renderTimer = new QTimer(this);
   connect(renderTimer, &QTimer::timeout, this,
           [this, dialog = QPointer<QMessageBox>(), waiting = TextureSlowRender{}]() mutable {
              if (dialog) {
                 if (!project->isRendering(waiting)) {
                    dialog->close();
                 }
                 return;
              }
              const auto slow = project->slowRender();
              if (!slow) {
                 return;
              }
              waiting = *slow;
              dialog = new QMessageBox(
                  QMessageBox::Warning, "Slow node render",
                  QString("Node %1 (%2) has been rendering for more than 60 seconds. "
                          "Cancel rendering or continue waiting?")
                      .arg(waiting.nodeId)
                      .arg(waiting.generatorName),
                  QMessageBox::Cancel, this);
              auto* continueButton = dialog->addButton("Continue", QMessageBox::AcceptRole);
              dialog->setDefaultButton(QMessageBox::Cancel);
              dialog->setAttribute(Qt::WA_DeleteOnClose);
              connect(dialog.data(), &QMessageBox::finished, this,
                      [this, prompt = dialog.data(), continueButton, node = waiting](int) {
                         // The graph may have changed or finished while the non-blocking dialog was
                         // open.
                         if (!project->isRendering(node)) {
                            return;
                         }
                         if (prompt->clickedButton() == continueButton) {
                            project->continueRendering(node);
                         } else {
                            project->stopRendering();
                         }
                      });
              dialog->open();
           });
   renderTimer->start(1000);

   setWindowTitle("ProceduralTextureMaker");
   statusBar()->hide();
   connect(
       project.get(), &TextureProject::historyReplayAdjusted, this,
       [this](const QString& message) {
          statusBar()->show();
          statusBar()->showMessage(message, 8000);
       },
       Qt::QueuedConnection);
   connect(project.get(), &TextureProject::renderFailed, this,
           [this](int id, QSize, const QString& message) {
              statusBar()->show();
              statusBar()->showMessage(QString("Node %1: %2").arg(id).arg(message));
           });
   connect(project.get(), &TextureProject::thumbnailRenderStatusChanged, this, [this](int id) {
      if (project->thumbnailRenderStatus(id).state == TextureRenderState::Rendering) {
         statusBar()->clearMessage();
         statusBar()->hide();
      }
   });
   setAttribute(Qt::WA_DeleteOnClose);
   restoreGeometry(QSettings().value("mainWindowGeometry").toByteArray());
   if (QSettings().value("jsTexGenEditorVisible", false).toBool()) toggleJsTexGenEditor();

   if (QSettings().value("showhelpstartup", true).toBool()) {
      showHelp();
   }
}

void MainWindow::closeEvent(QCloseEvent* event) {
   if (!jsTexGenEditor->canClose() || !maybeSave()) {
      event->ignore();
      return;
   }
   if (imageExporter) {
      imageExporter->cancel();
   }
   QSettings settings;
   settings.setValue("mainWindowGeometry", saveGeometry());
   settings.setValue("jsTexGenEditorVisible", !jsTexGenEditor->isHidden());
   if (!jsTexGenEditor->isHidden())
      settings.setValue("jsTexGenEditorSplitter",
                        qobject_cast<QSplitter*>(centralWidget())->saveState());
   event->accept();
}

MainWindow::~MainWindow() {
   imageExporter.reset();
   // MenuActions listens to application-wide window updates. Destroy it while this MainWindow and
   // its menus are still valid, before QObject emits MainWindow::destroyed.
   menuactions.reset();
   destroyGuiObservers();
}

void MainWindow::destroyGuiObservers() {
   if (view != nullptr) {
      view->setNodeScene(nullptr);
   }
   scene.reset();
   delete takeCentralWidget();
   view = nullptr;
   iteminfopanel = nullptr;
   settingspanel = nullptr;
   addnodewidget = nullptr;
   previewimagewidget = nullptr;
   jsTexGenEditor = nullptr;
}

bool MainWindow::saveAs() { return saveFile(true); }

void MainWindow::generatorNameCollision(const TextureGeneratorPtr& oldGen,
                                        const TextureGeneratorPtr& newGen) {
   QString question;
   question.append("There is a already a texture generator with the name ");
   question.append(oldGen->getName() + ". ");
   question.append("Replace it with the newer one just found?");
   if (QMessageBox::question(this, "Name collision", question,
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      project->removeGenerator(oldGen);
      project->addGenerator(newGen);
   }
}

bool MainWindow::saveFile(bool newFileName) {
   QString fileName;
   if (!newFileName) {
      fileName = savedFileName;
   }
   if (fileName.isNull()) {
      fileName =
          QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), "PTM graph (*.ptmg)");
   }
   if (fileName.isNull()) {
      return false;
   }
   QFileInfo testFile(fileName);
   if (testFile.exists() && newFileName) {
      QMessageBox msgBox(this);
      msgBox.setText(
          "There already exists a file at this location. \n"
          "Still want to save and thus overwrite the file or do you want to cancel the operation?");
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      int ret = msgBox.exec();
      switch (ret) {
         case QMessageBox::Save:
            break;
         case QMessageBox::Cancel:
         default:
            return false;
      }
   }
   const ProjectFileResult saveResult = ProjectFileService::save(fileName, *project, true);
   if (!saveResult) {
      QMessageBox::warning(this, "Error", saveResult.message);
      return false;
   }
#ifdef Q_OS_MAC
   setWindowTitle(QFileInfo(fileName).fileName());
#else
   setWindowTitle(QString("%1 - ProceduralTextureMaker").arg(QFileInfo(fileName).fileName()));
#endif
   savedFileName = fileName;
   editManager->setClean();
   return true;
}

bool MainWindow::maybeSave() {
   if (!project->isModified()) {
      return true;
   }
   const QMessageBox::StandardButton ret =
       QMessageBox::information(this, "ProceduralTextureMaker",
                                "The document has been modified.\n"
                                "Do you want to save your changes?",
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
   switch (ret) {
      case QMessageBox::Save:
         return saveFile(savedFileName.isEmpty());
      case QMessageBox::Cancel:
         return false;
      default:
         break;
   }
   return true;
}

void MainWindow::copyNode() { copyNodeToClipboard(*project, scene->getSelectedNode()); }

void MainWindow::cutNode() { cutNodeToClipboard(*project, *editManager, scene->getSelectedNode()); }

void MainWindow::pasteNode() { pasteNodesFromClipboard(*editManager); }

void MainWindow::saveImage(int id) {
   if (imageExporter) {
      return;
   }
   if (id == 0) {
      id = scene->getSelectedNode();
   }
   TextureNodePtr texNode = project->getNode(id);
   if (texNode.isNull()) {
      return;
   }
   QString fileName =
       QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), "PNG (*.png)");

   if (fileName.isNull()) {
      return;
   }
   QFileInfo testFile(fileName);
   if (testFile.exists()) {
      QMessageBox msgBox(this);
      msgBox.setText(
          "There already exists a file at this location. \n"
          "Still want to save and thus overwrite the file or do you want to cancel the operation?");
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      int ret = msgBox.exec();
      switch (ret) {
         case QMessageBox::Save:
            break;
         case QMessageBox::Cancel:
         default:
            return;
      }
   }
   exportImage(id, fileName);
}

void MainWindow::exportImage(int id, const QString& fileName) {
   if (imageExporter) {
      return;
   }
   imageExporter = std::make_unique<TextureExporter>();
   auto* progress = new QProgressDialog("Rendering image...", "Cancel", 0, 0, this);
   progress->setWindowTitle("Export image");
   progress->setWindowModality(Qt::NonModal);
   progress->setAutoClose(false);
   progress->setAutoReset(false);
   progress->show();
   menuactions->setImageExportEnabled(false);
   connect(progress, &QProgressDialog::canceled, imageExporter.get(), &TextureExporter::cancel);
   connect(imageExporter.get(), &TextureExporter::progress, progress,
           [progress](int completed, int total) {
              progress->setRange(0, total + 1);
              progress->setValue(completed);
              if (completed == total) {
                 progress->setLabelText("Writing PNG...");
              }
           });
   connect(imageExporter.get(), &TextureExporter::finished, this,
           [this, progress](TextureExportResult result) {
              progress->close();
              progress->deleteLater();
              imageExporter.reset();
              menuactions->setImageExportEnabled(true);
              if (!result && result.error != TextureExportError::Cancelled) {
                 QMessageBox::warning(this, "Export failed", result.message);
              }
           });
   imageExporter->start(*project, id, project->getPreviewSize(), fileName, true);
}

std::unique_ptr<ViewNodeScene> MainWindow::createScene(ViewNodeScene* source) {
   std::unique_ptr<ViewNodeScene> newscene;
   if (source != nullptr) {
      newscene = source->clone();
   } else {
      newscene = std::make_unique<ViewNodeScene>(*this);
   }
   view->setNodeScene(newscene.get());
   QObject::connect(newscene.get(), &ViewNodeScene::nodeSelected, iteminfopanel,
                    &ItemInfoPanel::setActiveNode);
   QObject::connect(newscene.get(), &ViewNodeScene::nodeSelected, previewimagewidget,
                    &PreviewImagePanel::setActiveNode);
   QObject::connect(newscene.get(), &ViewNodeScene::lineSelected, iteminfopanel,
                    &ItemInfoPanel::setActiveLine);
   QObject::connect(newscene.get(), &ViewNodeScene::nodeSelected, addnodewidget, [this](int id) {
      if (jsTexGenEditor) jsTexGenEditor->setGenerator({});
      if (id >= 0) {
         addnodewidget->clearGeneratorSelection();
      }
   });
   QObject::connect(newscene.get(), &ViewNodeScene::lineSelected, addnodewidget,
                    &AddNodePanel::clearGeneratorSelection);
   connect(newscene.get(), &ViewNodeScene::lineSelected, newscene.get(), [this] {
      if (jsTexGenEditor) jsTexGenEditor->setGenerator({});
   });
   QObject::connect(newscene.get(), &ViewNodeScene::sceneBackgroundSelected, addnodewidget,
                    &AddNodePanel::clearGeneratorSelection);
   connect(newscene.get(), &ViewNodeScene::sceneBackgroundSelected, newscene.get(), [this] {
      if (jsTexGenEditor) jsTexGenEditor->setGenerator({});
   });
   QObject::connect(addnodewidget, &AddNodePanel::generatorSelected, newscene.get(),
                    [this, scene = newscene.get()](const TextureGeneratorPtr& generator) {
                       scene->clearGraphSelection();
                       iteminfopanel->setActiveGenerator(generator);
                       jsTexGenEditor->setGenerator(generator);
                    });
   return newscene;
}

void MainWindow::reloadSceneView() { scene = createScene(scene.get()); }

void MainWindow::reloadJavaScriptGenerators() {
   if (jstexgenManager) {
      jstexgenManager->reload();
   }
}

void MainWindow::showAllNodesAndResetSceneView() {
   view->showAllNodes();
   reloadSceneView();
   view->showAllNodes();
}

void MainWindow::resetViewZoom() { view->resetZoom(); }

void MainWindow::zoomInView() { view->zoomIn(); }

void MainWindow::zoomOutView() { view->zoomOut(); }

void MainWindow::setLineWidths(int normalWidth, int highlightedWidth) {
   if (scene) {
      scene->setLineWidths(normalWidth, highlightedWidth);
   }
}

void MainWindow::setArrowSize(int arrowSize) {
   if (scene) {
      scene->setArrowSize(arrowSize);
   }
}

void MainWindow::setHeaderSize(int headerSize) {
   if (scene) {
      scene->setHeaderSize(headerSize);
   }
}

void MainWindow::setZoomStepFactor(double factor) {
   if (view) {
      view->setZoomStepFactor(factor);
   }
}

void MainWindow::openFile(const QString& fileName) {
   if (fileName.isNull()) {
      return;
   }
   // A project can reference custom generators discovered during startup.
   if (jstexgenManager->isScanning()) {
      QTimer::singleShot(10, this, [this, fileName] { openFile(fileName); });
      return;
   }
   // A newly shown window must be exposed before repaint() can paint it.
   if (isVisible() && !isMinimized() && !windowHandle()->isExposed()) {
      QTimer::singleShot(0, this, [this, fileName] { openFile(fileName); });
      return;
   }
   if (!maybeSave()) {
      return;
   }
   // Paint the window and empty canvas before loading blocks the GUI thread.
   view->setGraphVisible(false);
   repaint();
   const ProjectFileResult loadResult = ProjectFileService::load(fileName, *project);
   if (!loadResult) {
      view->setGraphVisible(true);
      QMessageBox::warning(this, "Error", loadResult.message);
      return;
   }
   savedFileName = fileName;
   editManager->reset();
#ifdef Q_OS_MAC
   setWindowTitle(QFileInfo(fileName).fileName());
#else
   setWindowTitle(QString("%1 - ProceduralTextureMaker").arg(QFileInfo(fileName).fileName()));
#endif
   reloadSceneView();
   // Wait for the restored layout before fitting, then paint only the final zoom.
   QTimer::singleShot(0, view, [this] {
      QTimer::singleShot(0, view, [this] {
         view->showAllNodes();
         view->setGraphVisible(true);
      });
   });
}

void MainWindow::moveToFront() {
   this->raise();
   this->activateWindow();
   this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

void MainWindow::showAbout() {
   QString aboutText;
   QTextStream ts(&aboutText);
   ts << "<p align='center'>"
      << "<h2>ProceduralTextureMaker</h2>"
      << "<hr><br>"
      << "Johan Lindqvist"
      << "<br>"
      << "<a href='mailto:johan.lindqvist@gmail.com'>johan.lindqvist@gmail.com</a>"
      << "</p>"
      << "<p align='center'>"
      << "More information at<br>"
      << "<a href='https://github.com/johanokl/ProceduralTextureMaker'>"
      << "github.com/johanokl/ProceduralTextureMaker"
      << "</a>"
      << "</p>"
      << "<p align='center'>"
      << "This version built: %1"
      << "</p>";
   QMessageBox::about(this, "About", aboutText.arg(__DATE__));
}

void MainWindow::showHelp() {
   if (helpDialog) {
      helpDialog->show();
      helpDialog->raise();
      helpDialog->activateWindow();
      return;
   }

   QString helpText;
   QTextStream ts(&helpText);
   ts << "<h1>Help</h1>"
      << "<h2>Add nodes</h2>"
      << "<p>There are two methods available:<br>"
      << "1. Drag a node from the Add node panel on the right to the scene view in the center.<br>"
      << "2. Right click in the scene view and select the node you want to add from the context "
         "menu.</p>"
      << "<h2>Remove nodes</h2>"
      << "<p>Right click on a node and select \"Remove node\" in the context menu.</p>"
      << "<h2>Connect nodes</h2>"
      << "<p>Connect nodes by holding down the ctrl key and dragging a connection to the node.</p>"
      << "<h2>Remove connections</h2>"
      << "<p>There are two methods available:<br>"
      << "1. Select the receiver and in the node settings panel press the \"Clear\" button.<br>"
      << "2. Select the connection line in the scene view and either press the keyboard's delete "
         "key "
      << "or press the \"Disconnect\" button in the connection info panel.</p>"
      << "<h2>Export images</h2>"
      << "<p>Right click on a node and select \"Save selected image\".</p>"
      << "<h2>Graph</h2>"
      << "<p>Zoom in and out in the scene view by holding down the alt or shift keys and "
      << "scrolling the mouse wheel.<br>"
      << "Select \"Show all nodes\" or \"Reset zoom\" to restore the view.</p>"
      << "<h2>Adding Javascript</h2>"
      << "<p>Open the settings panel and set the directory for the generators.<br>"
      << "For info about how the scripts should look, see "
         "https://github.com/johanokl/ProceduralTextureMaker.</p>";

   auto* dialog = new QDialog(this);
   dialog->setAttribute(Qt::WA_DeleteOnClose);
   helpDialog = dialog;
   auto* help = new QTextEdit;
   help->setReadOnly(true);
   help->setText(helpText);
   help->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   QCheckBox* displayOnStart = new QCheckBox("Show on startup");
   displayOnStart->setChecked(QSettings().value("showhelpstartup", true).toBool());
   QObject::connect(displayOnStart, &QCheckBox::toggled,
                    [=](bool val) { QSettings().setValue("showhelpstartup", val); });
   QPushButton* closeButton = new QPushButton("Close");
   QObject::connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
   auto* layout = new QVBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(help);
   layout->addWidget(displayOnStart);
   layout->addWidget(closeButton);
   dialog->setWindowTitle("Help");
   dialog->setMinimumSize(300, 400);
   dialog->setLayout(layout);
   dialog->show();
}

void MainWindow::clearScene() {
   if (maybeSave()) {
      editManager->clearProject();
   }
}

void MainWindow::toggleJsTexGenEditor() {
   auto* splitter = qobject_cast<QSplitter*>(centralWidget());
   if (!jsTexGenEditor->isHidden())
      QSettings().setValue("jsTexGenEditorSplitter", splitter->saveState());
   jsTexGenEditor->setVisible(jsTexGenEditor->isHidden());
   if (!jsTexGenEditor->isHidden() &&
       !splitter->restoreState(QSettings().value("jsTexGenEditorSplitter").toByteArray())) {
      splitter->setSizes({splitter->height() / 2, splitter->height() / 2});
   }
}

void MainWindow::createCustomVersion(const TextureGeneratorPtr& generator) {
   auto* js = dynamic_cast<JsTexGen*>(generator.data());
   if (!js) return;
   const QString name = "Custom " + js->getName();
   if (project->getGenerators().contains(name)) {
      QMessageBox::warning(this, tr("Create custom version"),
                           tr("A generator named %1 already exists.").arg(name));
      return;
   }
   QString directory = settingsManager->getJSTextureGeneratorsPath();
   if (directory.isEmpty())
      directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/generators";
   if (!QDir().mkpath(directory)) {
      QMessageBox::warning(this, tr("Create custom version"),
                           tr("Could not create the generator directory."));
      return;
   }
   QString filename = name.toLower();
   filename.replace(QRegularExpression("[^a-z0-9]+"), "-");
   const QString path = QDir(directory).filePath(filename + ".js");
   if (QFileInfo::exists(path)) {
      QMessageBox::warning(this, tr("Create custom version"),
                           tr("The file already exists: %1").arg(path));
      return;
   }
   QString source = JsTexGenEditor::withProperty(js->source(), "name", name);
   source = JsTexGenEditor::withProperty(source, "filename", QFileInfo(path).fileName());
   QString error;
   const auto custom = jstexgenManager->saveScript({}, source, path, error);
   if (!custom) {
      QMessageBox::warning(this, tr("Create custom version"), error);
      return;
   }
   settingsManager->setJSTextureGeneratorsPath(directory);
   settingsManager->setJSTextureGeneratorsEnabled(true);
   if (!settingsManager->saveSettings()) {
      QMessageBox::warning(
          this, tr("Generator settings"),
          tr("The script was saved, but the generator folder setting could not be saved."));
   }
   addnodewidget->selectGenerator(custom);
   if (jsTexGenEditor->isHidden()) toggleJsTexGenEditor();
}
