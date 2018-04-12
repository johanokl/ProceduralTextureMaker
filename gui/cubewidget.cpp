/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "gui/cubewidget.h"
#include <QMouseEvent>

/**
 * @brief CubeWidget::CubeWidget
 * @param parent
 */
CubeWidget::CubeWidget(QWidget *parent) :
   QOpenGLWidget(parent), indexBuf(QOpenGLBuffer::IndexBuffer)
{
   initialized = false;
   texture = nullptr;
   angularSpeed = 0;
   QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   sizePolicy.setHeightForWidth(true);
   setSizePolicy(sizePolicy);
}

/**
 * @brief CubeWidget::~CubeWidget
 */
CubeWidget::~CubeWidget()
{
   // Make sure the context is current when deleting the texture
   makeCurrent();
   if (texture != nullptr) {
      delete texture;
   }
   arrayBuf.destroy();
   indexBuf.destroy();
   doneCurrent();
}

/**
 * @brief CubeWidget::mousePressEvent
 * Save mouse press position
 */
void CubeWidget::mousePressEvent(QMouseEvent *e)
{
   mousePressPosition = QVector2D(e->localPos());
}

/**
 * @brief CubeWidget::mouseReleaseEvent
 * Change rotation based on difference.
 */
void CubeWidget::mouseReleaseEvent(QMouseEvent *e)
{
   // Mouse release position - mouse press position
   QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;
   // Rotation axis is perpendicular to the mouse position difference
   QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
   // Accelerate angular speed relative to the length of the mouse sweep
   qreal acc = diff.length() / 50.0;
   // Calculate new rotation axis as weighted sum
   rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();
   // Increase angular speed
   angularSpeed += acc;
}

/**
 * @brief CubeWidget::timerEvent
 * Called every 24 milliseconds.
 */
void CubeWidget::timerEvent(QTimerEvent *)
{
   // Decrease angular speed (friction)
   angularSpeed *= 0.99;
   // Stop rotation when speed goes below threshold
   if (isVisible() && (textureUpdated || angularSpeed > 0.01)) {
      rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
      textureUpdated = false;
      update();
   }
}

/**
 * @brief CubeWidget::initializeGL
 * Initializes OpenGL and creates the 3D cube.
 */
void CubeWidget::initializeGL()
{
   initializeOpenGLFunctions();
   // Compile shaders
   if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/3d/vshader.glsl") ||
       !program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/3d/fshader.glsl") ||
       !program.link() ||
       !program.bind()) {
      close();
   }
   glClearColor(backgroundcolor.redF(), backgroundcolor.greenF(), backgroundcolor.blueF(), 1);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   arrayBuf.create();
   indexBuf.create();
   CubeWidget::VertexData vertices[] = {
      {QVector3D(-1, -1,  1), QVector2D(0, 0)},
      {QVector3D( 1, -1,  1), QVector2D(1, 0)},
      {QVector3D(-1,  1,  1), QVector2D(0, 1)},
      {QVector3D( 1,  1,  1), QVector2D(1, 1)},
      {QVector3D( 1, -1,  1), QVector2D(0, 1)},
      {QVector3D( 1, -1, -1), QVector2D(0, 0)},
      {QVector3D( 1,  1,  1), QVector2D(1, 1)},
      {QVector3D( 1,  1, -1), QVector2D(1, 0)},
      {QVector3D( 1, -1, -1), QVector2D(1, 0)},
      {QVector3D(-1, -1, -1), QVector2D(0, 0)},
      {QVector3D( 1,  1, -1), QVector2D(1, 1)},
      {QVector3D(-1,  1, -1), QVector2D(0, 1)},
      {QVector3D(-1, -1, -1), QVector2D(0, 0)},
      {QVector3D(-1, -1,  1), QVector2D(0, 1)},
      {QVector3D(-1,  1, -1), QVector2D(1, 0)},
      {QVector3D(-1,  1,  1), QVector2D(1, 1)},
      {QVector3D(-1, -1, -1), QVector2D(0, 0)},
      {QVector3D( 1, -1, -1), QVector2D(1, 0)},
      {QVector3D(-1, -1,  1), QVector2D(0, 1)},
      {QVector3D( 1, -1,  1), QVector2D(1, 1)},
      {QVector3D(-1,  1,  1), QVector2D(0, 1)},
      {QVector3D( 1,  1,  1), QVector2D(1, 1)},
      {QVector3D(-1,  1, -1), QVector2D(0, 0)},
      {QVector3D( 1,  1, -1), QVector2D(1, 0)}
   };
   GLushort indices[] = {
      0, 1, 2, 3, 3, 4, 4, 5, 6, 7, 7, 8, 8, 9, 10, 11, 11, 12, 12,
      13, 14, 15, 15, 16, 16, 17, 18, 19, 19, 20, 20, 21, 22, 23
   };
   // Transfer vertex data to VBO 0
   arrayBuf.bind();
   arrayBuf.allocate(vertices, 24 * sizeof(VertexData));
   // Transfer index data to VBO 1
   indexBuf.bind();
   indexBuf.allocate(indices, 34 * sizeof(GLushort));
   // Use QBasicTimer because its faster than QTimer
   timer.start(24, this);
   initialized = true;
}

