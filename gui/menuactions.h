/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

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

/**
 * @brief The MenuActions class
 *
 * Sets up the MainWindow's menus and toolbars and
 * the respective actions.
 *
 * Upon destruction the panel and toolbar visiblity settings are written
 * to persistant storage and loaded the next time the application is started.
 */
class MenuActions : public QObject
{
   Q_OBJECT

public:
   MenuActions(MainWindow* parentwindow);
   MainWindow* parent() { return parentwindow; }
   virtual ~MenuActions();
   void setAddNodePanel(AddNodePanel* addNodePanel);
   void setPreviewImagePanel(PreviewImagePanel* previewImagePanel);
   void setSettingsPanel(SettingsPanel* settingsPanel);
   void setItemInfoPanel(ItemInfoPanel* settingswidget);

public slots:
   void toggleAddNodePanel();
   void togglePreviewImagePanel();
   void toggleItemInfoPanel();
   void toggleSettingsPanel();
   void toogleToolbars();
   void openFile();
   void windowsChanged();

private:
   MainWindow* parentwindow;
   AddNodePanel* nodepanel;
   PreviewImagePanel* previewImagePanel;
   ItemInfoPanel* nodesettings;
   SettingsPanel* settingspanel;

   QToolBar* settingsToolBar;
   QToolBar* fileToolBar;
   QToolBar* editToolBar;
   QToolBar* insertToolBar;
   QToolBar* toggleAddNodeToolBar;

   QMenu* fileMenu;
   QMenu* viewMenu;
   QMenu* editMenu;
   QMenu* helpMenu;

   QAction* newAct;
   QAction* openAct;
   QAction* saveAct;
   QAction* saveAsAct;
   QAction* saveImageAct;
   QAction* closeAct;
   QAction* exitAct;
   QAction* clearAct;
   QAction* copyAct;
   QAction* cutAct;
   QAction* pasteAct;
   QAction* helpAct;
   QAction* aboutAct;
   QAction* resetZoomAct;
   QAction* resetSceneAct;
   QAction* displayAddNodePanelAct;
   QAction* displayPreviewImagePanelAct;
   QAction* displayPreview3dPanelAct;
   QAction* displayItemInfoPanelAct;
   QAction* displaySettingsPanelAct;
   QAction* displayToolbarsAct;
   QAction* toggleItemInfoPanelAct;
   QAction* togglePreviewImagePanelAct;
   QAction* togglePreview3dPanelAct;
   QAction* toggleAddNodePanelAct;

   QVector<QAction*> windowlistActions;
   QString lastOpenedDirectory;
};

#endif // MENUACTIONS_H
