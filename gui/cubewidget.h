
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef PREVIEW3DPANEL_H
#define PREVIEW3DPANEL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QBasicTimer>
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

   /// @brief Schedules a pixmap for display on all six sides of the cube.
   /// @param pixmap Texture to display.
   void setTexture(const QPixmap& pixmap);

public slots:
   /// @brief Requests a repaint after the current image becomes invalid.
   void imageUpdated();

   /// @brief Sets the widget background color.
   /// @param color New background color.
   void setBackgroundColor(const QColor& color);

protected:
   /// @brief Stores the starting position for a mouse-driven rotation.
   /// @param e Mouse press event.
   void mousePressEvent(QMouseEvent* e) override;

   /// @brief Updates cube rotation from the completed mouse drag.
   /// @param e Mouse release event.
   void mouseReleaseEvent(QMouseEvent* e) override;

   /// @brief Advances cube rotation using the current angular speed.
   /// @param e Timer event.
   void timerEvent(QTimerEvent* e) override;

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
   /// @brief Timer driving inertial cube rotation.
   QBasicTimer timer;
   /// @brief Shader program used to render the cube.
   QOpenGLShaderProgram program;
   /// @brief Texture currently uploaded to the GPU.
   QOpenGLTexture* texture;
   /// @brief Pixmap waiting to be uploaded as the cube texture.
   QPixmap pendingTexture;
   /// @brief Whether the pending texture must be uploaded.
   bool textureUpdated;
   /// @brief Perspective projection matrix.
   QMatrix4x4 projection;
   /// @brief Position at which the current mouse drag began.
   QVector2D mousePressPosition;
   /// @brief Axis used for inertial cube rotation.
   QVector3D rotationAxis;
   /// @brief Current inertial rotation speed.
   double angularSpeed;
   /// @brief Current cube orientation.
   QQuaternion rotation;
   /// @brief Whether a texture is available for rendering.
   bool textureAvailable;
   /// @brief Vertex buffer containing cube geometry.
   QOpenGLBuffer arrayBuf;
   /// @brief Index buffer defining cube triangles.
   QOpenGLBuffer indexBuf;
   /// @brief Color used to clear the OpenGL background.
   QColor backgroundcolor;
   /// @brief Whether OpenGL resources have been initialized.
   bool initialized;
};

#endif  // PREVIEW3DPANEL_H