/**
 * @brief CubeWidget::imageUpdated
 * Called when a node's settings have been changed and the
 * current image is no longer valid.
 */
void CubeWidget::imageUpdated()
{
   update();
}

/**
 * @brief CubeWidget::setBackgroundColor
 * Set the widget's background color.
 */
void CubeWidget::setBackgroundColor(const QColor& bg)
{
   backgroundcolor = bg;
   if (initialized) {
      makeCurrent();
      glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1);
      update();
   }
}

/**
 * @brief CubeWidget::setTexture
 * @param pixmap The texture
 * Displays the QPixmap on all six sides of the cube.
 */
void CubeWidget::setTexture(const QPixmap& pixmap)
{
   if (texture != nullptr) {
      makeCurrent();
      texture->release();
      delete texture;
      texture = nullptr;
      textureUpdated = true;
   }
   if (!pixmap.isNull() && !pixmap.size().isEmpty()) {
      makeCurrent();
      texture = new QOpenGLTexture(pixmap.toImage().mirrored());
      texture->setMinificationFilter(QOpenGLTexture::Nearest);
      texture->setMagnificationFilter(QOpenGLTexture::Linear);
      textureUpdated = true;
   }
}

/**
 * @brief CubeWidget::resizeGL
 * Resizes the widget's perspective.
 */
void CubeWidget::resizeGL(int width, int height)
{
   float aspectRatio = static_cast<float>(width) / static_cast<float>(height ? height : 1);
   // Reset projection
   projection.setToIdentity();
   // Set perspective projection
   // Near plane to 3.0, far plane to 7.0, field of view 45 degrees
   projection.perspective(45.0, aspectRatio, 3.0, 7.0);
}

/**
 * @brief CubeWidget::paintGL
 * Qt's paint function.
 */
void CubeWidget::paintGL()
{
   // Clear color and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (texture != nullptr) {
      texture->bind();
   }
   // Calculate model view transformation
   QMatrix4x4 matrix;
   matrix.translate(0.0, 0.0, -5.0);
   matrix.rotate(rotation);
   program.setUniformValue("mvp_matrix", projection * matrix);
   program.setUniformValue("texture", 0);
   // Tell OpenGL which VBOs to use
   arrayBuf.bind();
   indexBuf.bind();
   // Offset for position
   int offset = 0;
   // Tell OpenGL programmable pipeline how to locate vertex position data
   int vertexLocation = program.attributeLocation("a_position");
   program.enableAttributeArray(vertexLocation);
   program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));
   // Offset for texture coordinate
   offset += sizeof(QVector3D);
   // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
   int texcoordLocation = program.attributeLocation("a_texcoord");
   program.enableAttributeArray(texcoordLocation);
   program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));
   glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
}
