/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPushButton>
#include <QSet>
#include <QApplication>
#include <QGroupBox>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "global.h"
#include "core/textureproject.h"
#include "generators/texturegenerator.h"
#include "gui/previewimagepanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/connectionwidget.h"
#include "generators/texturegenerator.h"

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
 * @brief ImageLabel::setTiledPixmap
 * @param pixmap background image
 * Sets a new background iamge for the widget. Downscales the
 * image 50% and draws it tiled 2*2 times.
 */
void ImageLabel::setTiledPixmap(const QPixmap &pixmap)
{
   if (pixmap.width() <= 0 || pixmap.height() <= 0) {
      return;
   }
   QPixmap newPixmap(pixmap.size() * 2);
   QPainter painter(&newPixmap);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, newPixmap.width(), newPixmap.height(), QBrush(pixmap));
   label->setPixmap(newPixmap);
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
   validImage = false;
   currId = -1;
   QObject::connect(project, SIGNAL(imageAvailable(int, QSize)),
                    this, SLOT(imageAvailable(int, QSize)));
   QObject::connect(project, SIGNAL(imageUpdated(int)),
                    this, SLOT(imageUpdated(int)));
   layout = new QVBoxLayout(this);
   setLayout(layout);
   setMaximumWidth(500);
   setMinimumWidth(200);
   imageLabel = new ImageLabel();
   tiledImageLabel = new ImageLabel();
   layout->addWidget(imageLabel);
   layout->addWidget(tiledImageLabel);
   QWidget *spacerWidget = new QWidget();
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   spacerWidget->setVisible(true);
   layout->addWidget(spacerWidget);
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
   validImage = false;
   imageLabel->setPixmap(QPixmap());
   tiledImageLabel->setTiledPixmap(QPixmap());
   update();
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
   imageLabel->setPixmap(newImage);
   tiledImageLabel->setTiledPixmap(newImage);
   imageLabel->show();
   tiledImageLabel->show();
   validImage = true;
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
      tiledImageLabel->hide();
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
   tiledImageLabel->hide();
   loadNodeImage(id);
}

