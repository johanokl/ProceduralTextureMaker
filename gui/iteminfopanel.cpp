/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPushButton>
#include <QVBoxLayout>
#include "core/textureproject.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/connectionwidget.h"
#include "gui/sceneinfowidget.h"

/**
 * @brief ItemInfoPanel::ItemInfoPanel
 * @param parent MainWindow parent
 * @param project
 */
ItemInfoPanel::ItemInfoPanel(QWidget* parent, TextureProject* project) : QWidget(parent)
{
   texproject = project;
   currWidget = NULL;
   lineWidget = NULL;
   sceneWidget = NULL;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(0, 0, 0);
   sceneWidget = new SceneInfoWidget(this);
   lineWidget = new ConnectionWidget(this);
   QVBoxLayout* layout = new QVBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(lineWidget);
   layout->addWidget(sceneWidget);
   setLayout(layout);
   lineWidget->hide();
   sceneWidget->show();
   QObject::connect(texproject, &TextureProject::nodeRemoved,
                    this, &ItemInfoPanel::removeNode);
   QObject::connect(texproject, &TextureProject::nodesDisconnected,
                    this, &ItemInfoPanel::nodesDisconnected);
   QObject::connect(texproject, &TextureProject::nodeAdded,
                    this, &ItemInfoPanel::addNode);
}

/**
 * @brief ItemInfoPanel::addNode
 *
 * Called when a node has been added to the graph.
 */
void ItemInfoPanel::addNode(TextureNodePtr)
{
   // Update the scene info widget's labels.
   sceneWidget->updateNumNodes();
}

/**
 * @brief ItemInfoPanel::nodeDisconnected
 * @param sourceNodeId Source node's id.
 * @param receiverNodeId Receiver node's id.
 * @param slot Slot id
 *
 * Called when lines have been disconnected.
 */
void ItemInfoPanel::nodesDisconnected(int sourceNodeId, int receiverNodeId, int slot)
{
   if (currLine == std::tuple<int, int, int>(sourceNodeId, receiverNodeId, slot)) {
      // The removed connection is currently visible
      setActiveNode(-1);
   }
}

/**
 * @brief ItemInfoPanel::removeNode
 * @param id Node id
 *
 * Called when a node has been removed from the graph.
 */
void ItemInfoPanel::removeNode(int id)
{
   if (currNodeId == id) {
      // The removed node is currently visible
      setActiveNode(0);
   }
   if (nodes.value(id)) {
      NodeSettingsWidget* currWidget = nodes[id];
      currWidget->hide();
      layout()->removeWidget(currWidget);
      delete currWidget;
      nodes.remove(id);
   }
   sceneWidget->updateNumNodes();
}

/**
 * @brief ItemInfoPanel::sourceUpdated
 * @param id Node id
 *
 * Called when the slot widget for a node needs to be updated.
 */
void ItemInfoPanel::sourceUpdated(int id)
{
   if (nodes.value(id)) {
      nodes.value(id)->slotsUpdated();
   }
}

/**
 * @brief ItemInfoPanel::setActiveNode
 * @param id Node id
 *
 * Displays a widget with info and properties for a node or the scene.
 * If no info widget exists for a node a new one is created and stored
 * in a cache for faster display the next time.
 */
void ItemInfoPanel::setActiveNode(int id)
{
   if (currNodeId == id) {
      return;
   }
   if (currWidget) {
      currWidget->hide();
   }
   lineWidget->hide();
   currWidget = NULL;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(0, 0, 0);

   TextureNodePtr texNode = texproject->getNode(id);
   if (texNode.isNull()) {
      sceneWidget->show();
      return;
   }
   sceneWidget->hide();
   if (!nodes.value(id)) {
      NodeSettingsWidget* newWidget = new NodeSettingsWidget(this, id);
      QObject::connect(texNode.data(), &TextureNode::slotsUpdated,
                       newWidget, &NodeSettingsWidget::slotsUpdated);
      QObject::connect(texNode.data(), &TextureNode::generatorUpdated,
                       newWidget, &NodeSettingsWidget::generatorUpdated);
      nodes[id] = newWidget;
      layout()->addWidget(newWidget);
   }

   currNodeId = id;
   currWidget = nodes[id];
   currWidget->show();
}

/**
 * @brief ItemInfoPanel::setActiveLine
 * @param sourceNodeId
 * @param receiverNodeId
 * @param slot Receiver's slot id
 *
 * Displays the connection widget with data for the selected node.
 */
void ItemInfoPanel::setActiveLine(int sourceNodeId, int receiverNodeId, int slot)
{
   if (sourceNodeId == -1 || receiverNodeId == -1) {
      return;
   }
   if (currWidget) {
      currWidget->hide();
   }
   sceneWidget->hide();
   currWidget = NULL;
   currNodeId = 0;
   currLine = std::tuple<int, int, int>(sourceNodeId, receiverNodeId, slot);
   lineWidget->setNodes(sourceNodeId, receiverNodeId, slot);
   lineWidget->show();
}
