
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

   /// @brief Loads a cached thumbnail for a node when available.
   /// @param id Node identifier.
   /// @return True when a cached image was displayed.
   bool loadNodeImage(int id);

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
   /// @brief Repeats a pixmap in a square tile arrangement.
   /// @param pixmap Source image.
   /// @param number Number of tiles along each dimension.
   /// @return Tiled pixmap.
   QPixmap tilePixmap(const QPixmap& pixmap, int number);

   /// @brief Project whose rendered images are displayed.
   TextureProject& project;
   /// @brief Selector controlling the preview tile count.
   QComboBox* combobox{nullptr};
   /// @brief Widget displaying the two-dimensional preview.
   ImageLabel* imageLabel{nullptr};
   /// @brief Widget displaying the texture on a cube.
   CubeWidget* cubeWidget{nullptr};
   /// @brief Button that prevents selection changes from replacing the active node.
   QPushButton* lockNodeButton{nullptr};
   /// @brief Identifier of the node currently displayed.
   int currId{-1};
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
};

#endif  // PREVIEWIMAGEPANEL_H
