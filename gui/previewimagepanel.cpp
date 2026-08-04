
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
#include <QEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOptionButton>
#include <QVBoxLayout>
#include <functional>
#include <utility>

namespace {
void paintRenderingGrid(QPainter& painter, const QRect& rect) {
   QBrush updateBrush(QColor(230, 126, 34, 180));
   updateBrush.setStyle(Qt::DiagCrossPattern);
   painter.fillRect(rect, updateBrush);
}

class RenderingOverlay final : public QWidget {
public:
   using QWidget::QWidget;

protected:
   void paintEvent(QPaintEvent*) override {
      QPainter painter(this);
      paintRenderingGrid(painter, rect());
   }
};

class LockNodeButton final : public QPushButton {
public:
   using QPushButton::QPushButton;

   QSize sizeHint() const override {
      return QPushButton::sizeHint()
          .expandedTo(styledSizeForLabel(QStringLiteral("Lock node")))
          .expandedTo(styledSizeForLabel(QStringLiteral("Unlock node")));
   }

   QSize minimumSizeHint() const override { return sizeHint(); }

private:
   QSize styledSizeForLabel(const QString& label) const {
      QStyleOptionButton option;
      initStyleOption(&option);
      option.text = label;
      const QSize contentSize = option.fontMetrics.size(Qt::TextShowMnemonic, label);
      return style()->sizeFromContents(QStyle::CT_PushButton, &option, contentSize, this);
   }
};

class PreviewControlsWidget final : public QWidget {
public:
   PreviewControlsWidget(std::function<void()> minimumSizeChanged, QWidget* parent)
       : QWidget(parent), minimumSizeChanged(std::move(minimumSizeChanged)) {}

protected:
   bool event(QEvent* event) override {
      const bool handled = QWidget::event(event);
      if (event->type() == QEvent::LayoutRequest || event->type() == QEvent::FontChange ||
          event->type() == QEvent::StyleChange) {
         minimumSizeChanged();
      }
      return handled;
   }

private:
   std::function<void()> minimumSizeChanged;
};

class PreviewGroupBox final : public QGroupBox {
public:
   explicit PreviewGroupBox(const QString& title, QWidget* parent) : QGroupBox(title, parent) {
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      policy.setHeightForWidth(true);
      setSizePolicy(policy);
   }

   void setPreviewWidget(QWidget* widget) {
      previewWidget = widget;
      previewWidget->setParent(this);
      updateGeometry();
   }

   int heightForWidth(const int width) const override {
      if (previewWidget == nullptr) {
         return QGroupBox::sizeHint().height();
      }
      const QMargins margins = contentsMargins();
      const int contentWidth = qMax(0, width - margins.left() - margins.right());
      const int contentHeight = previewWidget->hasHeightForWidth()
                                    ? previewWidget->heightForWidth(contentWidth)
                                    : previewWidget->sizeHint().height();
      return margins.top() + contentHeight + margins.bottom();
   }

   QSize sizeHint() const override {
      QSize hint = QGroupBox::sizeHint();
      hint.setHeight(heightForWidth(hint.width()));
      return hint;
   }

   void setHeightForWidth(const int width) {
      const int requiredHeight = heightForWidth(width);
      if (height() != requiredHeight) {
         setFixedHeight(requiredHeight);
      }
   }

protected:
   void resizeEvent(QResizeEvent* event) override {
      QGroupBox::resizeEvent(event);
      if (previewWidget == nullptr) {
         return;
      }
      const QRect area = contentsRect();
      const int contentHeight = previewWidget->hasHeightForWidth()
                                    ? previewWidget->heightForWidth(area.width())
                                    : area.height();
      previewWidget->setGeometry(area.x(), area.y(), area.width(), contentHeight);
   }

private:
   QWidget* previewWidget{nullptr};
};

void layoutPreviewList(QWidget* previewList) {
   if (previewList == nullptr || previewList->layout() == nullptr) {
      return;
   }
   previewList->layout()->invalidate();
   previewList->setFixedHeight(previewList->layout()->sizeHint().height());
   previewList->layout()->setGeometry(previewList->rect());
}

class StableWidthScrollArea final : public QScrollArea {
public:
   using QScrollArea::QScrollArea;

