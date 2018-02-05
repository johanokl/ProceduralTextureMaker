/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <stdio.h>
#include <math.h>
#include <QPainter>
#include <QLibrary>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QIcon>
#include <QFile>
#include <QMessageBox>
#include <QSharedPointer>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QAction>
#include <QStatusBar>
#include <QTextEdit>
#include <QSplitter>
#include <QGraphicsItemGroup>
#include "texgenapplication.h"
#include "core/textureproject.h"
#include "core/texturenode.h"
#include "core/settingsmanager.h"
#include "global.h"
#include "core/textureimage.h"
#include "generators/javascript.h"
#include "generators/fill.h"
#include "generators/circle.h"
#include "generators/perlinnoise.h"
#include "generators/blending.h"
#include "generators/lines.h"
#include "generators/bricks.h"
#include "generators/sinetransform.h"
#include "generators/setchannels.h"
#include "generators/mirror.h"
#include "generators/boxblur.h"
#include "generators/invert.h"
#include "generators/whirl.h"
#include "generators/gaussianblur.h"
#include "generators/sineplasma.h"
#include "generators/gradient.h"
#include "generators/normalmap.h"
#include "generators/square.h"
#include "generators/greyscale.h"
#include "generators/star.h"
#include "generators/transform.h"
#include "generators/modifyalpha.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodescene.h"
#include "gui/addnodepanel.h"
#include "gui/previewimagepanel.h"
#include "gui/preview3dpanel.h"
#include "gui/settingspanel.h"
#include "sceneview/viewnodeview.h"
#include "gui/iteminfopanel.h"
#include "gui/menuactions.h"
#include "mainwindow.h"

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

   connect(project, SIGNAL(generatorNameCollision(TextureGeneratorPtr,TextureGeneratorPtr)),
           this, SLOT(generatorNameCollision(TextureGeneratorPtr,TextureGeneratorPtr)));

   iteminfopanel = new ItemInfoPanel(this, project);
   iteminfopanel->hide();
   iteminfopanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   iteminfopanel->setMaximumWidth(500);
   iteminfopanel->setMinimumWidth(300);

   menuactions = new MenuActions(this);
   addnodewidget = new AddNodePanel(project);
   previewimagewidget = new PreviewImagePanel(project);
   preview3dwidget = new Preview3dPanel(project);
   settingspanel = new SettingsPanel(this, settingsManager);
   addnodewidget->hide();
   previewimagewidget->hide();
   preview3dwidget->hide();
   settingspanel->hide();

   view = new ViewNodeView(settingsManager->getDefaultZoom());
   view->show();
   scene = createScene();

   project->addGenerator(TextureGeneratorPtr(new FillTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new CircleTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new PerlinNoiseTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new BlendingTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new InvertTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new BricksTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new WhirlTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SetChannelsTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GreyscaleTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SquareTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SineTransformTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new LinesTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GradientTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new StarTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new TransformTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new ModifyAlphaTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new BoxBlurTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new MirrorTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new GaussianBlurTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new SinePlasmaTextureGenerator()));
   project->addGenerator(TextureGeneratorPtr(new NormalMapTextureGenerator()));
   project->clear();

   jstexgenManager = new JSTexGenManager(project);

   QVBoxLayout* centerLayout = new QVBoxLayout;
   centerLayout->addWidget(view);

   QSplitter* widget = new QSplitter(this);
   widget->addWidget(iteminfopanel);
   widget->addWidget(view);
   widget->addWidget(addnodewidget);
   widget->addWidget(previewimagewidget);
   widget->addWidget(preview3dwidget);
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
   menuactions->setPreview3dPanel(preview3dwidget);
   menuactions->setSettingsPanel(settingspanel);
   menuactions->setItemInfoPanel(iteminfopanel);

   setWindowTitle("ProceduralTextureMaker");
   statusBar()->hide();
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
   parentapp->removeWindow(this);
   delete this;
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
void MainWindow::generatorNameCollision(TextureGeneratorPtr oldGen, TextureGeneratorPtr newGen)
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
      msgBox.setInformativeText("Saving file...");
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
void MainWindow::saveImage()
{
   TextureNodePtr texNode = project->getNode(scene->getSelectedNode());
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
      msgBox.setInformativeText("Saving file...");
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
          (outSize.width() * outSize.height() * sizeof(TexturePixel)));
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
   if (source) {
      newscene = scene->clone();
   } else {
      newscene = new ViewNodeScene(this);
   }
   view->setScene(newscene);
   connect(newscene, SIGNAL(nodeSelected(int)), iteminfopanel, SLOT(setActiveNode(int)));
   connect(newscene, SIGNAL(nodeSelected(int)), previewimagewidget, SLOT(setActiveNode(int)));
   connect(newscene, SIGNAL(nodeSelected(int)), preview3dwidget, SLOT(setActiveNode(int)));
   connect(newscene, SIGNAL(lineSelected(int, int, int)), iteminfopanel, SLOT(setActiveLine(int, int, int)));
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
void MainWindow::openFile(QString fileName)
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
   this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

/**
 * @brief MainWindow::showAbout
 */
void MainWindow::showAbout()
{
   QMessageBox::about(this, "About",
                      QString("<p align='center'>")
                      .append("<h2>ProceduralTextureMaker</h2>")
                      .append("<hr> <br>")
                      .append("Johan Lindqvist <br>")
                      .append("<a href='mailto:johan.lindqvist@gmail.com'>johan.lindqvist@gmail.com</a>")
                      .append("<p>")
                      .append("<p align='center'>")
                      .append("More information at<br>")
                      .append("<a href='http://github.com/johanokl/ProceduralTextureMaker'>")
                      .append("http://github.com/johanokl/ProceduralTextureMaker")
                      .append("</a><br></p>")
                      .append("This version built: %1 %2")
                      .arg(__DATE__).arg(__TIME__));
}

/**
 * @brief MainWindow::showHelp
 * Displays a help popup
 */
void MainWindow::showHelp()
{
   QTextEdit* help = new QTextEdit;
   help->setWindowFlag(Qt::Tool);
   help->setWindowTitle("Help");
   help->setReadOnly(true);
   help->append("Drag nodes from the button list on the right side to the center workboard.<br/>");
   help->append("Connect nodes by holding down the ctrl key and dragging a connection to the node.<br/>");
   help->append("Zoom in and out by holding down the alt key and scrolling the mouse wheel.<br/>");
   help->show();
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
