/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

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
    QOpenGLTexture *texture;
    bool textureUpdated;
    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    double angularSpeed;
    QQuaternion rotation;
    bool textureAvailable;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QColor backgroundcolor;
    bool initialized;
};


#endif // PREVIEW3DPANEL_H
