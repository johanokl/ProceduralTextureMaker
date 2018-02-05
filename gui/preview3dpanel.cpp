/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <math.h>
#include <QVector2D>
#include <QVector3D>
#include "global.h"
#include "core/textureproject.h"
#include "generators/texturegenerator.h"
#include "gui/preview3dpanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/connectionwidget.h"
#include "generators/texturegenerator.h"
#include "core/settingsmanager.h"

/**
 * @brief Preview3dPanel::Preview3dPanel
 * @param project
 */
Preview3dPanel::Preview3dPanel(TextureProject* project)
{
   this->project = project;
   imageSize = project->getThumbnailSize();
   validImage = false;
   currId = -1;
   QObject::connect(project, SIGNAL(imageAvailable(int, QSize)),
                    this, SLOT(imageAvailable(int, QSize)));
   QObject::connect(project, SIGNAL(imageUpdated(int)),
                    this, SLOT(imageUpdated(int)));
   QObject::connect(project->getSettingsManager(), SIGNAL(settingsUpdated(void)),
                    this, SLOT(settingsUpdated(void)));
   layout = new QVBoxLayout(this);
   setLayout(layout);
   setMaximumWidth(500);
   setMinimumWidth(200);
   cubeWidget = new CubeWidget(this);
   tiledCubeWidget = new CubeWidget(this);
   layout->addWidget(cubeWidget);
   layout->addWidget(tiledCubeWidget);
   QWidget *spacerWidget = new QWidget();
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   spacerWidget->setVisible(true);
   layout->addWidget(spacerWidget);
   settingsUpdated();
}

/**
 * @brief Preview3dPanel::settingsUpdated
 * Called whenever a project setting has been changed.
 * Might not just be the background caller.
 */
void Preview3dPanel::settingsUpdated()
{
   QColor bg = project->getSettingsManager()->getPreviewBackgroundColor();
   cubeWidget->setBackgroundColor(bg);
   tiledCubeWidget->setBackgroundColor(bg);
}

/**
 * @brief Preview3dPanel::imageUpdated
 * @param id Node id
 * Removes the no longer valid image from the pixmap widgets.
 */
void Preview3dPanel::imageUpdated(int id)
{
   if (id != currId) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
   validImage = false;
   cubeWidget->imageUpdated();
   tiledCubeWidget->imageUpdated();
   update();
}

/**
 * @brief Preview3dPanel::loadNodeImage
 * @param id Node id
 * @return true if could load image
 * Checks if there is an image with the thumbnail size in the node's
 * texture cache and if found displays it in the pixmap widgets.
 */
bool Preview3dPanel::loadNodeImage(int id)
{
   TextureNodePtr texNode = project->getNode(id);
   if (texNode.isNull()) {
      return false;
   }
   imageSize = project->getThumbnailSize();
   if (!texNode->isTextureInCache(imageSize)) {
      return false;
   }
   QImage tempimage = QImage(imageSize.width(), imageSize.height(), QImage::Format_ARGB32);
   memcpy(tempimage.bits(),
          texNode->getImage(imageSize)->getData(),
          (imageSize.width() * imageSize.height() * sizeof(TexturePixel)));
   QPixmap newImage = QPixmap::fromImage(tempimage);
   cubeWidget->setTexture(newImage);
   tiledCubeWidget->setTiledTexture(newImage);
   cubeWidget->show();
   tiledCubeWidget->show();
   validImage = true;
   return true;
}

/**
 * @brief Preview3dPanel::imageAvailable
 * @param id Node id
 * @param size Image size
 * A node has a new generated image in the specified size.
 * If it's the desired size and the panel is visible it
 * loads a new image to the widgets.
 */
void Preview3dPanel::imageAvailable(int id, QSize size)
{
   if (id != currId || size != imageSize) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
   loadNodeImage(id);
}

/**
 * @brief Preview3dPanel::showEvent
 * @param event
 * Load the updated textures when the panel is opened.
 */
void Preview3dPanel::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);
   if (!loadNodeImage(currId)) {
      cubeWidget->imageUpdated();
      tiledCubeWidget->imageUpdated();
   }
}

/**
 * @brief Preview3dPanel::setActiveNode
 * @param id Node id
 * Updates the textures.
 */
void Preview3dPanel::setActiveNode(int id)
{
   if (currId == id) {
      return;
   }
   currId = id;
   if (this->isHidden()) {
      return;
   }
   cubeWidget->imageUpdated();
   tiledCubeWidget->imageUpdated();
   loadNodeImage(id);
}

/**
 * @brief CubeWidget::CubeWidget
 * @param parent
 */
CubeWidget::CubeWidget(QWidget *parent) :
   QOpenGLWidget(parent), indexBuf(QOpenGLBuffer::IndexBuffer)
{
   initialized = false;
   geometries = 0;
   texture = 0;
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
   delete texture;
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
 * Called every 12 milliseconds.
 */
void CubeWidget::timerEvent(QTimerEvent *)
{
   // Decrease angular speed (friction)
   angularSpeed *= 0.99;
   // Stop rotation when speed goes below threshold
   if (isVisible() && angularSpeed > 0.01) {
      // Update rotation
      rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
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
   // Compile vertex shader
   if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/3d/vshader.glsl") ||
       !program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/3d/fshader.glsl") ||
       !program.link() ||
       !program.bind()) {
      close();
   }
   QPixmap emptyPixmap(QSize(10, 10));
   emptyPixmap.fill(Qt::black);
   emptytexture = new QOpenGLTexture(emptyPixmap.toImage().mirrored());
   textureAvailable = false;
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
   timer.start(12, this);
   initialized = true;
}

/**
 * @brief CubeWidget::setTiledTexture
 * @param pixmap Original 1:1 texture.
 * Scales the texturs sizes 50% so four tiles are fitted in
 * the place of one.
 */
void CubeWidget::setTiledTexture(const QPixmap &pixmap)
{
   if (pixmap.width() <= 0 || pixmap.height() <= 0) {
      return;
   }
   QPixmap newPixmap(pixmap.size() * 2);
   newPixmap.fill(Qt::white);
   QPainter painter(&newPixmap);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, newPixmap.width(), newPixmap.height(), QBrush(pixmap));
   setTexture(newPixmap);
}

/**
 * @brief CubeWidget::imageUpdated
 * Called when a node's settings have been changed and the
 * current image is no longer valid.
 */
void CubeWidget::imageUpdated()
{
   textureAvailable = false;
   update();
}

/**
 * @brief CubeWidget::setBackgroundColor
 * Set the widget's background color.
 */
void CubeWidget::setBackgroundColor(QColor bg)
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
void CubeWidget::setTexture(const QPixmap &pixmap)
{
   if (pixmap.size().isEmpty() || !pixmap.size().isValid()) {
       return;
   }
   if (texture) {
      makeCurrent();
      texture->release();
      delete texture;
   }
   texture = new QOpenGLTexture(pixmap.toImage().mirrored());
   texture->setMinificationFilter(QOpenGLTexture::Nearest);
   texture->setMagnificationFilter(QOpenGLTexture::Linear);
   textureAvailable = true;
   update();
}

/**
 * @brief CubeWidget::resizeGL
 * Resizes the widget's perspective.
 */
void CubeWidget::resizeGL(int width, int height)
{
   double aspectRatio = (double) width / (double) (height ? height : 1);
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
   if (!texture) {
      return;
   }
   // Clear color and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (textureAvailable) {
      texture->bind();
   } else {
      emptytexture->bind();
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
   quintptr offset = 0;
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
   glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, 0);
}
