/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QSettings>
#include <QFileDialog>
#include "texgenapplication.h"
#include "mainwindow.h"
#include "gui/menuactions.h"
#include "gui/addnodepanel.h"
#include "gui/previewimagepanel.h"
#include "gui/settingspanel.h"
#include "gui/iteminfopanel.h"

/**
 * @brief MenuActions::MenuActions
 * @param parent
 */
MenuActions::MenuActions(MainWindow* parent)
   : parentwindow(parent)
{
   newAct = new QAction(QIcon(":/newicon.png"), "&New", parent);
   newAct->setShortcuts(QKeySequence::New);
   QObject::connect(newAct, &QAction::triggered,
                    parent->parent(), &TexGenApplication::addWindow);

   openAct = new QAction(QIcon(":/openicon.png"), "&Open", parent);
   openAct->setStatusTip("Open an existing file");
   openAct->setShortcuts(QKeySequence::Open);
   QObject::connect(openAct, &QAction::triggered, this,
                    &MenuActions::openFile);

   saveAct = new QAction(QIcon(":/saveicon.png"), "&Save", parent);
   saveAct->setStatusTip("Save the document to disk");
   saveAct->setShortcuts(QKeySequence::Save);
   QObject::connect(saveAct, &QAction::triggered,
                    parent, &MainWindow::saveFile);

   saveAsAct = new QAction("Save as", parent);
   saveAsAct->setStatusTip("Save the document under a new name");
   saveAsAct->setShortcuts(QKeySequence::SaveAs);
   QObject::connect(saveAsAct, &QAction::triggered,
                    parent, &MainWindow::saveAs);

   saveImageAct = new QAction("Save selected image", parent);
   QObject::connect(saveImageAct, &QAction::triggered,
                    parent, &MainWindow::saveImage);

   closeAct = new QAction("Close window", parent);
   closeAct->setShortcut(QKeySequence::Close);
   closeAct->setStatusTip("Close the window");
   QObject::connect(closeAct, &QAction::triggered,
                    parent, &MainWindow::close);


   exitAct = new QAction("Q&uit application", parent);
   exitAct->setShortcut(QKeySequence::Quit);
   QObject::connect(exitAct, &QAction::triggered,
                    parent->parent(), &TexGenApplication::quit);

   clearAct = new QAction("Cl&ear scene", parent);
   clearAct->setToolTip("Clear screen");
   QObject::connect(clearAct, &QAction::triggered,
                    parent, &MainWindow::clearScene);

   copyAct = new QAction(QIcon(":/copy.png"), "&Copy", parent);
   copyAct->setStatusTip("Copy the current selection's contents to the clipboard");
   copyAct->setShortcuts(QKeySequence::Copy);
   QObject::connect(copyAct, &QAction::triggered,
                    parent, &MainWindow::copyNode);

   cutAct = new QAction(QIcon(":/cut.png"), "&Cut", parent);
   cutAct->setStatusTip("Cut the current selection's contents to the clipboard");
   cutAct->setShortcuts(QKeySequence::Cut);
   QObject::connect(cutAct, &QAction::triggered,
                    parent, &MainWindow::cutNode);

   pasteAct = new QAction(QIcon(":/paste.png"), "&Paste", parent);
   pasteAct->setStatusTip("Paste the clipboard's contents into the current selection");
   pasteAct->setShortcuts(QKeySequence::Paste);
   QObject::connect(pasteAct, &QAction::triggered,
                    parent, &MainWindow::pasteNode);

   helpAct = new QAction("&Help", parent);
   helpAct->setStatusTip("Show a help popup");
   QObject::connect(helpAct, &QAction::triggered,
                    parent, &MainWindow::showHelp);
   aboutAct = new QAction("&About", parent);
   aboutAct->setStatusTip("Show the application's About popup");
   QObject::connect(aboutAct, &QAction::triggered,
                    parent, &MainWindow::showAbout);

   resetSceneAct = new QAction("Reset scene view", parent);
   QObject::connect(resetSceneAct, &QAction::triggered,
                    parent, &MainWindow::reloadSceneView);
   resetZoomAct = new QAction("Reset zoom", parent);
   QObject::connect(resetZoomAct, &QAction::triggered,
                    parent, &MainWindow::resetViewZoom);

   toggleAddNodePanelAct = new QAction("Toggle add node panel", this);
   QObject::connect(toggleAddNodePanelAct, &QAction::triggered,
                    this, &MenuActions::toggleAddNodePanel);
   displayAddNodePanelAct = new QAction("Display add node panel", this);
   QObject::connect(displayAddNodePanelAct, &QAction::triggered,
                    this, &MenuActions::toggleAddNodePanel);
   displayAddNodePanelAct->setCheckable(true);
   displayAddNodePanelAct->setEnabled(false);

   togglePreviewImagePanelAct = new QAction("Toggle preview image panel", this);
   QObject::connect(togglePreviewImagePanelAct, &QAction::triggered,
                    this, &MenuActions::togglePreviewImagePanel);
   displayPreviewImagePanelAct = new QAction("Display preview image panel", this);
   QObject::connect(displayPreviewImagePanelAct, &QAction::triggered,
                    this, &MenuActions::togglePreviewImagePanel);
   displayPreviewImagePanelAct->setCheckable(true);
   displayPreviewImagePanelAct->setEnabled(false);

   displaySettingsPanelAct = new QAction("Display settings", this);
   QObject::connect(displaySettingsPanelAct, &QAction::triggered,
                    this, &MenuActions::toggleSettingsPanel);
   displaySettingsPanelAct->setCheckable(true);
   displaySettingsPanelAct->setEnabled(false);

   displayToolbarsAct = new QAction("Display toolbars", this);
   QObject::connect(displayToolbarsAct, &QAction::triggered,
                    this, &MenuActions::toogleToolbars);
   displayToolbarsAct->setCheckable(true);
   displayToolbarsAct->setEnabled(true);

   toggleItemInfoPanelAct = new QAction("Toggle item info panel", this);
   QObject::connect(toggleItemInfoPanelAct, &QAction::triggered,
                    this, &MenuActions::toggleItemInfoPanel);
   displayItemInfoPanelAct = new QAction("Display item info panel", this);
   QObject::connect(displayItemInfoPanelAct, &QAction::triggered,
                    this, &MenuActions::toggleItemInfoPanel);
   displayItemInfoPanelAct->setCheckable(true);
   displayItemInfoPanelAct->setEnabled(false);

   fileMenu = parent->menuBar()->addMenu("&File");
   editMenu = parent->menuBar()->addMenu("&Edit");
   viewMenu = parent->menuBar()->addMenu("&View");
   helpMenu = parent->menuBar()->addMenu("&Help");

   viewMenu->addAction(resetSceneAct);
   viewMenu->addAction(resetZoomAct);
   viewMenu->addSeparator();
   viewMenu->addAction(displayAddNodePanelAct);
   viewMenu->addAction(displayToolbarsAct);
   viewMenu->addAction(displayPreviewImagePanelAct);
   viewMenu->addAction(displaySettingsPanelAct);
   viewMenu->addAction(displayItemInfoPanelAct);
   viewMenu->addSeparator();

   fileMenu->addAction(newAct);
   fileMenu->addAction(openAct);
   fileMenu->addAction(saveAct);
   fileMenu->addAction(saveAsAct);
   fileMenu->addSeparator();
   fileMenu->addAction(saveImageAct);
   fileMenu->addSeparator();
   fileMenu->addAction(closeAct);
   fileMenu->addAction(exitAct);

   editMenu->addAction(clearAct);
   editMenu->addSeparator();
   editMenu->addAction(copyAct);
   editMenu->addAction(cutAct);
   editMenu->addAction(pasteAct);

   helpMenu->addAction(helpAct);
   helpMenu->addAction(aboutAct);

   settingsToolBar = parent->addToolBar("Settings");
   fileToolBar = parent->addToolBar("File");
   editToolBar = parent->addToolBar("Edit");
   insertToolBar = parent->addToolBar("Insert");
   toggleAddNodeToolBar = parent->addToolBar("Add nodes");
   settingsToolBar->setIconSize(QSize(15, 15));
   fileToolBar->setIconSize(QSize(16, 16));
   editToolBar->setIconSize(QSize(16, 16));
   insertToolBar->setIconSize(QSize(16, 16));
   toggleAddNodeToolBar->setIconSize(QSize(16, 16));

   settingsToolBar->addAction(toggleItemInfoPanelAct);
   fileToolBar->addAction(newAct);
   fileToolBar->addAction(openAct);
   fileToolBar->addAction(saveAct);

   editToolBar->addAction(copyAct);
   editToolBar->addAction(cutAct);
   editToolBar->addAction(pasteAct);

   QWidget* spacerWidget = new QWidget();
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   spacerWidget->setVisible(true);
   toggleAddNodeToolBar->addWidget(spacerWidget);
   toggleAddNodeToolBar->addAction(toggleAddNodePanelAct);
   toggleAddNodeToolBar->addAction(togglePreviewImagePanelAct);

   QObject::connect(parent->parent(), &TexGenApplication::windowsChanged,
                    this, &MenuActions::windowsChanged);

   QSettings settings;
   lastOpenedDirectory = settings.value("lastopeneddirectory", QDir::homePath()).toString();
   displayAddNodePanelAct->setChecked(settings.value("displayAddNodePanel", true).toBool());
   displayPreviewImagePanelAct->setChecked(settings.value("displayPreviewImagePanel", false).toBool());
   displayItemInfoPanelAct->setChecked(settings.value("displayItemInfoPanel", true).toBool());
   displaySettingsPanelAct->setChecked(settings.value("displaySettingsPanel", false).toBool());
   displayToolbarsAct->setChecked(settings.value("displayToolbars", true).toBool());

   if (!displayToolbarsAct->isChecked()) {
      toogleToolbars();
   }
}

