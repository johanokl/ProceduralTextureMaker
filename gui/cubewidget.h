
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PREVIEW3DPANEL_H
#define PREVIEW3DPANEL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <memory>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QPixmap>
#include <QQuaternion>

/// @brief Displays a texture on a rotatable OpenGL cube.
class CubeWidget : public QOpenGLWidget, protected QOpenGLFunctions {
   Q_OBJECT
   struct VertexData {
      /// @brief Vertex position in model coordinates.
      QVector3D position;
      /// @brief Texture coordinate associated with the vertex.
      QVector2D texCoord;
   };

public:
   /// @brief Creates an OpenGL cube preview widget.
   /// @param parent Parent widget.
   explicit CubeWidget(QWidget* parent = nullptr);

   /// @brief Releases the cube's OpenGL resources.
   ~CubeWidget() override;

   /// @brief Returns the preferred square viewport size.
   QSize sizeHint() const override;

   /// @brief Returns the square viewport height for the available width.
   /// @param width Available viewport width.
   /// @return Matching viewport height.
   int heightForWidth(int width) const override;

   /// @brief Schedules a pixmap for display on all six sides of the cube.
   /// @param pixmap Texture to display.
   void setTexture(const QPixmap& pixmap);

   /// @brief Enables linear magnification or nearest-neighbor magnification.
   void setSmoothFiltering(bool enabled);

   /// @brief Checks whether linear texture magnification is enabled.
   bool hasSmoothFiltering() const { return smoothFiltering; }

public slots:
   /// @brief Requests a repaint after the current image becomes invalid.
   void imageUpdated();

   /// @brief Sets the widget background color.
   /// @param color New background color.
   void setBackgroundColor(const QColor& color);

protected:
   /// @brief Starts a left-button rotation or resets it with the right button.
   /// @param event Mouse press event.
   void mousePressEvent(QMouseEvent* event) override;

   /// @brief Rotates the cube while the left mouse button is dragged.
   /// @param event Mouse move event.
   void mouseMoveEvent(QMouseEvent* event) override;

   /// @brief Initializes OpenGL resources and cube geometry.
   void initializeGL() override;

   /// @brief Updates the perspective projection for a new widget size.
   /// @param width New viewport width.
   /// @param height New viewport height.
   void resizeGL(int width, int height) override;

   /// @brief Renders the textured cube.
   void paintGL() override;

   /// @brief Compiles and links the cube shaders.
   void initShaders();

private:
   /// @brief Uploads the pending texture to the GPU.
   void uploadTexture();

   /// @brief Shader program used to render the cube.
   QOpenGLShaderProgram program;
   /// @brief Texture currently uploaded to the GPU.
   std::unique_ptr<QOpenGLTexture> texture;
   /// @brief Pixmap waiting to be uploaded as the cube texture.
   QPixmap pendingTexture;
   /// @brief Perspective projection matrix.
   QMatrix4x4 projection;
   /// @brief Most recent position in the active mouse drag.
   QVector2D mousePosition;
   /// @brief Current cube orientation.
   QQuaternion rotation;
   /// @brief Whether a texture is available for rendering.
   bool textureAvailable{false};
   /// @brief Vertex buffer containing cube geometry.
   QOpenGLBuffer arrayBuf;
   /// @brief Index buffer defining cube triangles.
   QOpenGLBuffer indexBuf;
   /// @brief Color used to clear the OpenGL background.
   QColor backgroundcolor;
   /// @brief Whether OpenGL resources have been initialized.
   bool initialized{false};
   /// @brief Whether texture magnification uses linear filtering.
   bool smoothFiltering{true};
};

#endif  // PREVIEW3DPANEL_H
