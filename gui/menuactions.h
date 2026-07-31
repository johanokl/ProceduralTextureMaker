
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef MENUACTIONS_H
#define MENUACTIONS_H

#include <QObject>
#include <QVector>
class QMainWindow;
class TextureProject;
class TextureGenerator;
class QMenu;
class QAction;
class MainWindow;
class AddNodePanel;
class PreviewImagePanel;
class Preview3dPanel;
class ItemInfoPanel;
class SettingsPanel;
class QToolBar;

/// @brief Sets up the main window's menus, toolbars, and actions.
/// Upon destruction, panel and toolbar visibility settings are written
/// to persistent storage for the next application session.
class MenuActions : public QObject {
   Q_OBJECT

public:
   /// @brief Creates menus and toolbars for a main window.
   /// @param parentwindow Main window that owns the actions.
   explicit MenuActions(MainWindow* parentwindow);

   /// @brief Returns the main window that owns the actions.
   MainWindow* parent() { return parentwindow; }

   /// @brief Saves menu and toolbar state before destroying the actions.
   ~MenuActions() override;

   /// @brief Connects an add-node panel and restores its visibility.
   /// @param addNodePanel Panel controlled by the corresponding view action.
   void setAddNodePanel(AddNodePanel* addNodePanel);

   /// @brief Connects an image-preview panel and restores its visibility.
   /// @param previewImagePanel Panel controlled by the corresponding view action.
   void setPreviewImagePanel(PreviewImagePanel* previewImagePanel);

   /// @brief Connects a settings panel and restores its visibility.
   /// @param settingsPanel Panel controlled by the corresponding view action.
   void setSettingsPanel(SettingsPanel* settingsPanel);

   /// @brief Connects an item-information panel and restores its visibility.
   /// @param infopanel Panel controlled by the corresponding view action.
   void setItemInfoPanel(ItemInfoPanel* infopanel);

public slots:
   /// @brief Toggles the add-node panel.
   void toggleAddNodePanel();

   /// @brief Toggles the image-preview panel.
   void togglePreviewImagePanel();

   /// @brief Toggles the item-information panel.
   void toggleItemInfoPanel();

   /// @brief Toggles the settings panel.
   void toggleSettingsPanel();

   /// @brief Toggles all application toolbars.
   void toogleToolbars();

   /// @brief Opens a file-selection dialog and loads the selected project.
   void openFile();

   /// @brief Rebuilds the window menu after the set of application windows changes.
   void windowsChanged();

private:
   /// @brief Main window that owns the menus and actions.
   MainWindow* parentwindow;
   /// @brief Add-node panel controlled by the view actions.
   AddNodePanel* nodepanel;
   /// @brief Image-preview panel controlled by the view actions.
   PreviewImagePanel* previewImagePanel;
   /// @brief Item-information panel controlled by the view actions.
   ItemInfoPanel* nodesettings;
   /// @brief Settings panel controlled by the view actions.
   SettingsPanel* settingspanel;
   /// @brief Toolbar containing settings actions.
   QToolBar* settingsToolBar;
   /// @brief Toolbar containing file actions.
   QToolBar* fileToolBar;
   /// @brief Toolbar containing edit actions.
   QToolBar* editToolBar;
   /// @brief Toolbar containing insertion actions.
   QToolBar* insertToolBar;
   /// @brief Toolbar containing the add-node panel toggle.
   QToolBar* toggleAddNodeToolBar;
   /// @brief File menu.
   QMenu* fileMenu;
   /// @brief View menu.
   QMenu* viewMenu;
   /// @brief Edit menu.
   QMenu* editMenu;
   /// @brief Help menu.
   QMenu* helpMenu;
   /// @brief Action that creates a new window or project.
   QAction* newAct;
   /// @brief Action that opens a project file.
   QAction* openAct;
   /// @brief Action that saves the current project.
   QAction* saveAct;
   /// @brief Action that saves the current project to a new path.
   QAction* saveAsAct;
   /// @brief Action that exports a rendered node image.
   QAction* saveImageAct;
   /// @brief Action that closes the current window.
   QAction* closeAct;
   /// @brief Action that exits the application.
   QAction* exitAct;
   /// @brief Action that clears the current project.
   QAction* clearAct;
   /// @brief Action that copies the selected node.
   QAction* copyAct;
   /// @brief Action that cuts the selected node.
   QAction* cutAct;
   /// @brief Action that pastes serialized nodes.
   QAction* pasteAct;
   /// @brief Action that zooms in on the graph.
   QAction* zoomInAct;
   /// @brief Action that zooms out from the graph.
   QAction* zoomOutAct;
   /// @brief Action that opens application help.
   QAction* helpAct;
   /// @brief Action that opens application information.
   QAction* aboutAct;
   /// @brief Action that reveals all graph nodes.
   QAction* showAllNodesAct;
   /// @brief Action that restores the default graph zoom.
   QAction* resetZoomAct;
   /// @brief Checkable action controlling the add-node panel.
   QAction* displayAddNodePanelAct;
   /// @brief Checkable action controlling the image-preview panel.
   QAction* displayPreviewImagePanelAct;
   /// @brief Checkable action controlling the 3D-preview panel.
   QAction* displayPreview3dPanelAct;
   /// @brief Checkable action controlling the item-information panel.
   QAction* displayItemInfoPanelAct;
   /// @brief Checkable action controlling the settings panel.
   QAction* displaySettingsPanelAct;
   /// @brief Checkable action controlling toolbar visibility.
   QAction* displayToolbarsAct;
   /// @brief Shortcut action toggling the item-information panel.
   QAction* toggleItemInfoPanelAct;
   /// @brief Shortcut action toggling the settings panel.
   QAction* toggleSettingsPanelAct;
   /// @brief Shortcut action toggling the image-preview panel.
   QAction* togglePreviewImagePanelAct;
   /// @brief Shortcut action toggling the 3D-preview panel.
   QAction* togglePreview3dPanelAct;
   /// @brief Shortcut action toggling the add-node panel.
   QAction* toggleAddNodePanelAct;
   /// @brief Dynamically generated actions for other application windows.
   QVector<QAction*> windowlistActions;
   /// @brief Directory last used by the project-open dialog.
   QString lastOpenedDirectory;
};

#endif  // MENUACTIONS_H
