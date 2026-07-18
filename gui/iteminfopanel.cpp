
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

/// @brief ItemInfoPanel constructor.
/// @param parent MainWindow parent
/// @param project Pointer to the TextureProject that this widget will display info for.
ItemInfoPanel::ItemInfoPanel(QWidget* parent, TextureProject* project) : QWidget(parent) {
   texproject = project;
   currWidget = nullptr;
   lineWidget = nullptr;
   sceneWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(0, 0, 0);
   sceneWidget = new SceneInfoWidget(this);
   lineWidget = new ConnectionWidget(this);
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

/// @brief Called when a node has been added to the graph.
/// @param unused The added node.
void ItemInfoPanel::addNode(const TextureNodePtr&) {
   // Update the scene info widget's labels.
   sceneWidget->updateNumNodes();
}

/// @brief Called when lines have been disconnected.
/// @param sourceNodeId Source node's id.
/// @param receiverNodeId Receiver node's id.
/// @param slot Slot id
void ItemInfoPanel::nodesDisconnected(int sourceNodeId, int receiverNodeId, int slot) {
   if (currLine == std::tuple<int, int, int>(sourceNodeId, receiverNodeId, slot)) {
      // The removed connection is currently visible
      setActiveNode(-1);
   }
}

/// @brief Called when a node has been removed from the graph.
/// @param id Node id
void ItemInfoPanel::removeNode(int id) {
   if (currNodeId == id) {
      // The removed node is currently visible
      setActiveNode(0);
   }
   if (nodes.value(id) != nullptr) {
      NodeSettingsWidget* currWidget = nodes[id];
      currWidget->hide();
      layout()->removeWidget(currWidget);
      delete currWidget;
      nodes.remove(id);
   }
   sceneWidget->updateNumNodes();
}

/// @brief Called when the slot widget for a node needs to be updated.
/// @param id Node id
void ItemInfoPanel::sourceUpdated(int id) {
   auto* nodeptr = nodes.value(id, nullptr);
   if (nodeptr != nullptr) {
      nodeptr->slotsUpdated();
   }
}

/// @brief Displays a widget with info and properties for a node or the scene.
/// @details If no info widget exists for a node a new one is created and stored
/// in a cache for faster display the next time.
/// @param id Node id
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
      auto* newWidget = new NodeSettingsWidget(this, id);
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

/// @brief Displays the connection widget with data for the selected node.
/// @param sourceNodeId
/// @param receiverNodeId
/// @param slot Receiver's slot id
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