/**
 * @brief MenuActions::~MenuActions
 */
MenuActions::~MenuActions()
{
   parent()->parent()->disconnect(this);
   // Save state
   QSettings settings;
   settings.setValue("displayAddNodePanel", displayAddNodePanelAct->isChecked());
   settings.setValue("displayPreviewImagePanel", displayPreviewImagePanelAct->isChecked());
   settings.setValue("displayItemInfoPanel", displayItemInfoPanelAct->isChecked());
   settings.setValue("displaySettingsPanel", displaySettingsPanelAct->isChecked());
   settings.setValue("displayToolbars", displayToolbarsAct->isChecked());
   settings.setValue("lastopeneddirectory", lastOpenedDirectory);
   settings.sync();
}

/**
 * @brief MenuActions::setAddNodePanel
 * @param addNodePanel
 * Connects a new add node panel and sets its visiblity.
 */
void MenuActions::setAddNodePanel(AddNodePanel* addNodePanel)
{
   this->nodepanel = addNodePanel;
   displayAddNodePanelAct->setEnabled(true);
   addNodePanel->setVisible(displayAddNodePanelAct->isChecked());
}

/**
 * @brief MenuActions::setPreviewImagePanel
 * @param previewImagePanel
 * Connects a new node preview panel and sets its visiblity.
 */
