
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
#include "generators/builtinregistry.h"
#include "generators/javascript.h"
#include "global.h"
#include "gui/addnodepanel.h"
#include "gui/clipboardoperations.h"
#include "gui/cubewidget.h"
#include "gui/iteminfopanel.h"
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
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

/// @brief Constructor. Creates the main window and all its widgets.
/// @param parent
MainWindow::MainWindow(TexGenApplication* parent) {
   parentapp = parent;
   scene = nullptr;
   view = nullptr;
   preview3dwidget = nullptr;
   project = std::make_unique<TextureProject>();
   settingsManager = std::make_unique<SettingsManager>();
   project->setSettingsManager(settingsManager.get());

   QObject::connect(project.get(), &TextureProject::generatorNameCollision, this,
                    &MainWindow::generatorNameCollision);

   iteminfopanel = new ItemInfoPanel(this, project.get());
   iteminfopanel->hide();
   iteminfopanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   iteminfopanel->setMaximumWidth(560);
   iteminfopanel->setMinimumWidth(340);

   menuactions = new MenuActions(this);
   addnodewidget = new AddNodePanel(project.get());
   previewimagewidget = new PreviewImagePanel(project.get());
   settingspanel = new SettingsPanel(this, settingsManager.get());
   addnodewidget->hide();
   previewimagewidget->hide();
   settingspanel->hide();

   view = new ViewNodeView();
   view->show();
   scene = createScene();

   registerBuiltInGenerators(*project);
   project->clear();

   jstexgenManager = std::make_unique<JSTexGenManager>(project.get());

   auto* centerLayout = new QVBoxLayout;
   centerLayout->addWidget(view);

   auto* widget = new QSplitter(this);
   widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   widget->addWidget(iteminfopanel);
   widget->addWidget(view);
   widget->addWidget(addnodewidget);
   widget->addWidget(previewimagewidget);
   widget->addWidget(settingspanel);
   widget->setMinimumSize(1000, 450);
   widget->setChildrenCollapsible(false);
   widget->setCollapsible(0, false);
   widget->setCollapsible(1, false);
   widget->setStretchFactor(0, 0);
   widget->setStretchFactor(1, 100);
   setCentralWidget(widget);

   setGeometry(100, 100, 1000, 600);
   menuactions->setAddNodePanel(addnodewidget);
   menuactions->setPreviewImagePanel(previewimagewidget);
   menuactions->setSettingsPanel(settingspanel);
   menuactions->setItemInfoPanel(iteminfopanel);

   setWindowTitle("ProceduralTextureMaker");
   statusBar()->hide();
   setAttribute(Qt::WA_DeleteOnClose);

   if (QSettings().value("showhelpstartup", true).toBool()) {
      showHelp();
   }
}

/// @brief Will get called by the window manager when it tries to close this window.
void MainWindow::closeEvent(QCloseEvent* event) {
   if (!maybeSave()) {
      event->ignore();
      return;
   }
   event->accept();
}

/// @brief Destructor. Cleans up the project and application managers.
MainWindow::~MainWindow() {
   // MenuActions listens to application-wide window updates. Destroy it while this MainWindow and
   // its menus are still valid, before QObject emits MainWindow::destroyed.
   delete menuactions;
   menuactions = nullptr;
}

/// @brief Like saveFile with the difference that the user always gets prompted for a filename.
/// @return @c true if saved succesfully.
bool MainWindow::saveAs() { return saveFile(true); }

/// @brief Called when the user is trying to add a texture generator with the same name as one
/// already existing in the project. Displays a message box.
/// @param oldGen The one existing in the project.
/// @param newGen The new one.
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

/// @brief Saves the project as an XML file.
/// @param newFileName Don't reuse the previous saved file, ask for a new location.
/// @return @c true if suceesfully saved
bool MainWindow::saveFile(bool newFileName) {
   QString fileName;
   if (!newFileName) {
      fileName = savedFileName;
   }
   if (fileName.isNull()) {
      fileName =
          QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), "Texture Set (*.txl)");
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
   return true;
}

/// @brief Called before the project is closed.
/// @details If it has been modified the user gets to answer if he wants to save the changes. it's
/// not okay to close the file if the file isn't saved and the user hasn't chosen to discard the
/// changes.
/// @return @c true if now ok to close the project, @c false if not okay.
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

/// @brief Copies the selected node to the OS's clipboard.
void MainWindow::copyNode() { copyNodeToClipboard(*project, scene->getSelectedNode()); }

/// @brief Copies the selected node to the clipboard and removes it from the scene.
void MainWindow::cutNode() { cutNodeToClipboard(*project, scene->getSelectedNode()); }

