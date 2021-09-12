/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef PREVIEWIMAGEPANEL_H
#define PREVIEWIMAGEPANEL_H

#include <QWidget>

class TextureProject;
class QGridLayout;
class QVBoxLayout;
class QGroupBox;
class QPushButton;
class ImageLabel;
class QLabel;
class QComboBox;
class CubeWidget;
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
   explicit PreviewImagePanel(TextureProject*);
   ~PreviewImagePanel() override = default;
   void showEvent(QShowEvent* event) override;
   bool loadNodeImage(int);

public slots:
   void setActiveNode(int);
   void imageAvailable(int, QSize);
   void imageUpdated(int);
   void settingsUpdated();
   void nodeRemoved(int);

private:
   QPixmap tilePixmap(const QPixmap& pixmap, int number);
   TextureProject* project;
   QVBoxLayout* layout;
   QComboBox* combobox;
   ImageLabel* imageLabel;
   CubeWidget* cubeWidget;
   QPushButton* lockNodeButton;
   int currId;
   int numTiles;
   QSize imageSize;
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
    explicit ImageLabel(QWidget* parent = nullptr);
public slots:
    void setPixmap(const QPixmap&);
protected:
    void resizeEvent(QResizeEvent*) override;
private slots:
    void resizeImage();
private:
    QLabel* label;
};

#endif // PREVIEWIMAGEPANEL_H