void MenuActions::setPreviewImagePanel(PreviewImagePanel* previewImagePanel)
{
   this->previewImagePanel = previewImagePanel;
   displayPreviewImagePanelAct->setEnabled(true);
   previewImagePanel->setVisible(displayPreviewImagePanelAct->isChecked());
}

/**
 * @brief MenuActions::setSettingsPanel
 * @param settingsPanel
 * Connects a new project settings panel and sets its visiblity.
 */
void MenuActions::setSettingsPanel(SettingsPanel* settingsPanel)
{
   this->settingspanel = settingsPanel;
   displaySettingsPanelAct->setEnabled(true);
   settingsPanel->setVisible(displaySettingsPanelAct->isChecked());
}

/**
 * @brief MenuActions::setItemInfoPanel
 * @param infopanel
 * Connects a new item info panel and sets its visiblity.
 */
void MenuActions::setItemInfoPanel(ItemInfoPanel* infopanel)
{
   this->nodesettings = infopanel;
   displayItemInfoPanelAct->setEnabled(true);
   nodesettings->setVisible(displayItemInfoPanelAct->isChecked());
}

/**
 * @brief MenuActions::toggleAddNodePanel
 * Displays or hides the add node panel depending on if visible or hidden.
 */
void MenuActions::toggleAddNodePanel()
{
   if (!nodepanel) {
      return;
   }
   nodepanel->setVisible(!nodepanel->isVisible());
   displayAddNodePanelAct->setChecked(nodepanel->isVisible());
}

