
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/settingsmanager.h"
#include "base/textureproject.h"
#include "global.h"
#include "gui/cubewidget.h"
#include "gui/previewimagepanel.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

ImageLabel::ImageLabel(QWidget* parent) : QWidget(parent) {
   label = new QLabel(this);
   label->setPixmap(QPixmap(0, 0));
   label->setScaledContents(true);
   label->setFixedSize(0, 0);
   label->setObjectName("previewImage");
   label->setFrameShape(QFrame::NoFrame);

   QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   sizePolicy.setHeightForWidth(true);
   setSizePolicy(sizePolicy);
}

void ImageLabel::resizeEvent(QResizeEvent* event) {
   QWidget::resizeEvent(event);
   resizeImage();
}

void ImageLabel::setPixmap(const QPixmap& pixmap) {
   label->setPixmap(pixmap);
   resizeImage();
}

void ImageLabel::resizeImage() {
   QSize pixSize = label->pixmap(Qt::ReturnByValue).size();
   pixSize.scale(size(), Qt::KeepAspectRatio);
   label->setFixedSize(pixSize);
}

PreviewImagePanel::PreviewImagePanel(TextureProject& project) : project(project) {
   imageSize = project.getThumbnailSize();

   auto* layout = new QVBoxLayout(this);
   setLayout(layout);
   setMaximumWidth(500);
   setMinimumWidth(200);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
   imageLabel = new ImageLabel();
   cubeWidget = new CubeWidget();

   combobox = new QComboBox;
   combobox->addItem("1x1", 1);
   combobox->addItem("2x2", 2);
   combobox->addItem("3x3", 3);
   combobox->addItem("4x4", 4);
   combobox->setCurrentIndex(0);
   QObject::connect(combobox,
                    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                    [=](int i) {
                       Q_UNUSED(i);
                       this->settingsUpdated();
                    });

   lockNodeButton = new QPushButton("Lock node");
   lockNodeButton->setCheckable(true);
   layout->addWidget(cubeWidget, 1);
   layout->addWidget(imageLabel, 1);

   QWidget* optionsWidget = new QWidget;
   auto* optionsLayout = new QHBoxLayout();
   optionsLayout->setContentsMargins(0, 0, 0, 0);
   optionsWidget->setLayout(optionsLayout);
   optionsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
   optionsLayout->addWidget(lockNodeButton);
   optionsLayout->addWidget(combobox);
   layout->addWidget(optionsWidget);
   QObject::connect(&project, &TextureProject::imageAvailable, this,
                    &PreviewImagePanel::imageAvailable);
   QObject::connect(&project, &TextureProject::imageUpdated, this,
                    &PreviewImagePanel::imageUpdated);
   QObject::connect(&project, &TextureProject::nodeRemoved, this, &PreviewImagePanel::nodeRemoved);
   QObject::connect(project.getSettingsManager(), &SettingsManager::settingsUpdated, this,
                    &PreviewImagePanel::settingsUpdated);
   settingsUpdated();
}

void PreviewImagePanel::imageUpdated(int id) {
   if (id != currId) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
}

bool PreviewImagePanel::loadNodeImage(int id) {
   TextureNodePtr texNode = project.getNode(id);
   if (texNode.isNull()) {
      return false;
   }
   imageSize = project.getThumbnailSize();
   const TextureImagePtr image = texNode->cachedImage(imageSize);
   if (image.isNull()) {
      return false;
   }
   const TextureImage& texture = *image;
   QPixmap newImage = QPixmap::fromImage(texture.toQImageView());
   if (numTiles > 1) {
      newImage = tilePixmap(newImage, numTiles);
   }
   imageLabel->setPixmap(newImage);
   imageLabel->show();
   cubeWidget->show();
   cubeWidget->setTexture(newImage);
   return true;
}

void PreviewImagePanel::imageAvailable(int id, QSize size) {
   if (id != currId || size != imageSize) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
   loadNodeImage(id);
}

void PreviewImagePanel::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   if (!loadNodeImage(currId)) {
      imageLabel->hide();
      cubeWidget->hide();
   }
}

void PreviewImagePanel::setActiveNode(int id) {
   if (lockNodeButton->isChecked()) {
      return;
   }
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

void PreviewImagePanel::nodeRemoved(int id) {
   if (currId != id) {
      return;
   }
   lockNodeButton->setChecked(false);
   imageLabel->hide();
   cubeWidget->hide();
}

QPixmap PreviewImagePanel::tilePixmap(const QPixmap& pixmap, int number) {
   QPixmap newPixmap(pixmap.size() * number);
   newPixmap.fill(QColor(255, 255, 255, 255));
   QPainter painter(&newPixmap);
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter.fillRect(0, 0, newPixmap.width(), newPixmap.height(), QBrush(pixmap));
   return newPixmap;
}

void PreviewImagePanel::settingsUpdated() {
   QColor bg = project.getSettingsManager()->getPreviewBackgroundColor();
   cubeWidget->setBackgroundColor(bg);
   int newNumTiles = combobox->currentData().toInt();
   if (numTiles != newNumTiles) {
      numTiles = combobox->currentData().toInt();
      loadNodeImage(currId);
   }
}
