
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/addnodepanel.h"
#include "gui/iteminfopanel.h"
#include "gui/menuactions.h"
#include "gui/previewimagepanel.h"
#include "gui/settingspanel.h"
#include "mainwindow.h"
#include "texgenapplication.h"
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QToolBar>

/// @brief Constructor for the MenuActions class. Creates the main window's menus and toolbars.
/// @param parent
MenuActions::MenuActions(MainWindow* parent) : QObject(parent), parentwindow(parent) {
   nodepanel = nullptr;
   previewImagePanel = nullptr;
   nodesettings = nullptr;
   settingspanel = nullptr;
   displayPreview3dPanelAct = nullptr;
   togglePreview3dPanelAct = nullptr;

   newAct = new QAction(QIcon(":/newicon.png"), "&New", parent);
   newAct->setShortcuts(QKeySequence::New);
   QObject::connect(newAct, &QAction::triggered, parent->parent(), &TexGenApplication::addWindow);

   openAct = new QAction(QIcon(":/openicon.png"), "&Open", parent);
   openAct->setStatusTip("Open an existing file");
   openAct->setShortcuts(QKeySequence::Open);
   QObject::connect(openAct, &QAction::triggered, this, &MenuActions::openFile);

   saveAct = new QAction(QIcon(":/saveicon.png"), "&Save", parent);
   saveAct->setStatusTip("Save the document to disk");
   saveAct->setShortcuts(QKeySequence::Save);
   QObject::connect(saveAct, &QAction::triggered, parent, &MainWindow::saveFile);

   saveAsAct = new QAction("Save as", parent);
   saveAsAct->setStatusTip("Save the document under a new name");
   saveAsAct->setShortcuts(QKeySequence::SaveAs);
   QObject::connect(saveAsAct, &QAction::triggered, parent, &MainWindow::saveAs);

   saveImageAct = new QAction("Save selected image", parent);
   QObject::connect(saveImageAct, &QAction::triggered, parent, &MainWindow::saveImage);

   closeAct = new QAction("Close window", parent);
   closeAct->setShortcut(QKeySequence::Close);
   closeAct->setStatusTip("Close the window");
   QObject::connect(closeAct, &QAction::triggered, parent, &MainWindow::close);

   exitAct = new QAction("Q&uit application", parent);
   exitAct->setShortcut(QKeySequence::Quit);
   QObject::connect(exitAct, &QAction::triggered, parent->parent(),
                    &TexGenApplication::quitApplication);

   clearAct = new QAction("Cl&ear scene", parent);
   clearAct->setToolTip("Clear screen");
   QObject::connect(clearAct, &QAction::triggered, parent, &MainWindow::clearScene);

   copyAct = new QAction(QIcon(":/copy.png"), "&Copy", parent);
   copyAct->setStatusTip("Copy the current selection's contents to the clipboard");
   copyAct->setShortcuts(QKeySequence::Copy);
   QObject::connect(copyAct, &QAction::triggered, parent, &MainWindow::copyNode);

   cutAct = new QAction(QIcon(":/cut.png"), "&Cut", parent);
   cutAct->setStatusTip("Cut the current selection's contents to the clipboard");
   cutAct->setShortcuts(QKeySequence::Cut);
   QObject::connect(cutAct, &QAction::triggered, parent, &MainWindow::cutNode);

   pasteAct = new QAction(QIcon(":/paste.png"), "&Paste", parent);
   pasteAct->setStatusTip("Paste the clipboard's contents into the current selection");
   pasteAct->setShortcuts(QKeySequence::Paste);
   QObject::connect(pasteAct, &QAction::triggered, parent, &MainWindow::pasteNode);

   zoomInAct = new QAction(QIcon::fromTheme("zoom-in"), "Zoom in", parent);
   zoomInAct->setIconText("+");
   zoomInAct->setToolTip("Zoom in");
   zoomInAct->setStatusTip("Zoom in on the node scene");
   zoomInAct->setShortcuts(QKeySequence::ZoomIn);
   QObject::connect(zoomInAct, &QAction::triggered, parent, &MainWindow::zoomInView);

   zoomOutAct = new QAction(QIcon::fromTheme("zoom-out"), "Zoom out", parent);
   zoomOutAct->setIconText("-");
   zoomOutAct->setToolTip("Zoom out");
   zoomOutAct->setStatusTip("Zoom out from the node scene");
   zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
   QObject::connect(zoomOutAct, &QAction::triggered, parent, &MainWindow::zoomOutView);

   helpAct = new QAction("&Help", parent);
   helpAct->setStatusTip("Show a help popup");
   QObject::connect(helpAct, &QAction::triggered, parent, &MainWindow::showHelp);
   aboutAct = new QAction("&About", parent);
   aboutAct->setStatusTip("Show the application's About popup");
   QObject::connect(aboutAct, &QAction::triggered, parent, &MainWindow::showAbout);

   showAllNodesAct = new QAction(QIcon::fromTheme("zoom-fit-best"), "Show all nodes", parent);
   showAllNodesAct->setIconText("All");
   showAllNodesAct->setToolTip("Show all nodes");
   showAllNodesAct->setStatusTip("Show every node in the scene view");
   QObject::connect(showAllNodesAct, &QAction::triggered, parent,
                    &MainWindow::showAllNodesAndResetSceneView);
   resetZoomAct = new QAction(QIcon::fromTheme("zoom-original"), "Reset zoom", parent);
   resetZoomAct->setIconText("100%");
   resetZoomAct->setToolTip("Reset zoom to 100%");
   QObject::connect(resetZoomAct, &QAction::triggered, parent, &MainWindow::resetViewZoom);

   toggleAddNodePanelAct = new QAction("Nodes", this);
   toggleAddNodePanelAct->setToolTip("Show or hide the node palette");
   QObject::connect(toggleAddNodePanelAct, &QAction::triggered, this,
                    &MenuActions::toggleAddNodePanel);
   displayAddNodePanelAct = new QAction("Display add node panel", this);
   QObject::connect(displayAddNodePanelAct, &QAction::triggered, this,
                    &MenuActions::toggleAddNodePanel);
   displayAddNodePanelAct->setCheckable(true);
   displayAddNodePanelAct->setEnabled(false);

   togglePreviewImagePanelAct = new QAction("Preview", this);
   togglePreviewImagePanelAct->setToolTip("Show or hide the image preview panel");
   QObject::connect(togglePreviewImagePanelAct, &QAction::triggered, this,
                    &MenuActions::togglePreviewImagePanel);
   displayPreviewImagePanelAct = new QAction("Display preview image panel", this);
   QObject::connect(displayPreviewImagePanelAct, &QAction::triggered, this,
                    &MenuActions::togglePreviewImagePanel);
   displayPreviewImagePanelAct->setCheckable(true);
   displayPreviewImagePanelAct->setEnabled(false);

   toggleSettingsPanelAct = new QAction("Settings", this);
   toggleSettingsPanelAct->setToolTip("Show or hide the settings panel");
   toggleSettingsPanelAct->setCheckable(true);
   QObject::connect(toggleSettingsPanelAct, &QAction::triggered, this,
                    &MenuActions::toggleSettingsPanel);
   displaySettingsPanelAct = new QAction("Display settings", this);
   QObject::connect(displaySettingsPanelAct, &QAction::triggered, this,
                    &MenuActions::toggleSettingsPanel);
   displaySettingsPanelAct->setCheckable(true);
   displaySettingsPanelAct->setEnabled(false);

   displayToolbarsAct = new QAction("Display toolbars", this);
   QObject::connect(displayToolbarsAct, &QAction::triggered, this, &MenuActions::toogleToolbars);
   displayToolbarsAct->setCheckable(true);
   displayToolbarsAct->setEnabled(true);

   toggleItemInfoPanelAct = new QAction("Info", this);
   toggleItemInfoPanelAct->setToolTip("Show or hide the item info panel");
   QObject::connect(toggleItemInfoPanelAct, &QAction::triggered, this,
                    &MenuActions::toggleItemInfoPanel);
   displayItemInfoPanelAct = new QAction("Display item info panel", this);
   QObject::connect(displayItemInfoPanelAct, &QAction::triggered, this,
                    &MenuActions::toggleItemInfoPanel);
   displayItemInfoPanelAct->setCheckable(true);
   displayItemInfoPanelAct->setEnabled(false);

   fileMenu = parent->menuBar()->addMenu("&File");
   editMenu = parent->menuBar()->addMenu("&Edit");
   viewMenu = parent->menuBar()->addMenu("&View");
   helpMenu = parent->menuBar()->addMenu("&Help");

   viewMenu->addAction(showAllNodesAct);
   viewMenu->addAction(resetZoomAct);
   viewMenu->addAction(zoomInAct);
   viewMenu->addAction(zoomOutAct);
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
   settingsToolBar->setIconSize(QSize(18, 18));
   fileToolBar->setIconSize(QSize(18, 18));
   editToolBar->setIconSize(QSize(18, 18));
   insertToolBar->setIconSize(QSize(18, 18));
   toggleAddNodeToolBar->setIconSize(QSize(18, 18));
   settingsToolBar->setMovable(false);
   fileToolBar->setMovable(false);
   editToolBar->setMovable(false);
   insertToolBar->setMovable(false);
   toggleAddNodeToolBar->setMovable(false);
   settingsToolBar->setFloatable(false);
   fileToolBar->setFloatable(false);
   editToolBar->setFloatable(false);
   insertToolBar->setFloatable(false);
   toggleAddNodeToolBar->setFloatable(false);

   settingsToolBar->addAction(toggleItemInfoPanelAct);
   fileToolBar->addAction(newAct);
   fileToolBar->addAction(openAct);
   fileToolBar->addAction(saveAct);

   editToolBar->addAction(copyAct);
   editToolBar->addAction(cutAct);
   editToolBar->addAction(pasteAct);
   editToolBar->addSeparator();
   editToolBar->addAction(zoomInAct);
   editToolBar->addAction(zoomOutAct);
   editToolBar->addAction(resetZoomAct);
   editToolBar->addAction(showAllNodesAct);

   QWidget* spacerWidget = new QWidget();
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   spacerWidget->setVisible(true);
   toggleAddNodeToolBar->addWidget(spacerWidget);
   toggleAddNodeToolBar->addAction(toggleAddNodePanelAct);
   toggleAddNodeToolBar->addAction(togglePreviewImagePanelAct);
   toggleAddNodeToolBar->addAction(toggleSettingsPanelAct);

   QObject::connect(parent->parent(), &TexGenApplication::windowsChanged, this,
                    &MenuActions::windowsChanged);

   QSettings settings;
   lastOpenedDirectory = settings.value("lastopeneddirectory", QDir::homePath()).toString();
   displayAddNodePanelAct->setChecked(settings.value("displayAddNodePanel", true).toBool());
   displayPreviewImagePanelAct->setChecked(
       settings.value("displayPreviewImagePanel", false).toBool());
   displayItemInfoPanelAct->setChecked(settings.value("displayItemInfoPanel", true).toBool());
   displaySettingsPanelAct->setChecked(settings.value("displaySettingsPanel", false).toBool());
   displayToolbarsAct->setChecked(settings.value("displayToolbars", true).toBool());

   if (!displayToolbarsAct->isChecked()) {
      toogleToolbars();
   }
}