/// @brief Reads the OS's clipboard buffer and if it contains
/// a texture node adds it to the scene.
void MainWindow::pasteNode() { pasteNodesFromClipboard(*project); }

/// @brief Saves the image of a node to a PNG file.
/// @param id
void MainWindow::saveImage(int id) {
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
   const TextureExportResult exportResult =
       TextureExporter::exportPng(*project, id, project->getPreviewSize(), fileName, true);
   if (!exportResult) {
      QMessageBox::warning(this, "Error", exportResult.message);
   }
}

/// @brief Creates a new scene instance.
/// @details If a parameter is parsed the other scene's nodes and connections are copied to this
/// one. Useful for debugging and resetting internal states.
/// @param source Scene to be copied
/// @return a new scene
ViewNodeScene* MainWindow::createScene(ViewNodeScene* source) {
   ViewNodeScene* newscene;
   if (source != nullptr) {
      newscene = scene->clone();
   } else {
      newscene = new ViewNodeScene(this);
   }
   view->setNodeScene(newscene);
   QObject::connect(newscene, &ViewNodeScene::nodeSelected, iteminfopanel,
                    &ItemInfoPanel::setActiveNode);
   QObject::connect(newscene, &ViewNodeScene::nodeSelected, previewimagewidget,
                    &PreviewImagePanel::setActiveNode);
   QObject::connect(newscene, &ViewNodeScene::lineSelected, iteminfopanel,
                    &ItemInfoPanel::setActiveLine);
   return newscene;
}

/// @brief Replaces the old node scene with a new one with the same content.
void MainWindow::reloadSceneView() {
   ViewNodeScene* oldscene = scene;
   scene = createScene(oldscene);
   oldscene->deleteLater();
}

/// @brief Shows all nodes and recreates the scene view.
void MainWindow::showAllNodesAndResetSceneView() {
   view->showAllNodes();
   reloadSceneView();
   view->showAllNodes();
}

/// @brief Reset the view zoom factor to default.
void MainWindow::resetViewZoom() { view->resetZoom(); }

/// @brief Zoom in on the node scene.
void MainWindow::zoomInView() { view->zoomIn(); }

/// @brief Zoom out on the node scene.
void MainWindow::zoomOutView() { view->zoomOut(); }

/// @brief Sets the line widths for the node connections in the scene view.
/// @param normalWidth Width for regular lines.
/// @param highlightedWidth Width for highlighted lines.
void MainWindow::setLineWidths(int normalWidth, int highlightedWidth) {
   if (scene) {
      scene->setLineWidths(normalWidth, highlightedWidth);
   }
}

/// @brief Sets the size of the line arrows in the scene view.
/// @param arrowSize Size of the line arrow.
void MainWindow::setArrowSize(int arrowSize) {
   if (scene) {
      scene->setArrowSize(arrowSize);
   }
}

/// @brief Sets the size of the node title area in the scene view.
/// @param headerSize Height of the node title area.
void MainWindow::setHeaderSize(int headerSize) {
   if (scene) {
      scene->setHeaderSize(headerSize);
   }
}

/// @brief Sets the zoom factor for each zoom step in the scene view.
/// @param factor Zoom factor.
void MainWindow::setZoomStepFactor(double factor) {
   if (view) {
      view->setZoomStepFactor(factor);
   }
}

/// @brief Opens a file and loads it into the project.
/// @details Before opening, check if the project has been modified and prompt the user to save it
/// first.
/// @param fileName
void MainWindow::openFile(const QString& fileName) {
   if (fileName.isNull()) {
      return;
   }
   if (!maybeSave()) {
      return;
   }
   const ProjectFileResult loadResult = ProjectFileService::load(fileName, *project);
   if (!loadResult) {
      QMessageBox::warning(this, "Error", loadResult.message);
      return;
   }
   savedFileName = fileName;
#ifdef Q_OS_MAC
   setWindowTitle(QFileInfo(fileName).fileName());
#else
   setWindowTitle(QString("%1 - ProceduralTextureMaker").arg(QFileInfo(fileName).fileName()));
#endif
   showAllNodesAndResetSceneView();
}

/// @brief Moves the window to the front, so that it lies on top of all other windows.
void MainWindow::moveToFront() {
   this->raise();
   this->activateWindow();
   this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

/// @brief Displays an about popup with information about ProceduralTextureMaker and the build date
/// of this version.
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

/// @brief Displays a help popup
void MainWindow::showHelp() {
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

/// @brief Clears the scene and removes all nodes and connections.
/// @details If the project has been modified, the user will be prompted to save it first.
void MainWindow::clearScene() {
   if (maybeSave()) {
      project->clear();
   }
}
