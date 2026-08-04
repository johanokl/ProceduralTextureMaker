
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/cubewidget.h"
#include <QDebug>
#include <QMouseEvent>

CubeWidget::CubeWidget(QWidget* parent)
    : QOpenGLWidget(parent), indexBuf(QOpenGLBuffer::IndexBuffer) {
   QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   sizePolicy.setHeightForWidth(true);
   setSizePolicy(sizePolicy);
}

QSize CubeWidget::sizeHint() const { return {256, 256}; }

int CubeWidget::heightForWidth(const int width) const { return width; }

CubeWidget::~CubeWidget() {
   // Make sure the context is current when deleting the texture
   if (initialized) {
      makeCurrent();
   }
   texture.reset();
   arrayBuf.destroy();
   indexBuf.destroy();
   if (initialized) {
      doneCurrent();
   }
}

void CubeWidget::mousePressEvent(QMouseEvent* event) {
   if (event->button() == Qt::RightButton) {
      rotation = QQuaternion();
      update();
      event->accept();
      return;
   }
   if (event->button() == Qt::LeftButton) {
      mousePosition = QVector2D(event->position());
      event->accept();
      return;
   }
   QOpenGLWidget::mousePressEvent(event);
}

void CubeWidget::mouseMoveEvent(QMouseEvent* event) {
   if (!event->buttons().testFlag(Qt::LeftButton)) {
      QOpenGLWidget::mouseMoveEvent(event);
      return;
   }
   const QVector2D currentPosition(event->position());
   QVector2D movement = currentPosition - mousePosition;
   mousePosition = currentPosition;
   if (movement.isNull()) {
      return;
   }
   if (qAbs(movement.y()) * 4.0F < qAbs(movement.x())) {
      movement.setY(0.0F);
   } else if (qAbs(movement.x()) * 4.0F < qAbs(movement.y())) {
      movement.setX(0.0F);
   }
   const float degreesPerPixel = 180.0F / static_cast<float>(qMax(1, qMin(width(), height())));
   const QQuaternion yaw =
       QQuaternion::fromAxisAndAngle(QVector3D(0.0F, 1.0F, 0.0F), movement.x() * degreesPerPixel);
   const QQuaternion pitch =
       QQuaternion::fromAxisAndAngle(QVector3D(1.0F, 0.0F, 0.0F), movement.y() * degreesPerPixel);
   rotation = (pitch * yaw * rotation).normalized();
   update();
   event->accept();
}

void CubeWidget::initializeGL() {
   initializeOpenGLFunctions();
   // Compile shaders
   if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/3d/vshader.glsl") ||
       !program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/3d/fshader.glsl") ||
       !program.link() || !program.bind()) {
      qWarning() << "Could not initialize 3D preview shader:" << program.log();
      close();
      return;
   }
   glClearColor(backgroundcolor.redF(), backgroundcolor.greenF(), backgroundcolor.blueF(), 1);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   arrayBuf.create();
   indexBuf.create();
   CubeWidget::VertexData vertices[] = {
       {QVector3D(-1, -1, 1), QVector2D(0, 0)},  {QVector3D(1, -1, 1), QVector2D(1, 0)},
       {QVector3D(-1, 1, 1), QVector2D(0, 1)},   {QVector3D(1, 1, 1), QVector2D(1, 1)},
       {QVector3D(1, -1, 1), QVector2D(0, 0)},   {QVector3D(1, -1, -1), QVector2D(1, 0)},
       {QVector3D(1, 1, 1), QVector2D(0, 1)},    {QVector3D(1, 1, -1), QVector2D(1, 1)},
       {QVector3D(1, -1, -1), QVector2D(1, 0)},  {QVector3D(-1, -1, -1), QVector2D(0, 0)},
       {QVector3D(1, 1, -1), QVector2D(1, 1)},   {QVector3D(-1, 1, -1), QVector2D(0, 1)},
       {QVector3D(-1, -1, -1), QVector2D(0, 0)}, {QVector3D(-1, -1, 1), QVector2D(1, 0)},
       {QVector3D(-1, 1, -1), QVector2D(0, 1)},  {QVector3D(-1, 1, 1), QVector2D(1, 1)},
       {QVector3D(-1, -1, -1), QVector2D(0, 0)}, {QVector3D(1, -1, -1), QVector2D(1, 0)},
       {QVector3D(-1, -1, 1), QVector2D(0, 1)},  {QVector3D(1, -1, 1), QVector2D(1, 1)},
       {QVector3D(-1, 1, 1), QVector2D(0, 1)},   {QVector3D(1, 1, 1), QVector2D(1, 1)},
       {QVector3D(-1, 1, -1), QVector2D(0, 0)},  {QVector3D(1, 1, -1), QVector2D(1, 0)}};
   GLushort indices[] = {0,  1,  2,  3,  3,  4,  4,  5,  6,  7,  7,  8,  8,  9,  10, 11, 11,
                         12, 12, 13, 14, 15, 15, 16, 16, 17, 18, 19, 19, 20, 20, 21, 22, 23};
   // Transfer vertex data to VBO 0
   arrayBuf.bind();
   arrayBuf.allocate(vertices, 24 * sizeof(VertexData));
   // Transfer index data to VBO 1
   indexBuf.bind();
   indexBuf.allocate(indices, 34 * sizeof(GLushort));
   initialized = true;
   uploadTexture();
}

