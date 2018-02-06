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
   connect(newAct, SIGNAL(triggered()), parent->parent(), SLOT(addWindow()));

   openAct = new QAction(QIcon(":/openicon.png"), "&Open", parent);
   openAct->setStatusTip("Open an existing file");
   openAct->setShortcuts(QKeySequence::Open);
   connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

   saveAct = new QAction(QIcon(":/saveicon.png"), "&Save", parent);
   saveAct->setStatusTip("Save the document to disk");
   saveAct->setShortcuts(QKeySequence::Save);
   connect(saveAct, SIGNAL(triggered()), parent, SLOT(saveFile()));

   saveAsAct = new QAction("Save as", parent);
   saveAsAct->setStatusTip("Save the document under a new name");
   saveAsAct->setShortcuts(QKeySequence::SaveAs);
   connect(saveAsAct, SIGNAL(triggered()), parent, SLOT(saveAs()));

   saveImageAct = new QAction("Save selected image", parent);
   connect(saveImageAct, SIGNAL(triggered()), parent, SLOT(saveImage()));

   exitAct = new QAction("E&xit", parent);
   exitAct->setShortcut(QKeySequence::Quit);
   exitAct->setStatusTip("Exit the application");
   connect(exitAct, SIGNAL(triggered()), parent, SLOT(close()));

   clearAct = new QAction("Cl&ear scene", parent);
   clearAct->setToolTip("Clear screen");
   connect(clearAct, SIGNAL(triggered()), parent, SLOT(clearScene()));

   copyAct = new QAction(QIcon(":/copy.png"), "&Copy", parent);
   copyAct->setStatusTip("Copy the current selection's contents to the clipboard");
   copyAct->setShortcuts(QKeySequence::Copy);
   connect(copyAct, SIGNAL(triggered()), parent, SLOT(copyNode()));

   cutAct = new QAction(QIcon(":/cut.png"), "&Cut", parent);
   cutAct->setStatusTip("Cut the current selection's contents to the clipboard");
   cutAct->setShortcuts(QKeySequence::Cut);
   connect(cutAct, SIGNAL(triggered()), parent, SLOT(cutNode()));

   pasteAct = new QAction(QIcon(":/paste.png"), "&Paste", parent);
   pasteAct->setStatusTip("Paste the clipboard's contents into the current selection");
   pasteAct->setShortcuts(QKeySequence::Paste);
   connect(pasteAct, SIGNAL(triggered()), parent, SLOT(pasteNode()));

   helpAct = new QAction("&Help", parent);
   helpAct->setStatusTip("Show a help popup");
   connect(helpAct, SIGNAL(triggered()), parent, SLOT(showHelp()));
   aboutAct = new QAction("&About", parent);
   aboutAct->setStatusTip("Show the application's About popup");
   connect(aboutAct, SIGNAL(triggered()), parent, SLOT(showAbout()));

   resetSceneAct = new QAction("Reset scene view", parent);
   connect(resetSceneAct, SIGNAL(triggered()), parent, SLOT(reloadSceneView()));
   resetZoomAct = new QAction("Reset zoom", parent);
   connect(resetZoomAct, SIGNAL(triggered()), parent, SLOT(resetViewZoom()));

   toggleAddNodePanelAct = new QAction("Toggle add nodes panel", this);
   connect(toggleAddNodePanelAct, SIGNAL(triggered()), this, SLOT(toggleAddNodePanel()));
   displayAddNodePanelAct = new QAction("Display add nodes panel", this);
   connect(displayAddNodePanelAct, SIGNAL(triggered()), this, SLOT(toggleAddNodePanel()));
   displayAddNodePanelAct->setCheckable(true);
   displayAddNodePanelAct->setEnabled(false);

   togglePreviewImagePanelAct = new QAction("Toggle preview image panel", this);
   connect(togglePreviewImagePanelAct, SIGNAL(triggered()), this, SLOT(togglePreviewImagePanel()));
   displayPreviewImagePanelAct = new QAction("Display preview image panel", this);
   connect(displayPreviewImagePanelAct, SIGNAL(triggered()), this, SLOT(togglePreviewImagePanel()));
   displayPreviewImagePanelAct->setCheckable(true);
   displayPreviewImagePanelAct->setEnabled(false);

   displaySettingsPanelAct = new QAction("Display settings", this);
   connect(displaySettingsPanelAct, SIGNAL(triggered()), this, SLOT(toggleSettingsPanel()));
   displaySettingsPanelAct->setCheckable(true);
   displaySettingsPanelAct->setEnabled(false);

   displayToolbarsAct = new QAction("Display toolbars", this);
   connect(displayToolbarsAct, SIGNAL(triggered()), this, SLOT(toogleToolbars()));
   displayToolbarsAct->setCheckable(true);
   displayToolbarsAct->setEnabled(true);

   toggleItemInfoPanelAct = new QAction("Toggle item info panel", this);
   connect(toggleItemInfoPanelAct, SIGNAL(triggered()), this, SLOT(toggleItemInfoPanel()));
   displayItemInfoPanelAct = new QAction("Display item info panel", this);
   connect(displayItemInfoPanelAct, SIGNAL(triggered()), this, SLOT(toggleItemInfoPanel()));
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

   connect(parent->parent(), SIGNAL(windowsChanged()), this, SLOT(windowsChanged()));

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
      windowlistActions.remove(windowlistActions.size()-1);
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
      connect(newAction, SIGNAL(triggered()), currWindow, SLOT(moveToFront()));
      viewMenu->addAction(newAction);
      windowlistActions.push_back(newAction);
   }
}
