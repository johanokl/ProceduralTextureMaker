/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "core/settingsmanager.h"
#include "core/textureimage.h"
#include "core/texturenode.h"
#include "core/textureproject.h"
#include "generators/blending.h"
#include "generators/boxblur.h"
#include "generators/bricks.h"
#include "generators/checkboard.h"
#include "generators/circle.h"
#include "generators/cutout.h"
#include "generators/displacementmap.h"
#include "generators/fill.h"
#include "generators/fire.h"
#include "generators/gaussianblur.h"
#include "generators/glow.h"
#include "generators/gradient.h"
#include "generators/greyscale.h"
#include "generators/invert.h"
#include "generators/javascript.h"
#include "generators/lens.h"
#include "generators/lines.h"
#include "generators/merge.h"
#include "generators/mirror.h"
#include "generators/modifylevels.h"
#include "generators/noise.h"
#include "generators/normalmap.h"
#include "generators/perlinnoise.h"
#include "generators/pixelate.h"
#include "generators/pointillism.h"
#include "generators/setchannels.h"
#include "generators/shadow.h"
#include "generators/sineplasma.h"
#include "generators/sinetransform.h"
#include "generators/square.h"
#include "generators/stackblur.h"
#include "generators/star.h"
#include "generators/text.h"
#include "generators/transform.h"
#include "generators/whirl.h"
#include "global.h"
#include "gui/addnodepanel.h"
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
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(TexGenApplication* parent)
{
   parentapp = parent;
   project = new TextureProject();
   settingsManager = new SettingsManager;
   project->setSettingsManager(settingsManager);

   QObject::connect(project, &TextureProject::generatorNameCollision,
                    this, &MainWindow::generatorNameCollision);

   iteminfopanel = new ItemInfoPanel(this, project);
   iteminfopanel->hide();
   iteminfopanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   iteminfopanel->setMaximumWidth(500);
   iteminfopanel->setMinimumWidth(300);

   menuactions = new MenuActions(this);
   addnodewidget = new AddNodePanel(project);
   previewimagewidget = new PreviewImagePanel(project);
   settingspanel = new SettingsPanel(this, settingsManager);
   addnodewidget->hide();
   previewimagewidget->hide();
   settingspanel->hide();

   view = new ViewNodeView(settingsManager->getDefaultZoom());
   view->show();
   scene = createScene();

   project->addGenerator(TextureGeneratorPtr(new BlendingTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new BoxBlurTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new BricksTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new CheckboardTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new CircleTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new CutoutTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new DisplacementMapTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new FillTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new FireTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GaussianBlurTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GlowTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GradientTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GreyscaleTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new InvertTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new LinesTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new LensTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new MergeTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new MirrorTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new ModifyLevelsTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new NoiseTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new NormalMapTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new PerlinNoiseTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new PixelateTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new PointillismTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SetChannelsTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new ShadowTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SinePlasmaTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SineTransformTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SquareTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new StarTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new StackBlurTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new TextTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new TransformTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new WhirlTextureGenerator()));
   project->clear();

   jstexgenManager = new JSTexGenManager(project);

   auto* centerLayout = new QVBoxLayout;
   centerLayout->addWidget(view);

   auto* widget = new QSplitter(this);
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


/**
 * @brief MainWindow::closeEvent
 *
 * Will get called by the window manager when it tries to close this window.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
   if (!maybeSave()) {
      event->ignore();
      return;
   }
   event->accept();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
   delete menuactions;
   delete scene;
   delete view;
   delete settingsManager;
   delete project;
}

/**
 * @brief MainWindow::saveAs
 * @return true if saved succesfully.
 * Like saveFile with the difference that the user always gets prompted for a filename.
 */
bool MainWindow::saveAs()
{
   return saveFile(true);
}

/**
 * @brief MainWindow::generatorNameCollision
 * @param oldGen The one existing in the project.
 * @param newGen The new one.

 * Called when the user is trying to add a texture generator with the same
 * name as one already existing in the project.
 * Displays a message box.
 */
void MainWindow::generatorNameCollision(const TextureGeneratorPtr& oldGen, const TextureGeneratorPtr& newGen)
{
   QString question;
   question.append("There is a already a texture generator with the name ");
   question.append(oldGen->getName() + ". ");
   question.append("Replace it with the newer one just found?");
   if (QMessageBox::question(this, "Name collision", question,
                             QMessageBox::Yes | QMessageBox::No)
       == QMessageBox::Yes) {
      project->removeGenerator(oldGen);
      project->addGenerator(newGen);
   }
}

/**
 * @brief MainWindow::saveFile
 * @param newFileName Don't reuse the previous saved file, ask for a new location.
 * @return true if suceesfully saved
 *
 * Saves the project as an xml file.
 */
bool MainWindow::saveFile(bool newFileName)
{
   QString fileName;
   if (!newFileName) {
      fileName = savedFileName;
   }
   if (fileName.isNull()) {
      fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(),
                                              "Texture Set (*.txl)");
   }
   if (fileName.isNull()) {
      return false;
   }
   QFileInfo testFile(fileName);
   if (testFile.exists() && newFileName) {
      QMessageBox msgBox(this);
      msgBox.setText("There already exists a file at this location. \n"
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
   QFile outputFile(fileName);
   if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::warning(this, "Error", "Could not save file to this location.");
      return false;
   }
   QByteArray sceneFile = project->saveAsXML().toString(3).toLatin1();
   outputFile.write(sceneFile);
   outputFile.close();
#ifdef Q_OS_MAC
   setWindowTitle(QFileInfo(outputFile).fileName());
#else
   setWindowTitle(QString("%1 - ProceduralTextureMaker").arg(QFileInfo(outputFile).fileName()));
#endif
   savedFileName = fileName;
   return true;
}

/**
 * @brief MainWindow::maybeSave
 * @return true if now ok to close the project, false if not okay.
 *
 * Called before the project is closed. If it has been modified the user gets
 * to answer if he wants to save the changes. it's not okay to close the file
 * if the file isn't saved and the user hasn't chosen to discard the changes.
 */
bool MainWindow::maybeSave()
{
   if (!project->isModified()) {
      return true;
   }
   const QMessageBox::StandardButton ret =
         QMessageBox::information(
            this, "ProceduralTextureMaker",
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

/**
 * @brief MainWindow::copyNode
 * Copies the selected node to the OS's clipboard.
 */
void MainWindow::copyNode()
{
   project->copyNode(scene->getSelectedNode());
}

/**
 * @brief MainWindow::cutNode
 * Copies the selected node to the clipboard and removes it from the scene.
 */
void MainWindow::cutNode()
{
   project->cutNode(scene->getSelectedNode());
}

/**
 * @brief MainWindow::pasteNode
 * Reads the OS's clipboard buffer and if it contains
 * a texture node adds it to the scene.
 */
void MainWindow::pasteNode()
{
   project->pasteNode();
}

/**
 * @brief MainWindow::saveImage
 */
void MainWindow::saveImage(int id)
{
   if (id == 0) {
      id = scene->getSelectedNode();
   }
   TextureNodePtr texNode = project->getNode(id);
   if (texNode.isNull()) {
      return;
   }
   QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), "PNG (*.png)");

   if (fileName.isNull()) {
      return;
   }
   QFileInfo testFile(fileName);
   if (testFile.exists()) {
      QMessageBox msgBox(this);
      msgBox.setText("There already exists a file at this location. \n"
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
   QSize outSize = project->getPreviewSize();
   QImage tempimage = QImage(outSize.width(), outSize.height(), QImage::Format_ARGB32);
   memcpy(tempimage.bits(),
          texNode->getImage(outSize)->getData(),
          outSize.width() * outSize.height() * sizeof(TexturePixel));
   tempimage.save(fileName, "PNG", 100);
}

/**
 * @brief MainWindow::createScene
 * @param source Scene to be copied
 * @return a new scene
 *
 * Creates a new scene instance. If a parameeter is parsed the other scene's
 * nodes and connections are copied to this one.
 * Useful for debugging and resetting internal states.
 */
ViewNodeScene* MainWindow::createScene(ViewNodeScene* source)
{
   ViewNodeScene* newscene;
   if (source != nullptr) {
      newscene = scene->clone();
   } else {
      newscene = new ViewNodeScene(this);
   }
   view->setScene(newscene);
   QObject::connect(newscene, &ViewNodeScene::nodeSelected,
                    iteminfopanel, &ItemInfoPanel::setActiveNode);
   QObject::connect(newscene, &ViewNodeScene::nodeSelected,
                    previewimagewidget, &PreviewImagePanel::setActiveNode);
   QObject::connect(newscene, &ViewNodeScene::lineSelected,
                    iteminfopanel, &ItemInfoPanel::setActiveLine);
   return newscene;
}

/**
 * @brief MainWindow::reloadSceneView
 * Replaces the old node scene with a new one with the same content.
 */
void MainWindow::reloadSceneView()
{
   ViewNodeScene* oldscene = scene;
   scene = createScene(oldscene);
   oldscene->deleteLater();
}

/**
 * @brief MainWindow::resetViewZoom
 * Reset the view zoom factor to default.
 */
void MainWindow::resetViewZoom()
{
   view->resetZoom();
}

/**
 * @brief MainWindow::openFile
 * @param fileName
 */
void MainWindow::openFile(const QString& fileName)
{
   if (fileName.isNull()) {
      return;
   }
   if (!maybeSave()) {
      return;
   }
   QFile inputFile(fileName);
   if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::warning(this, "Error", "Could not open the specified file.");
      return;
   }
   if (QFileInfo(fileName).size() < 100) {
      QMessageBox::warning(this, "Error", "File not a valid TXL file.");
      return;
   }
   QString inputString(inputFile.readAll());
   QDomDocument inputXmlDocument;
   inputXmlDocument.setContent(inputString);

   project->clear();
   project->loadFromXML(inputXmlDocument);
   savedFileName = fileName;
#ifdef Q_OS_MAC
   setWindowTitle(QFileInfo(inputFile).fileName());
#else
   setWindowTitle(QString("%1 - ProceduralTextureMaker").arg(QFileInfo(inputFile).fileName()));
#endif
}

/**
 * @brief MainWindow::moveToFront
 * Moves the window to the front, so that it lies on top of all other windows.
 */
void MainWindow::moveToFront()
{
   this->raise();
   this->activateWindow();
   this->setWindowState((this->windowState()&  ~Qt::WindowMinimized) | Qt::WindowActive);
}

/**
 * @brief MainWindow::showAbout
 */
void MainWindow::showAbout()
{
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

/**
 * @brief MainWindow::showHelp
 * Displays a help popup
 */
void MainWindow::showHelp()
{
   QString helpText;
   QTextStream ts(&helpText);
   ts << "<h1>Help</h1>"
      << "<h2>Add nodes</h2>"
      << "<p>There are two methods available:<br>"
      << "1. Drag a node from the Add node panel on the right to the scene view in the center.<br>"
      << "2. Right click in the scene view and select the node you want to add from the context menu.</p>"
      << "<h2>Remove nodes</h2>"
      << "<p>Right click on a node and select \"Remove node\" in the context menu.</p>"
      << "<h2>Connect nodes</h2>"
      << "<p>Connect nodes by holding down the ctrl key and dragging a connection to the node.</p>"
      << "<h2>Remove connections</h2>"
      << "<p>There are two methods available:<br>"
      << "1. Select the receiver and in the node settings panel press the \"Clear\" button.<br>"
      << "2. Select the connection line in the scene view and either press the keyboard's delete key "
      << "or press the \"Disconnect\" button in the connection info panel.</p>"
      << "<h2>Export images</h2>"
      << "<p>Right click on a node and select \"Save selected image\".</p>"
      << "<h2>Graph</h2>"
      << "<p>Zoom in and out in the scene view by holding down the alt or shift keys and "
      << "scrolling the mouse wheel.<br>"
      << "Select \"Reset scene view\" or \"Reset zoom\" to restore the view to the default settings.</p>"
      << "<h2>Adding Javascript</h2>"
      << "<p>Open the settings panel and set the directory for the generators.<br>"
      << "For info about how the scripts should look, see github.com/johanokl/ProceduralTextureMaker.</p>";

   auto* dialog = new QDialog(this);
   auto* help = new QTextEdit;
   help->setReadOnly(true);
   help->setText(helpText);
   help->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   QCheckBox* displayOnStart = new QCheckBox("Show on startup");
   displayOnStart->setChecked(QSettings().value("showhelpstartup", true).toBool());
   QObject::connect(displayOnStart, &QCheckBox::toggled,
                    [=](bool val) {
      QSettings().setValue("showhelpstartup", val);
   });
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

/**
 * @brief MainWindow::clearScene
 */
void MainWindow::clearScene()
{
   if (maybeSave()) {
      project->clear();
   }
}
