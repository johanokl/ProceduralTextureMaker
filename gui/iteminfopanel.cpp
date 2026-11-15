
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "base/settingsmanager.h"
#include "gui/connectionwidget.h"
#include "gui/generatorinfowidget.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/previewimagepanel.h"
#include "gui/sceneinfowidget.h"
#include "gui/texturebackground.h"
#include "gui/hovertooltip.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QSettings>
#include <QVBoxLayout>

namespace {

/// @brief Toggles the inspector image between full and half size when clicked.
class NodePreviewImage : public ImageLabel {
public:
   /// @brief Restores the preview size and adds a delayed popup that follows the pointer.
   /// @param parent Parent widget.
   explicit NodePreviewImage(QWidget* parent) : ImageLabel(parent) { new HoverTooltip(this); }

   /// @brief Returns the preferred image size for the selected scale.
   QSize sizeHint() const override { return ImageLabel::sizeHint() / (halfSize ? 2 : 1); }

   /// @brief Reserves half the image height when the smaller preview is selected.
   /// @param width Available preview width.
   int heightForWidth(int width) const override {
      return ImageLabel::heightForWidth(halfSize ? width / 2 : width);
   }

protected:
   /// @brief Switches and saves the preview size on a left click.
   /// @param event Mouse press event.
   void mousePressEvent(QMouseEvent* event) override {
      if (event->button() == Qt::LeftButton) {
         halfSize = !halfSize;
         QSettings().setValue("nodePreviewHalfSize", halfSize);
         updateGeometry();
         event->accept();
      } else {
         ImageLabel::mousePressEvent(event);
      }
   }

private:
   /// @brief Whether the image uses half the available width and height.
   bool halfSize{QSettings().value("nodePreviewHalfSize", false).toBool()};
};

}  // namespace

ItemInfoPanel::ItemInfoPanel(QWidget* parent, TextureProject* project, EditManager* editManager)
    : QWidget(parent), texproject(project), editManager(editManager) {
   sceneWidget = new SceneInfoWidget(*this);
   lineWidget = new ConnectionWidget(*this);
   generatorWidget = new GeneratorInfoWidget(this);
   auto* layout = new QVBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(lineWidget, 1);
   layout->addWidget(sceneWidget, 1);
   layout->addWidget(generatorWidget, 1);
   previewButton = new QPushButton(tr("Preview"), this);
   previewButton->setObjectName("showNodePreviewButton");
   previewButton->setCheckable(true);
   previewButton->setChecked(true);
   previewButton->setToolTip(tr("Show or hide the selected node's 2D preview."));
   layout->addWidget(previewButton);
   nodePreview = new NodePreviewImage(this);
   nodePreview->setCursor(Qt::PointingHandCursor);
   nodePreview->setObjectName("inspectorNodePreview");
   layout->addWidget(nodePreview);
   setLayout(layout);
   lineWidget->hide();
   generatorWidget->hide();
   sceneWidget->show();
   updateNodePreview();
   connect(previewButton, &QPushButton::toggled, this, &ItemInfoPanel::updateNodePreview);
   connect(texproject, &TextureProject::imageAvailable, this, [this](int id, QSize size) {
      if (id == currNodeId && size == texproject->getThumbnailSize()) {
         updateNodePreview();
      }
   });
   connect(texproject, &TextureProject::imageUpdated, this, [this](int id) {
      if (id == currNodeId) {
         updateNodePreview();
      }
   });
   connect(texproject, &TextureProject::thumbnailRenderStatusChanged, this, [this](int id) {
      if (id == currNodeId) {
         updateNodePreview();
      }
   });
   if (texproject->getSettingsManager() != nullptr) {
      connect(texproject->getSettingsManager(), &SettingsManager::settingsUpdated, this,
              &ItemInfoPanel::updateNodePreview);
   }
   QObject::connect(texproject, &TextureProject::nodeRemoved, this, &ItemInfoPanel::removeNode);
   QObject::connect(texproject, &TextureProject::nodesDisconnected, this,
                    &ItemInfoPanel::nodesDisconnected);
   QObject::connect(texproject, &TextureProject::nodeAdded, this, &ItemInfoPanel::addNode);
   QObject::connect(texproject, &TextureProject::generatorRemoved, this,
                    [this](const TextureGeneratorPtr& generator) {
                       if (currGenerator == generator) {
                          setActiveNode(0);
                       }
                    });
}

void ItemInfoPanel::addNode(const TextureNodePtr& node) {
   // Update the scene info widget's labels.
   sceneWidget->updateNumNodes();
   QObject::connect(node.data(), &TextureNode::nameUpdated, this, [this, node](int) {
      QSetIterator<int> receivers = node->getReceivers();
      while (receivers.hasNext()) {
         sourceUpdated(receivers.next());
      }
   });
}

