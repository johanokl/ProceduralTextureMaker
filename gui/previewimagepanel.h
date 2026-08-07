
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PREVIEWIMAGEPANEL_H
#define PREVIEWIMAGEPANEL_H

#include <QWidget>
class TextureProject;
class QPushButton;
class ImageLabel;
class QLabel;
class QComboBox;
class CubeWidget;
class QGroupBox;
class QResizeEvent;
class QShowEvent;

/// @brief Vertical panel that displays a node's generated image.
/// When an active node's image is updated this also propagates
/// to the panel, which then updates the displayed image.
class PreviewImagePanel : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates an image preview panel for a texture project.
   /// @param project Project whose rendered images are displayed.
   explicit PreviewImagePanel(TextureProject& project);

   /// @brief Destroys the image preview panel.
   ~PreviewImagePanel() override = default;

   /// @brief Reloads preview images when the panel becomes visible.
   /// @param event Show event.
   void showEvent(QShowEvent* event) override;

public slots:
   /// @brief Selects the node displayed by the preview panel.
   /// @param id Node identifier.
   void setActiveNode(int id);

   /// @brief Displays a newly available thumbnail for the active node.
   /// @param id Node identifier.
   /// @param size Available image size.
   void imageAvailable(int id, QSize size);

   /// @brief Clears a preview whose cached image is no longer valid.
   /// @param id Updated node identifier.
   void imageUpdated(int id);

   /// @brief Applies changed project settings to the preview widgets.
   void settingsUpdated();

   /// @brief Clears the preview when its active node is removed.
   /// @param id Removed node identifier.
   void nodeRemoved(int id);

private:
   /// @brief Loads the selected node into the 2D and 3D previews when cached.
   /// @return True when a cached image was displayed.
   bool loadSelectedNodeImage();

   /// @brief Loads the locked node into its 2D preview when cached.
   /// @return True when a cached image was displayed.
   bool loadLockedNodeImage();

   /// @brief Returns a tiled cached thumbnail for a node.
   /// @param id Node identifier.
   /// @return The tiled thumbnail, or a null pixmap when no cached image is available.
   QPixmap nodePixmap(int id);

   /// @brief Composites a texture over the configured transparent-node background.
   QPixmap pixmapWithNodeBackground(const QPixmap& pixmap) const;

   /// @brief Repeats a pixmap in a square tile arrangement.
   /// @param pixmap Source image.
   /// @param number Number of tiles along each dimension.
   /// @return Tiled pixmap.
   QPixmap tilePixmap(const QPixmap& pixmap, int number);

   /// @brief Locks or unlocks the currently selected node preview.
   /// @param locked Whether the current node should be retained in the locked preview.
   void setNodeLocked(bool locked);

   /// @brief Shows or hides the optional three-dimensional preview.
   /// @param visible Whether the 3D preview was requested.
   void setThreeDPreviewVisible(bool visible);

   /// @brief Applies pending preview visibility and aspect-ratio geometry changes.
   void updatePreviewLayout();

   /// @brief Updates the panel minimum width from the current preview controls.
   void updateControlsMinimumWidth();

   /// @brief Project whose rendered images are displayed.
   TextureProject& project;
   /// @brief Selector controlling the preview tile count.
   QComboBox* tileCountComboBox{nullptr};
   /// @brief Widget displaying the currently selected node.
   ImageLabel* selectedImageLabel{nullptr};
   /// @brief Widget displaying the retained locked node.
   ImageLabel* lockedImageLabel{nullptr};
   /// @brief Widget displaying the texture on a cube.
   CubeWidget* cubeWidget{nullptr};
   /// @brief Container for the locked-node preview, hidden while no node is locked.
   QGroupBox* lockedNodePreview{nullptr};
   /// @brief Container for the selected-node preview.
   QGroupBox* selectedNodePreview{nullptr};
   /// @brief Container for the optional three-dimensional preview.
   QGroupBox* threeDPreview{nullptr};
   /// @brief Button that captures or releases the current node in the locked preview.
   QPushButton* lockNodeButton{nullptr};
   /// @brief Button controlling whether the three-dimensional preview is displayed.
   QPushButton* showThreeDButton{nullptr};
   /// @brief Identifier of the currently selected node.
   int selectedNodeId{-1};
   /// @brief Identifier retained in the locked preview, or -1 when unlocked.
   int lockedNodeId{-1};
   /// @brief Number of tiles along each preview dimension.
   int numTiles{1};
   /// @brief Cached thumbnail size requested from the project.
   QSize imageSize;
};

/// @brief Automatically scales a pixmap to fit inside a widget.
class ImageLabel : public QWidget {
   Q_OBJECT
public:
   /// @brief Creates an automatically scaling image widget.
   /// @param parent Parent widget.
   explicit ImageLabel(QWidget* parent = nullptr);

public slots:
   /// @brief Sets the image and scales it to fill the widget.
   /// @param pixmap Image to display.
   void setPixmap(const QPixmap& pixmap);

   /// @brief Shows or hides the stale-image grid over the current pixmap.
   /// @param rendering Whether a replacement image is being rendered.
   void setRendering(bool rendering);

public:
   /// @brief Returns a compact preferred size that preserves the current image aspect ratio.
   QSize sizeHint() const override;

   /// @brief Returns the image height required for a given widget width.
   /// @param width Available image width.
   /// @return Aspect-ratio-preserving image height.
   int heightForWidth(int width) const override;

   /// @brief Returns the current pixmap with the stale-image grid composited over it.
   /// @return Grid-marked pixmap, or a null pixmap when no image is displayed.
   QPixmap pixmapWithRenderingOverlay() const;

protected:
   /// @brief Rescales the image after the widget size changes.
   /// @param event Resize event.
   void resizeEvent(QResizeEvent* event) override;

private slots:
   /// @brief Updates the image transformation for the current widget size.
   void resizeImage();

private:
   /// @brief Label that renders the current pixmap.
   QLabel* label{nullptr};
   /// @brief Transparent grid displayed over a stale pixmap during rendering.
   QWidget* renderingOverlay{nullptr};
};

#endif  // PREVIEWIMAGEPANEL_H