   void stabilizeWidgetWidth() {
      if (widget() == nullptr) {
         return;
      }
      const int scrollBarWidth = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
      const int contentWidth = qMax(0, width() - (2 * frameWidth()) - scrollBarWidth);
      widget()->setFixedWidth(contentWidth);
      for (QObject* child : widget()->children()) {
         if (auto* preview = dynamic_cast<PreviewGroupBox*>(child)) {
            preview->setHeightForWidth(contentWidth);
         }
      }
      layoutPreviewList(widget());
   }

protected:
   void resizeEvent(QResizeEvent* event) override {
      QScrollArea::resizeEvent(event);
      stabilizeWidgetWidth();
   }
};
}  // namespace

ImageLabel::ImageLabel(QWidget* parent) : QWidget(parent) {
   label = new QLabel(this);
   label->setPixmap(QPixmap(0, 0));
   label->setScaledContents(true);
   label->setFixedSize(0, 0);
   label->setObjectName("previewImage");
   label->setFrameShape(QFrame::NoFrame);
   renderingOverlay = new RenderingOverlay(this);
   renderingOverlay->setObjectName(QStringLiteral("renderingOverlay"));
   renderingOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
   renderingOverlay->hide();

   QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   sizePolicy.setHeightForWidth(true);
   setSizePolicy(sizePolicy);
}

void ImageLabel::resizeEvent(QResizeEvent* event) {
   QWidget::resizeEvent(event);
   resizeImage();
}

void ImageLabel::setPixmap(const QPixmap& pixmap) {
   label->setPixmap(pixmap);
   setRendering(false);
   updateGeometry();
   if (parentWidget() != nullptr) {
      parentWidget()->updateGeometry();
   }
   resizeImage();
}

void ImageLabel::setRendering(const bool rendering) {
   renderingOverlay->setVisible(rendering);
   if (rendering) {
      renderingOverlay->raise();
   }
}

QSize ImageLabel::sizeHint() const {
   QSize preferredSize = label->pixmap(Qt::ReturnByValue).size();
   if (preferredSize.isEmpty()) {
      return {};
   }
   preferredSize.scale(QSize(256, 256), Qt::KeepAspectRatio);
   return preferredSize;
}

int ImageLabel::heightForWidth(const int width) const {
   const QSize pixmapSize = label->pixmap(Qt::ReturnByValue).size();
   if (width <= 0 || pixmapSize.width() <= 0) {
      return 0;
   }
   return qRound(static_cast<double>(width) * pixmapSize.height() / pixmapSize.width());
}

QPixmap ImageLabel::pixmapWithRenderingOverlay() const {
   QPixmap pixmap = label->pixmap(Qt::ReturnByValue);
   if (pixmap.isNull()) {
      return {};
   }
   QPainter painter(&pixmap);
   paintRenderingGrid(painter, pixmap.rect());
   return pixmap;
}

void ImageLabel::resizeImage() {
   QSize pixSize = label->pixmap(Qt::ReturnByValue).size();
   pixSize.scale(size(), Qt::KeepAspectRatio);
   label->setFixedSize(pixSize);
   renderingOverlay->setGeometry(label->geometry());
   renderingOverlay->raise();
}