void CubeWidget::imageUpdated() { update(); }

void CubeWidget::setBackgroundColor(const QColor& bg) {
   backgroundcolor = bg;
   if (initialized) {
      makeCurrent();
      glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1);
      update();
   }
}

void CubeWidget::setTexture(const QPixmap& pixmap) {
   pendingTexture = pixmap;
   if (!initialized) {
      update();
      return;
   }
   makeCurrent();
   uploadTexture();
   doneCurrent();
   update();
}

void CubeWidget::uploadTexture() {
   if (texture) {
      texture->release();
      texture.reset();
   }
   if (!pendingTexture.isNull() && !pendingTexture.size().isEmpty()) {
      QImage image = pendingTexture.toImage();
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
      image = image.flipped(Qt::Vertical);
#else
      image = image.mirrored(false, true);
#endif
      texture = std::make_unique<QOpenGLTexture>(image);
      texture->setMinificationFilter(QOpenGLTexture::Nearest);
      texture->setMagnificationFilter(QOpenGLTexture::Linear);
   }
}

void CubeWidget::resizeGL(int width, int height) {
   float aspectRatio = static_cast<float>(width) / static_cast<float>(height ? height : 1);
   // Reset projection
   projection.setToIdentity();
   // Set perspective projection
   // Keep the near plane well in front of the cube so rotations are not clipped.
   projection.perspective(45.0, aspectRatio, 1.0, 10.0);
}

void CubeWidget::paintGL() {
   // Clear color and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (!program.isLinked()) {
      return;
   }
   if (!program.bind()) {
      qWarning() << "Could not bind 3D preview shader:" << program.log();
      return;
   }
   glEnable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   if (texture != nullptr) {
      texture->bind(0);
   }
   // Calculate model view transformation
   QMatrix4x4 matrix;
   matrix.translate(0.0, 0.0, -4.0);
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
   if (vertexLocation >= 0) {
      program.enableAttributeArray(vertexLocation);
      program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));
   }
   // Offset for texture coordinate
   offset += sizeof(QVector3D);
   // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
   int texcoordLocation = program.attributeLocation("a_texcoord");
   if (texcoordLocation >= 0) {
      program.enableAttributeArray(texcoordLocation);
      program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));
   }
   glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
   if (texcoordLocation >= 0) {
      program.disableAttributeArray(texcoordLocation);
   }
   if (vertexLocation >= 0) {
      program.disableAttributeArray(vertexLocation);
   }
   if (texture != nullptr) {
      texture->release();
   }
   program.release();
}
