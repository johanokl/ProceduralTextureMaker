/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PREVIEWIMAGEPANEL_H
#define PREVIEWIMAGEPANEL_H

#include <QWidget>
#include <QMap>

class TextureProject;
class QGridLayout;
class QVBoxLayout;
class QGroupBox;
class QPushButton;
class ImageLabel;
class QLabel;
class QResizeEvent;
class QShowEvent;

/**
 * @brief The PreviewImagePanel class
 *
 * Vertical panel that displays a node's generated image.
 * When an active node's image is updated this also propagates
 * to the panel, which then updates the displayed image.
 */
class PreviewImagePanel : public QWidget
{
   Q_OBJECT

public:
   PreviewImagePanel(TextureProject*);
   virtual ~PreviewImagePanel() {}
   virtual void showEvent(QShowEvent* event);
   bool loadNodeImage(int);

public slots:
   void setActiveNode(int id);
   void imageAvailable(int, QSize);
   void imageUpdated(int);

private:
   TextureProject* project;
   QVBoxLayout* layout;
   ImageLabel* imageLabel;
   ImageLabel* tiledImageLabel;
   int currId;
   QSize imageSize;
   bool validImage;
};

/**
 * @brief The ImageLabel class
 *
 * QLabel with added functions to handle automatic
 * scaling of QPixmap images to fit them in inside a QWidget.
 * Supports 2x2 tiled drawing.
 */
class ImageLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget* parent = 0);
    const QPixmap* pixmap() const;
public slots:
    void setPixmap(const QPixmap&);
    void setTiledPixmap(const QPixmap&);
protected:
    void resizeEvent(QResizeEvent*) ;//override;
private slots:
    void resizeImage();
private:
    QLabel* label;
};

#endif // PREVIEWIMAGEPANEL_H