/**
 * @brief MenuActions::togglePreviewImagePanel
 * Displays or hides the image preview panel depending on if visible or hidden.
 */
void MenuActions::togglePreviewImagePanel()
{
   if (!previewImagePanel) {
      return;
   }
   previewImagePanel->setVisible(!previewImagePanel->isVisible());
   displayPreviewImagePanelAct->setChecked(previewImagePanel->isVisible());
}

/**
 * @brief MenuActions::toggleSettingsPanel
 */
void MenuActions::toggleSettingsPanel()
{
   if (!settingspanel) {
      return;
   }
   settingspanel->setVisible(!settingspanel->isVisible());
   displaySettingsPanelAct->setChecked(settingspanel->isVisible());
}

/**
 * @brief MenuActions::toggleItemInfoPanel
 * Displays or hides the item info panel depending on if visible or hidden.
 */
void MenuActions::toggleItemInfoPanel()
{
   if (!nodesettings) {
      return;
   }
   nodesettings->setVisible(!nodesettings->isVisible());
   displayItemInfoPanelAct->setChecked(nodesettings->isVisible());
}

/**
 * @brief MenuActions::toogleToolbars
 * Displays or hides all toolbars depending on if visible or hidden.
 */
void MenuActions::toogleToolbars()
{
   if (settingsToolBar) {
      settingsToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (fileToolBar) {
      fileToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (editToolBar) {
      editToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (insertToolBar) {
      insertToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (toggleAddNodeToolBar) {
      toggleAddNodeToolBar->setVisible(displayToolbarsAct->isChecked());
   }
}

/**
 * @brief MenuActions::openFile
 *
 * Displays an open file dialog and opens the chosen project file.
 */
void MenuActions::openFile()
{
   QString fileName = QFileDialog::getOpenFileName(parent(), tr("Open File"), lastOpenedDirectory, "Texture Set (*.txl)");
   if (fileName.isNull()) {
      return;
   }
   lastOpenedDirectory = QFileInfo(fileName).path();
   parent()->openFile(fileName);
}

/**
 * @brief MenuActions::windowsChanged
 *
 * Slot invoked when the window manager has closed a window.
 * Updates the list of other windows in the window menu.
 */
void MenuActions::windowsChanged()
{
   while(!windowlistActions.isEmpty()) {
      QAction* currAction = windowlistActions.last();
      windowlistActions.remove(windowlistActions.size() - 1);
      viewMenu->removeAction(currAction);
      delete currAction;
   }
   for (int i=0; i < parent()->parent()->mainwindows.count(); i++) {
      MainWindow* currWindow = parent()->parent()->mainwindows[i];
      QString title = currWindow->windowTitle();
      if (title == "ProceduralTextureMaker") {
         title = "Untitled project";
      }
#ifndef Q_OS_MAC
      else {
         title.remove(title.length() - QString(" - ProceduralTextureMaker").length(), title.length() + 1);
      }
#endif
      QAction* newAction = new QAction(title, parent());
      if (i < 10) {
         newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1 + i));
      }
      QObject::connect(newAction, &QAction::triggered,
                       currWindow, &MainWindow::moveToFront);
      viewMenu->addAction(newAction);
      windowlistActions.push_back(newAction);
   }
}