PreviewImagePanel::PreviewImagePanel(TextureProject& project) : project(project) {
   imageSize = project.getThumbnailSize();

   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins(6, 14, 6, 6);
   layout->setSpacing(10);
   setMinimumWidth(200);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

   QWidget* optionsWidget =
       new PreviewControlsWidget([this]() { updateControlsMinimumWidth(); }, this);
   optionsWidget->setObjectName(QStringLiteral("previewControls"));
   auto* optionsLayout = new QHBoxLayout(optionsWidget);
   optionsLayout->setContentsMargins(0, 0, 0, 0);
   optionsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

   lockNodeButton = new LockNodeButton(QStringLiteral("Lock node"), optionsWidget);
   lockNodeButton->setObjectName(QStringLiteral("lockNodeButton"));
   lockNodeButton->setCheckable(true);
   optionsLayout->addWidget(lockNodeButton);

   tileCountComboBox = new QComboBox(optionsWidget);
   tileCountComboBox->setObjectName(QStringLiteral("previewTileCount"));
   tileCountComboBox->addItem(QStringLiteral("1x1"), 1);
   tileCountComboBox->addItem(QStringLiteral("2x2"), 2);
   tileCountComboBox->addItem(QStringLiteral("3x3"), 3);
   tileCountComboBox->addItem(QStringLiteral("4x4"), 4);
   tileCountComboBox->setCurrentIndex(0);
   optionsLayout->addWidget(tileCountComboBox);

   showThreeDButton = new QPushButton(QStringLiteral("3D"), optionsWidget);
   showThreeDButton->setObjectName(QStringLiteral("showThreeDButton"));
   showThreeDButton->setCheckable(true);
   showThreeDButton->setChecked(false);
   optionsLayout->addWidget(showThreeDButton);
   optionsLayout->addStretch();
   layout->addWidget(optionsWidget);
   updateControlsMinimumWidth();

   auto* previewScrollArea = new StableWidthScrollArea(this);
   previewScrollArea->setObjectName(QStringLiteral("previewScrollArea"));
   previewScrollArea->setFrameShape(QFrame::NoFrame);
   previewScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   previewScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   previewScrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   previewScrollArea->setWidgetResizable(true);
   auto* previewList = new QWidget(previewScrollArea);
   previewList->setObjectName(QStringLiteral("previewList"));
   previewList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
   auto* previewLayout = new QVBoxLayout(previewList);
   previewLayout->setContentsMargins(0, 0, 0, 0);
   previewLayout->setSpacing(10);
   previewLayout->setAlignment(Qt::AlignTop);
   previewScrollArea->setWidget(previewList);
   previewScrollArea->stabilizeWidgetWidth();
   layout->addWidget(previewScrollArea, 1);

   lockedNodePreview = new PreviewGroupBox(QStringLiteral("Locked node"), previewList);
   lockedNodePreview->setObjectName(QStringLiteral("lockedNodePreview"));
   lockedNodePreview->setProperty("previewSection", true);
   lockedImageLabel = new ImageLabel(lockedNodePreview);
   lockedImageLabel->setObjectName(QStringLiteral("lockedNodeImage"));
   static_cast<PreviewGroupBox*>(lockedNodePreview)->setPreviewWidget(lockedImageLabel);
   lockedNodePreview->hide();
   previewLayout->addWidget(lockedNodePreview);

   selectedNodePreview = new PreviewGroupBox(QStringLiteral("Selected node"), previewList);
   selectedNodePreview->setObjectName(QStringLiteral("selectedNodePreview"));
   selectedNodePreview->setProperty("previewSection", true);
   selectedImageLabel = new ImageLabel(selectedNodePreview);
   selectedImageLabel->setObjectName(QStringLiteral("selectedNodeImage"));
   selectedImageLabel->hide();
   static_cast<PreviewGroupBox*>(selectedNodePreview)->setPreviewWidget(selectedImageLabel);
   previewLayout->addWidget(selectedNodePreview);

   threeDPreview = new PreviewGroupBox(QStringLiteral("3D view"), previewList);
   threeDPreview->setObjectName(QStringLiteral("threeDPreview"));
   threeDPreview->setProperty("previewSection", true);
   cubeWidget = new CubeWidget(threeDPreview);
   cubeWidget->setObjectName(QStringLiteral("previewCube"));
   static_cast<PreviewGroupBox*>(threeDPreview)->setPreviewWidget(cubeWidget);
   threeDPreview->hide();
   previewLayout->addWidget(threeDPreview);

   QObject::connect(tileCountComboBox,
                    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                    [=](int i) {
                       Q_UNUSED(i);
                       this->settingsUpdated();
                    });
   QObject::connect(lockNodeButton, &QPushButton::toggled, this, &PreviewImagePanel::setNodeLocked);
   QObject::connect(showThreeDButton, &QPushButton::toggled, this,
                    &PreviewImagePanel::setThreeDPreviewVisible);
   QObject::connect(&project, &TextureProject::imageAvailable, this,
                    &PreviewImagePanel::imageAvailable);
   QObject::connect(&project, &TextureProject::imageUpdated, this,
                    &PreviewImagePanel::imageUpdated);
   QObject::connect(&project, &TextureProject::nodeRemoved, this, &PreviewImagePanel::nodeRemoved);
   if (project.getSettingsManager() != nullptr) {
      QObject::connect(project.getSettingsManager(), &SettingsManager::settingsUpdated, this,
                       &PreviewImagePanel::settingsUpdated);
   }
   settingsUpdated();
}