void ItemInfoPanel::nodesDisconnected(int sourceNodeId, int receiverNodeId, QString slot) {
   if (currLine == std::tuple<int, int, QString>(sourceNodeId, receiverNodeId, slot)) {
      // The removed connection is currently visible
      setActiveNode(-1);
   }
}

void ItemInfoPanel::removeNode(int id) {
   if (currNodeId == id) {
      // The removed node is currently visible
      setActiveNode(0);
   }
   if (nodes.value(id) != nullptr) {
      NodeSettingsWidget* widget = nodes[id].data();
      widget->hide();
      layout()->removeWidget(widget);
      delete widget;
      nodes.remove(id);
   }
   sceneWidget->updateNumNodes();
}

void ItemInfoPanel::sourceUpdated(int id) {
   NodeSettingsWidget* nodeptr = nodes.value(id).data();
   if (nodeptr != nullptr) {
      nodeptr->slotsUpdated();
   }
}

void ItemInfoPanel::setActiveNode(int id) {
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   lineWidget->hide();
   generatorWidget->hide();
   currGenerator.clear();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(0, 0, QString());
   nodePreview->setPixmap({});
   updateNodePreview();

   TextureNodePtr texNode = texproject->getNode(id);
   if (texNode.isNull()) {
      sceneWidget->show();
      return;
   }
   sceneWidget->hide();
   if (!nodes.contains(id)) {
      auto* newWidget = new NodeSettingsWidget(*this, id);
      QObject::connect(texNode.data(), &TextureNode::slotsUpdated, newWidget,
                       &NodeSettingsWidget::slotsUpdated);
      QObject::connect(texNode.data(), &TextureNode::generatorUpdated, newWidget,
                       &NodeSettingsWidget::generatorUpdated);
      QObject::connect(texNode.data(), &TextureNode::settingsUpdated, newWidget,
                       &NodeSettingsWidget::settingsUpdated);
      QObject::connect(texNode.data(), &TextureNode::nameUpdated, newWidget,
                       &NodeSettingsWidget::settingsUpdated);
      nodes[id] = newWidget;
      static_cast<QVBoxLayout*>(layout())->insertWidget(0, newWidget, 1);
   }
   currNodeId = id;
   currWidget = nodes[id];
   currWidget->show();
   updateNodePreview();
}

void ItemInfoPanel::setActiveLine(int sourceNodeId, int receiverNodeId, QString slot) {
   if (sourceNodeId == -1 || receiverNodeId == -1) {
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   sceneWidget->hide();
   generatorWidget->hide();
   currGenerator.clear();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(sourceNodeId, receiverNodeId, slot);
   updateNodePreview();
   lineWidget->setNodes(sourceNodeId, receiverNodeId, slot);
   lineWidget->show();
}

void ItemInfoPanel::setActiveGenerator(const TextureGeneratorPtr& generator) {
   if (generator.isNull()) {
      setActiveNode(0);
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   sceneWidget->hide();
   lineWidget->hide();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(0, 0, QString());
   currGenerator = generator;
   updateNodePreview();
   generatorWidget->setGenerator(generator);
   generatorWidget->show();
}

void ItemInfoPanel::updateNodePreview() {
   const TextureNodePtr node = texproject->getNode(currNodeId);
   previewButton->setVisible(!node.isNull());
   const bool visible = !node.isNull() && previewButton->isChecked();
   nodePreview->setVisible(visible);
   if (!visible) {
      return;
   }
   const SettingsManager* settings = texproject->getSettingsManager();
   const TextureImagePtr image = node->cachedImage(texproject->getThumbnailSize());
   if (!image.isNull()) {
      QPixmap pixmap = QPixmap::fromImage(image->toQImageView());
      if (settings != nullptr) {
         pixmap = TextureBackground::composite(pixmap, settings->getNodeBackgroundColor(),
                                               settings->getNodeBackgroundBrushColor(),
                                               Qt::BrushStyle(settings->getNodeBackgroundBrush()));
      }
      nodePreview->setPixmap(pixmap);
   }
   if (settings != nullptr) {
      nodePreview->setSmoothFiltering(settings->getTextureFiltering() ==
                                      SettingsManager::TextureFiltering::Smooth);
   }
   nodePreview->setRenderStatus(texproject->thumbnailRenderStatus(currNodeId));
   const QString error = nodePreview->toolTip();
   nodePreview->setToolTip(tr("Click to change size") +
                           (error.isEmpty() ? QString() : "\n\n" + error));
}