/// @brief Destructor for the MenuActions class.
/// @details Saves the state of the main window's menus and toolbars.
MenuActions::~MenuActions() {
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

/// @brief Connects a new add node panel and sets its visiblity.
/// @param addNodePanel
void MenuActions::setAddNodePanel(AddNodePanel* addNodePanel) {
   this->nodepanel = addNodePanel;
   displayAddNodePanelAct->setEnabled(true);
   addNodePanel->setVisible(displayAddNodePanelAct->isChecked());
}

/// @brief Connects a new node preview panel and sets its visiblity.
/// @param previewImagePanel
void MenuActions::setPreviewImagePanel(PreviewImagePanel* previewImagePanel) {
   this->previewImagePanel = previewImagePanel;
   displayPreviewImagePanelAct->setEnabled(true);
   previewImagePanel->setVisible(displayPreviewImagePanelAct->isChecked());
}

/// @brief Connects a new project settings panel and sets its visiblity.
/// @param settingsPanel
void MenuActions::setSettingsPanel(SettingsPanel* settingsPanel) {
   this->settingspanel = settingsPanel;
   displaySettingsPanelAct->setEnabled(true);
   settingsPanel->setVisible(displaySettingsPanelAct->isChecked());
   toggleSettingsPanelAct->setChecked(settingsPanel->isVisible());
}

/// @brief Connects a new item info panel and sets its visiblity.
/// @param infopanel
void MenuActions::setItemInfoPanel(ItemInfoPanel* infopanel) {
   this->nodesettings = infopanel;
   displayItemInfoPanelAct->setEnabled(true);
   nodesettings->setVisible(displayItemInfoPanelAct->isChecked());
}

/// @brief Displays or hides the add node panel depending on if visible or hidden.
void MenuActions::toggleAddNodePanel() {
   if (!nodepanel) {
      return;
   }
   nodepanel->setVisible(!nodepanel->isVisible());
   displayAddNodePanelAct->setChecked(nodepanel->isVisible());
}

/// @brief Displays or hides the image preview panel depending on if visible or hidden.
void MenuActions::togglePreviewImagePanel() {
   if (previewImagePanel != nullptr) {
      previewImagePanel->setVisible(!previewImagePanel->isVisible());
      displayPreviewImagePanelAct->setChecked(previewImagePanel->isVisible());
   }
}

/// @brief MenuActions::toggleSettingsPanel
void MenuActions::toggleSettingsPanel() {
   if (settingspanel != nullptr) {
      settingspanel->setVisible(!settingspanel->isVisible());
      displaySettingsPanelAct->setChecked(settingspanel->isVisible());
      toggleSettingsPanelAct->setChecked(settingspanel->isVisible());
   }
}

/// @brief Displays or hides the item info panel depending on if visible or hidden.
void MenuActions::toggleItemInfoPanel() {
   if (nodesettings != nullptr) {
      nodesettings->setVisible(!nodesettings->isVisible());
      displayItemInfoPanelAct->setChecked(nodesettings->isVisible());
   }
}

/// @brief Displays or hides all toolbars depending on if visible or hidden.
void MenuActions::toogleToolbars() {
   if (settingsToolBar != nullptr) {
      settingsToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (fileToolBar != nullptr) {
      fileToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (editToolBar != nullptr) {
      editToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (insertToolBar != nullptr) {
      insertToolBar->setVisible(displayToolbarsAct->isChecked());
   }
   if (toggleAddNodeToolBar != nullptr) {
      toggleAddNodeToolBar->setVisible(displayToolbarsAct->isChecked());
   }
}

/// @brief Displays an open file dialog and opens the chosen project file.
void MenuActions::openFile() {
   QString fileName = QFileDialog::getOpenFileName(parent(), tr("Open File"), lastOpenedDirectory,
                                                   "Texture Set (*.txl)");
   if (fileName.isNull()) {
      return;
   }
   lastOpenedDirectory = QFileInfo(fileName).path();
   parent()->openFile(fileName);
}

/// @brief Slot invoked when the window manager has closed a window.
/// @details Updates the list of other windows in the window menu.
void MenuActions::windowsChanged() {
   while (!windowlistActions.isEmpty()) {
      QAction* currAction = windowlistActions.last();
      windowlistActions.remove(windowlistActions.size() - 1);
      viewMenu->removeAction(currAction);
      delete currAction;
   }
   for (int i = 0; i < parent()->parent()->mainwindows.count(); i++) {
      MainWindow* currWindow = parent()->parent()->mainwindows[i];
      QString title = currWindow->windowTitle();
      if (title == "ProceduralTextureMaker") {
         title = "Untitled project";
      }
#ifndef Q_OS_MAC
      else {
         title.remove(title.length() - QString(" - ProceduralTextureMaker").length(),
                      title.length() + 1);
      }
#endif
      auto* newAction = new QAction(title, parent());
      if (i < 10) {
         newAction->setShortcut(
             QKeySequence(Qt::ControlModifier | static_cast<Qt::Key>(Qt::Key_1 + i)));
      }
      QObject::connect(newAction, &QAction::triggered, currWindow, &MainWindow::moveToFront);
      viewMenu->addAction(newAction);
      windowlistActions.push_back(newAction);
   }
}
