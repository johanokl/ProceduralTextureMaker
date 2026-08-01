
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/connectionwidget.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/sceneinfowidget.h"
#include <QPushButton>
#include <QVBoxLayout>

ItemInfoPanel::ItemInfoPanel(QWidget* parent, TextureProject* project)
    : QWidget(parent), texproject(project) {
   sceneWidget = new SceneInfoWidget(*this);
   lineWidget = new ConnectionWidget(*this);
   auto* layout = new QVBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(lineWidget);
   layout->addWidget(sceneWidget);
   setLayout(layout);
   lineWidget->hide();
   sceneWidget->show();
   QObject::connect(texproject, &TextureProject::nodeRemoved, this, &ItemInfoPanel::removeNode);
   QObject::connect(texproject, &TextureProject::nodesDisconnected, this,
                    &ItemInfoPanel::nodesDisconnected);
   QObject::connect(texproject, &TextureProject::nodeAdded, this, &ItemInfoPanel::addNode);
}

void ItemInfoPanel::addNode(const TextureNodePtr&) {
   // Update the scene info widget's labels.
   sceneWidget->updateNumNodes();
}

void ItemInfoPanel::nodesDisconnected(int sourceNodeId, int receiverNodeId, int slot) {
   if (currLine == std::tuple<int, int, int>(sourceNodeId, receiverNodeId, slot)) {
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
   if (currNodeId == id) {
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   lineWidget->hide();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(0, 0, 0);

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
      nodes[id] = newWidget;
      layout()->addWidget(newWidget);
   }
   currNodeId = id;
   currWidget = nodes[id];
   currWidget->show();
}

void ItemInfoPanel::setActiveLine(int sourceNodeId, int receiverNodeId, int slot) {
   if (sourceNodeId == -1 || receiverNodeId == -1) {
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   sceneWidget->hide();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(sourceNodeId, receiverNodeId, slot);
   lineWidget->setNodes(sourceNodeId, receiverNodeId, slot);
   lineWidget->show();
}
