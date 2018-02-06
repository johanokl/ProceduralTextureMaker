/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QComboBox>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include "global.h"
#include "core/textureproject.h"
#include "core/settingsmanager.h"
#include "gui/cubewidget.h"
#include "gui/previewimagepanel.h"

/**
 * @brief ImageLabel::ImageLabel
 * @param parent
 */
ImageLabel::ImageLabel(QWidget *parent) :
   QWidget(parent)
{
   label = new QLabel(this);
   label->setPixmap(QPixmap(0, 0));
   label->setScaledContents(true);
   label->setFixedSize(0, 0);
   label->setFrameStyle(QFrame::Panel | QFrame::Sunken);

   QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   sizePolicy.setHeightForWidth(true);
   setSizePolicy(sizePolicy);
}

/**
 * @brief ImageLabel::resizeEvent
 * @param event
 * Called by Qt when widget's size changed.
 * Updates the image transform to match the new size.
 */
void ImageLabel::resizeEvent(QResizeEvent *event)
{
   QWidget::resizeEvent(event);
   resizeImage();
}

/**
 * @brief ImageLabel::pixmap
 * @return the widget's pixmap visible at the moment.
 */
const QPixmap* ImageLabel::pixmap() const
{
   return label->pixmap();
}

/**
 * @brief ImageLabel::setPixmap
 * @param pixmap background image
 * Sets the pixmap to fill the whole widget.
 */
void ImageLabel::setPixmap(const QPixmap &pixmap)
{
   label->setPixmap(pixmap);
   resizeImage();
}

/**
 * @brief ImageLabel::resizeImage
 * Updates the image rescale transform.
 * Calling it multiple times doesn't lead to artifacts.
 */
void ImageLabel::resizeImage() {
   QSize pixSize = label->pixmap()->size();
   pixSize.scale(size(), Qt::KeepAspectRatio);
   label->setFixedSize(pixSize);
}

/**
 * @brief PreviewImagePanel::PreviewImagePanel
 * @param project
 */
PreviewImagePanel::PreviewImagePanel(TextureProject* project)
{
   this->project = project;
   imageSize = project->getThumbnailSize();
   currId = -1;
   QObject::connect(project, SIGNAL(imageAvailable(int, QSize)),
                    this, SLOT(imageAvailable(int, QSize)));
   QObject::connect(project, SIGNAL(imageUpdated(int)),
                    this, SLOT(imageUpdated(int)));
   QObject::connect(project->getSettingsManager(), SIGNAL(settingsUpdated(void)),
                    this, SLOT(settingsUpdated(void)));
   layout = new QVBoxLayout(this);
   numTiles = 1;
   setLayout(layout);
   setMaximumWidth(500);
   setMinimumWidth(200);
   imageLabel = new ImageLabel();
   cubeWidget = new CubeWidget();

   combobox = new QComboBox;
   combobox->addItem("1x1", 1);
   combobox->addItem("2x2", 2);
   combobox->addItem("3x3", 3);
   combobox->addItem("4x4", 4);
   combobox->setCurrentIndex(0);
   QObject::connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsUpdated()));

   QWidget *spacerWidget = new QWidget();
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   spacerWidget->setVisible(true);
   layout->addWidget(cubeWidget);
   layout->addWidget(imageLabel);
   layout->addWidget(spacerWidget);
   layout->addWidget(combobox);
   settingsUpdated();
}

/**
 * @brief PreviewImagePanel::imageUpdated
 * @param id Node id
 * Removes the no longer valid image from the pixmap widgets.
 */
void PreviewImagePanel::imageUpdated(int id)
{
   if (id != currId) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
}

/**
 * @brief PreviewImagePanel::loadNodeImage
 * @param id Node id
 * @return true if could load image
 * Checks if there is an image with the thumbnail size in the node's
 * texture cache and if found displays it in the pixmap widgets.
 */
bool PreviewImagePanel::loadNodeImage(int id)
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
   if (numTiles > 1) {
      newImage = tilePixmap(newImage, numTiles);
   }
   imageLabel->setPixmap(newImage);
   cubeWidget->setTexture(newImage);
   imageLabel->show();
   cubeWidget->show();
   return true;
}

/**
 * @brief PreviewImagePanel::imageAvailable
 * @param id Node id
 * @param size Image size
 * If the panel is visible loads a new image to the pixmap widgets.
 */
void PreviewImagePanel::imageAvailable(int id, QSize size)
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
 * @brief PreviewImagePanel::showEvent
 * @param event
 * Load the images when the panel is opened.
 */
void PreviewImagePanel::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);
   if (!loadNodeImage(currId)) {
      imageLabel->hide();
      cubeWidget->hide();
   }
}

/**
 * @brief PreviewImagePanel::setActiveNode
 * @param id Node id
 * Updates the images.
 */
void PreviewImagePanel::setActiveNode(int id)
{
   if (currId == id) {
      return;
   }
   currId = id;
   if (this->isHidden()) {
      return;
   }
   imageLabel->hide();
   cubeWidget->hide();
   loadNodeImage(id);
}

/**
 * @brief PreviewImagePanel::tilePixmap
 * @param pixmap Image
 * Draws the image tiled 2*2 times.
 */
QPixmap PreviewImagePanel::tilePixmap(const QPixmap &pixmap, int number)
{
   QPixmap newPixmap(pixmap.size() * number);
   newPixmap.fill(QColor(255, 255, 255, 255));
   QPainter painter(&newPixmap);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, newPixmap.width(), newPixmap.height(), QBrush(pixmap));
   return newPixmap;
}

/**
 * @brief PreviewImagePanel::settingsUpdated
 * Called whenever a project setting has been changed.
 * Might not just be the background color.
 */
void PreviewImagePanel::settingsUpdated()
{
   QColor bg = project->getSettingsManager()->getPreviewBackgroundColor();
   cubeWidget->setBackgroundColor(bg);
   int newNumTiles = combobox->currentData().toInt();
   if (numTiles != newNumTiles) {
      numTiles = combobox->currentData().toInt();
      loadNodeImage(currId);
   }
}