void PreviewImagePanel::imageUpdated(int id) {
   if (this->isHidden()) {
      return;
   }
   if (id == selectedNodeId) {
      selectedImageLabel->setRendering(true);
      selectedImageLabel->show();
      const QPixmap staleTexture = selectedImageLabel->pixmapWithRenderingOverlay();
      if (showThreeDButton->isChecked() && !staleTexture.isNull()) {
         cubeWidget->setTexture(staleTexture);
         cubeWidget->show();
      } else {
         cubeWidget->hide();
      }
   }
   if (id == lockedNodeId) {
      lockedImageLabel->setRendering(true);
      lockedImageLabel->show();
   }
   updatePreviewLayout();
}

QPixmap PreviewImagePanel::nodePixmap(int id) {
   TextureNodePtr texNode = project.getNode(id);
   if (texNode.isNull()) {
      return {};
   }
   imageSize = project.getThumbnailSize();
   const TextureImagePtr image = texNode->cachedImage(imageSize);
   if (image.isNull()) {
      return {};
   }
   const TextureImage& texture = *image;
   QPixmap newImage = QPixmap::fromImage(texture.toQImageView());
   if (numTiles > 1) {
      newImage = tilePixmap(newImage, numTiles);
   }
   return newImage;
}

bool PreviewImagePanel::loadSelectedNodeImage() {
   const QPixmap newImage = nodePixmap(selectedNodeId);
   if (newImage.isNull()) {
      return false;
   }
   selectedImageLabel->setPixmap(newImage);
   selectedImageLabel->show();
   cubeWidget->setTexture(newImage);
   cubeWidget->setVisible(showThreeDButton->isChecked());
   updatePreviewLayout();
   return true;
}

bool PreviewImagePanel::loadLockedNodeImage() {
   const QPixmap newImage = nodePixmap(lockedNodeId);
   if (newImage.isNull()) {
      return false;
   }
   lockedImageLabel->setPixmap(newImage);
   lockedImageLabel->show();
   updatePreviewLayout();
   return true;
}

void PreviewImagePanel::imageAvailable(int id, QSize size) {
   if (size != imageSize) {
      return;
   }
   if (this->isHidden()) {
      return;
   }
   if (id == selectedNodeId) {
      loadSelectedNodeImage();
   }
   if (id == lockedNodeId) {
      loadLockedNodeImage();
   }
}

void PreviewImagePanel::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   lockedNodePreview->setVisible(lockedNodeId >= 0);
   if (lockedNodeId >= 0 && !loadLockedNodeImage()) {
      lockedImageLabel->hide();
   }
   if (!loadSelectedNodeImage()) {
      selectedImageLabel->hide();
      cubeWidget->hide();
   }
   threeDPreview->setVisible(showThreeDButton->isChecked());
}

