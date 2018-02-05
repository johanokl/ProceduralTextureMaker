/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PREVIEW3DPANEL_H
#define PREVIEW3DPANEL_H

class TextureProject;
class QVBoxLayout;
class GeometryEngine;
class CubeWidget;
class GeometryEngine;

#include <QWidget>
#include <QMap>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

/**
 * @brief The PreviewImagePanel class
 *
 * Vertical panel that displays a node's generated image.
 * When an active node's image is updated this also propagates
 * to the panel, which then updates the displayed image.
 */
class Preview3dPanel : public QWidget
{
   Q_OBJECT

public:
   Preview3dPanel(TextureProject*);
   virtual ~Preview3dPanel() {}
   virtual void showEvent(QShowEvent* event);
   bool loadNodeImage(int);

public slots:
   void setActiveNode(int id);
   void imageAvailable(int, QSize);
   void imageUpdated(int);
   void settingsUpdated();

private:
   TextureProject* project;
   QVBoxLayout* layout;
   CubeWidget* cubeWidget;
   CubeWidget* tiledCubeWidget;
   int currId;
   QSize imageSize;
   bool validImage;
};


/**
 * @brief The CubeWidget class
 */
class CubeWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
   Q_OBJECT
   struct VertexData {
      QVector3D position;
      QVector2D texCoord;
   };

public:
    explicit CubeWidget(QWidget *parent = 0);
    ~CubeWidget();
   void setTexture(const QPixmap &pixmap);
   void setTiledTexture(const QPixmap &pixmap);

public slots:
   void imageUpdated();
   void setBackgroundColor(QColor);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void timerEvent(QTimerEvent *e) override;
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void initShaders();
private:
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;
    QOpenGLTexture *texture;
    QOpenGLTexture *emptytexture;
    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    bool textureAvailable;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QColor backgroundcolor;
    bool initialized;
};


#endif // PREVIEW3DPANEL_H
