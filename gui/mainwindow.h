
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base/texturegenerator.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>
class TexGenApplication;
class TextureProject;
class ViewNodeScene;
class ViewNodeView;
class ItemInfoPanel;
class AddNodePanel;
class QCloseEvent;
class QDialog;
class MenuActions;
class PreviewImagePanel;
class SettingsPanel;
class SettingsManager;
class JsTexGenManager;
class EditManager;

/// @brief The application's main window, containing all the scenes and panels.
///
/// It initializes and configures the TextureProject with its submodules,
/// including which texture generators should be loaded.
class MainWindow : public QMainWindow {
   Q_OBJECT

public:
   /// @brief Creates the main window and its project, scene, panels, and actions.
   /// @param parent Application that owns and manages the window.
   explicit MainWindow(TexGenApplication* parent = nullptr);

   /// @brief Destroys the window and unregisters it from the application.
   ~MainWindow() override;

   /// @brief Returns the texture project displayed by the window.
   TextureProject* getTextureProject() { return project.get(); }

   /// @brief Returns the application that owns the window.
   TexGenApplication* parent() { return parentapp; }

   /// @brief Returns the window's menu and toolbar manager.
   MenuActions* getMenu() { return menuactions.get(); }

   /// @brief Returns the manager for user-initiated project edits in this window.
   EditManager& getEditManager() { return *editManager; }

public slots:
   /// @brief Prompts for a destination and saves the current project.
   /// @return True when the project was saved successfully.
   bool saveAs();

   /// @brief Saves the current project as an XML file.
   /// @param newFileName Whether to prompt for a new destination instead of reusing the current
   /// one.
   /// @return True when the project was saved successfully.
   bool saveFile(bool newFileName = false);

   /// @brief Opens a project file after offering to save pending changes.
   /// @param fileName Project file to load.
   void openFile(const QString& fileName);

   /// @brief Clears the scene after offering to save pending changes.
   void clearScene();

   /// @brief Displays application and build information.
   void showAbout();

   /// @brief Displays the application help dialog.
   void showHelp();

   /// @brief Accepts or rejects a close request according to pending project changes.
   /// @param event Window close event.
   void closeEvent(QCloseEvent* event) override;

   /// @brief Copies the selected node to the system clipboard.
   void copyNode();

   /// @brief Pastes serialized nodes from the system clipboard.
   void pasteNode();

   /// @brief Copies the selected node to the clipboard and removes it.
   void cutNode();

   /// @brief Saves a node's rendered image as a PNG file.
   /// @param id Node identifier, or 0 to use the selected node.
   void saveImage(int id = 0);

   /// @brief Recreates the scene view while preserving its graph.
   void reloadSceneView();

   /// @brief Rescans the configured custom JavaScript generator directory.
   void reloadJavaScriptGenerators();

   /// @brief Raises and activates the window.
   void moveToFront();

   /// @brief Resets the scene view to its default zoom level.
   void resetViewZoom();

   /// @brief Shows all hidden nodes and recreates the scene view.
   void showAllNodesAndResetSceneView();

   /// @brief Zooms in on the node scene.
   void zoomInView();

   /// @brief Zooms out from the node scene.
   void zoomOutView();

   /// @brief Sets regular and highlighted connection widths.
   /// @param normalWidth Width of regular connection lines.
   /// @param highlightedWidth Width of highlighted connection lines.
   void setLineWidths(int normalWidth, int highlightedWidth);

   /// @brief Sets the connection arrow size.
   /// @param arrowSize New arrow size.
   void setArrowSize(int arrowSize);

   /// @brief Sets the height of node title areas.
   /// @param headerSize New title-area height.
   void setHeaderSize(int headerSize);

   /// @brief Sets the zoom factor applied by each scene-view zoom step.
   /// @param factor New zoom step factor.
   void setZoomStepFactor(double factor);

   /// @brief Reports an attempt to register two generators with the same name.
   /// @param oldGen Generator already registered with the project.
   /// @param newGen Generator rejected because its name is already used.
   void generatorNameCollision(const TextureGeneratorPtr& oldGen,
                               const TextureGeneratorPtr& newGen);

private:
   /// @brief Creates the node view and embeds it in the main window.
   void drawScene();

   /// @brief Creates menu actions and connects project and application signals.
   void createActions();
   /// @brief Destroys GUI observers before their model dependencies.
   void destroyGuiObservers();

   /// @brief Offers to save a modified project before it is closed or replaced.
   /// @return True when it is safe to discard the current project state.
   bool maybeSave();

   /// @brief Creates a node scene, optionally copying another scene's presentation state.
   /// @param source Existing scene to copy, or nullptr for a new scene.
   /// @return Newly created owned scene.
   std::unique_ptr<ViewNodeScene> createScene(ViewNodeScene* source = nullptr);

   /// @brief Application that owns and manages this window.
   TexGenApplication* parentapp{nullptr};
   /// @brief Texture project displayed by the window.
   std::unique_ptr<TextureProject> project;
   /// @brief Manager and undo history for user-initiated project edits.
   std::unique_ptr<EditManager> editManager;
   /// @brief Path last used to save the current project.
   QString savedFileName;
   /// @brief Manager for the window's menus, toolbars, and actions.
   std::unique_ptr<MenuActions> menuactions;
   /// @brief Scene containing the graph presentation.
   std::unique_ptr<ViewNodeScene> scene;
   /// @brief View displaying the node scene.
   ViewNodeView* view{nullptr};
   /// @brief Persistent settings manager used by the project and panels.
   std::unique_ptr<SettingsManager> settingsManager;
   /// @brief Manager for external JavaScript generators.
   std::unique_ptr<JsTexGenManager> jstexgenManager;
   /// @brief Panel displaying scene, node, and connection information.
   ItemInfoPanel* iteminfopanel{nullptr};
   /// @brief Panel used to edit application and project settings.
   SettingsPanel* settingspanel{nullptr};
   /// @brief Panel containing draggable generator buttons.
   AddNodePanel* addnodewidget{nullptr};
   /// @brief Panel displaying the selected node's rendered image.
   PreviewImagePanel* previewimagewidget{nullptr};
   /// @brief Non-modal help dialog, guarded because it deletes itself when closed.
   QPointer<QDialog> helpDialog;
};

#endif  // MAINWINDOW_H