void PreviewImagePanel::setActiveNode(int id) {
   if (selectedNodeId == id) {
      return;
   }
   selectedNodeId = id;
   if (this->isHidden()) {
      return;
   }
   selectedImageLabel->hide();
   cubeWidget->hide();
   loadSelectedNodeImage();
}

void PreviewImagePanel::nodeRemoved(int id) {
   if (selectedNodeId == id) {
      selectedNodeId = -1;
      selectedImageLabel->hide();
      cubeWidget->hide();
   }
   if (lockedNodeId == id) {
      lockNodeButton->setChecked(false);
   }
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
   if (project.getSettingsManager() != nullptr) {
      cubeWidget->setBackgroundColor(project.getSettingsManager()->getPreviewBackgroundColor());
   }
   numTiles = tileCountComboBox->currentData().toInt();
   if (!this->isHidden()) {
      if (!loadSelectedNodeImage()) {
         selectedImageLabel->hide();
         cubeWidget->hide();
      }
      if (lockedNodeId >= 0 && !loadLockedNodeImage()) {
         lockedImageLabel->hide();
      }
   }
}

void PreviewImagePanel::setNodeLocked(const bool locked) {
   const bool restoreUpdates = updatesEnabled();
   if (restoreUpdates) {
      setUpdatesEnabled(false);
   }
   if (!locked) {
      lockedNodeId = -1;
      lockNodeButton->setText(QStringLiteral("Lock node"));
      lockedImageLabel->hide();
      lockedNodePreview->hide();
      updatePreviewLayout();
      if (restoreUpdates) {
         setUpdatesEnabled(true);
         update();
      }
      return;
   }
   if (project.getNode(selectedNodeId).isNull()) {
      const QSignalBlocker blocker(lockNodeButton);
      lockNodeButton->setChecked(false);
      if (restoreUpdates) {
         setUpdatesEnabled(true);
      }
      return;
   }
   lockedNodeId = selectedNodeId;
   lockNodeButton->setText(QStringLiteral("Unlock node"));
   lockedNodePreview->show();
   lockedImageLabel->hide();
   if (!this->isHidden()) {
      loadLockedNodeImage();
   }
   updatePreviewLayout();
   if (restoreUpdates) {
      setUpdatesEnabled(true);
      update();
   }
}

void PreviewImagePanel::setThreeDPreviewVisible(const bool visible) {
   const bool restoreUpdates = updatesEnabled();
   if (restoreUpdates) {
      setUpdatesEnabled(false);
   }
   threeDPreview->setVisible(visible);
   if (!visible) {
      cubeWidget->hide();
   } else {
      cubeWidget->setVisible(loadSelectedNodeImage());
   }
   updatePreviewLayout();
   if (restoreUpdates) {
      setUpdatesEnabled(true);
      update();
   }
}

void PreviewImagePanel::updatePreviewLayout() {
   QWidget* previewList = selectedNodePreview->parentWidget();
   if (previewList == nullptr || previewList->layout() == nullptr) {
      return;
   }
   for (QObject* child : previewList->children()) {
      if (auto* preview = dynamic_cast<PreviewGroupBox*>(child)) {
         preview->setHeightForWidth(previewList->width());
      }
   }
   layoutPreviewList(previewList);
}

void PreviewImagePanel::updateControlsMinimumWidth() {
   if (lockNodeButton == nullptr || lockNodeButton->parentWidget() == nullptr ||
       lockNodeButton->parentWidget()->layout() == nullptr || layout() == nullptr) {
      return;
   }
   QLayout* controlsLayout = lockNodeButton->parentWidget()->layout();
   controlsLayout->invalidate();
   const QMargins margins = layout()->contentsMargins();
   const int requiredWidth =
       controlsLayout->minimumSize().width() + margins.left() + margins.right();
   setMinimumWidth(qMax(200, requiredWidth));
}
