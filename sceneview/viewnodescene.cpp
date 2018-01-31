/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QGraphicsSceneMouseEvent>
#include <QObject>
#include <QMapIterator>
#include <QMenu>
#include <QAction>
#include <tuple>
#include <QMimeData>
#include <QGraphicsView>
#include <QListIterator>
#include "gui/mainwindow.h"
#include "core/textureproject.h"
#include "core/texturenode.h"
#include "core/settingsmanager.h"
#include "sceneview/viewnodescene.h"
#include "sceneview/viewnodeview.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodeline.h"
#include "generators/texturegenerator.h"
#include "gui/menuactions.h"
#include "global.h"

/**
 * @brief ViewNodeScene::ViewNodeScene
 * @param parent
 */
ViewNodeScene::ViewNodeScene(MainWindow* parent)
{
   this->parent = parent;
   this->project = parent->getTextureProject();

   startLineNode = 0;
   lineDrawing = false;
   lineItem = NULL;
   dropItem = NULL;
   selectedNode = -1;

   QObject::connect(project, SIGNAL(nodeAdded(TextureNodePtr)), this, SLOT(addNode(TextureNodePtr)));
   QObject::connect(project, SIGNAL(nodeRemoved(int)), this, SLOT(nodeRemoved(int)));
   QObject::connect(project, SIGNAL(nodesConnected(int, int, int)), this, SLOT(nodesConnected(int, int, int)));
   QObject::connect(project, SIGNAL(nodesDisconnected(int, int, int)), this, SLOT(nodesDisconnected(int, int, int)));
   QObject::connect(project, SIGNAL(nodeRemoved(int)), this, SLOT(nodeRemoved(int)));
   QObject::connect(project->getSettingsManager(), SIGNAL(settingsUpdated(void)), this, SLOT(settingsUpdated(void)));

   QBrush backgroundBrush;
   backgroundBrush.setStyle(Qt::SolidPattern);
   backgroundBrush.setColor(QColor(200, 200, 200));
   setBackgroundBrush(backgroundBrush);
   settingsUpdated();
}

/**
 * @brief ViewNodeScene::~ViewNodeScene
 */
ViewNodeScene::~ViewNodeScene()
{
}

/**
 * @brief ViewNodeScene::clone
 * @return a new instance with the nodes copied.
 * Used for restarting with a new instance with clean QGraphicsScene properties.
 * For now a quick hack for resetting the scene view region properties.
 */
ViewNodeScene* ViewNodeScene::clone()
{
   ViewNodeScene* newscene = new ViewNodeScene(parent);
   QMapIterator<int, ViewNodeItem*> nodesIter(nodeItems);
   while (nodesIter.hasNext()) {
      TextureNodePtr ptr = nodesIter.next().value()->getTextureNode();
      newscene->addNode(ptr);
      newscene->imageAvailable(ptr->getId(), this->getTextureProject()->getThumbnailSize());
   }
   QMapIterator<std::tuple<int, int, int>, ViewNodeLine*> connectionsIter(nodeConnections);
   while (connectionsIter.hasNext()) {
      std::tuple<int, int, int> connection = connectionsIter.next().key();
      newscene->nodesConnected(std::get<0>(connection), std::get<1>(connection), std::get<2>(connection));
   }
   return newscene;
}

/**
 * @brief ViewNodeScene::getTextureProject
 * @return the active TextureProject tied to this scene instance.
 */
TextureProject* ViewNodeScene::getTextureProject()
{
   return project;
}

/**
 * @brief ViewNodeScene::getItem
 * @param id Node id
 * @return the ViewNodeItem if found, NULL if not.
 */
ViewNodeItem* ViewNodeScene::getItem(int id)
{
   if (id < 0) {
      return NULL;
   }
   if (!nodeItems.contains(id)) {
      return NULL;
   }
   return nodeItems.value(id);
}

/**
 * @brief ViewNodeScene::clear
 * Clears the active TextureProject, removing all nodes.
 */
void ViewNodeScene::clear()
{
   project->clear();
}

/**
 * @brief ViewNodeScene::nodesConnected
 * @param sourceid Node id
 * @param receiverid Node id
 * @param slot
 * Called when nodes are connected. Adds a line between the nodes to the scene.
 */
void ViewNodeScene::nodesConnected(int sourceid, int receiverid, int slot)
{
   ViewNodeItem* sourceNode = nodeItems.value(sourceid);
   ViewNodeItem* receiverNode = nodeItems.value(receiverid);
   if (!sourceNode || !receiverNode) {
      return;
   }
   std::tuple<int, int, int> key(sourceid, receiverid, slot);
   if (nodeConnections.contains(key)) {
      return;
   }
   ViewNodeLine* newLine = new ViewNodeLine(this, sourceid, receiverid, slot);
   sourceNode->addConnectionLine(newLine);
   receiverNode->addConnectionLine(newLine);
   newLine->update();
   nodeConnections.insert(key, newLine);
}

/**
 * @brief ViewNodeScene::nodesDisconnected
 * @param sourceid Node id
 * @param receiverid Node id
 * @param slot
 * Called when nodes are disconnected. Removes the scene's line between the nodes.
 */
void ViewNodeScene::nodesDisconnected(int sourceid, int receiverid, int slot)
{
   auto key = std::tuple<int, int, int>(sourceid, receiverid, slot);
   if (!nodeConnections.contains(key)) {
      return;
   }
   ViewNodeLine* line = nodeConnections.value(key);
   nodeConnections.remove(key);
   ViewNodeItem* sourceNode = nodeItems.value(sourceid);
   ViewNodeItem* receiverNode = nodeItems.value(receiverid);
   if (sourceNode) {
      sourceNode->removeConnectionLine(line);
   }
   if (receiverNode) {
      receiverNode->removeConnectionLine(line);
   }
   delete line;
}

/**
 * @brief ViewNodeScene::addNode
 * @param newNode
 */
void ViewNodeScene::addNode(TextureNodePtr newNode)
{
   ViewNodeItem* newItem = new ViewNodeItem(this, newNode);
   addItem(newItem);
   nodeItems.insert(newNode->getId(), newItem);
   newItem->settingsUpdated();
   newItem->imageUpdated();
   QObject::connect(newNode.data(), SIGNAL(positionUpdated(int)), this, SLOT(positionUpdated(int)));
   QObject::connect(newNode.data(), SIGNAL(settingsUpdated(int)), this, SLOT(nodeSettingsUpdated(int)));
   QObject::connect(newNode.data(), SIGNAL(imageUpdated(int)), this, SLOT(imageUpdated(int)));
   QObject::connect(newNode.data(), SIGNAL(imageAvailable(int, QSize)), this, SLOT(imageAvailable(int, QSize)));
   QObject::connect(newNode.data(), SIGNAL(generatorUpdated(int)), this, SLOT(generatorUpdated(int)));
   update();
}

/**
 * @brief ViewNodeScene::positionUpdated
 * @param id
 */
void ViewNodeScene::positionUpdated(int id)
{
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->positionUpdated();
   }
}

/**
 * @brief ViewNodeScene::setSelectedNode
 * @param id
 * Highlights a specific node and sends a signal to the affected widgets.
 */
void ViewNodeScene::setSelectedNode(int id)
{
   selectedNode = id;
   emit nodeSelected(id);
}

/**
 * @brief ViewNodeScene::setSelectedLine
 * @param sourceNode The line's start node id.
 * @param receiverNode The line's end node id.
 * @param slot Source slot number.
 * Highlights a specific line between nodes and sends a
 * signal to the affected widgets.
 */
void ViewNodeScene::setSelectedLine(int sourceNode, int receiverNode, int slot)
{
   selectedNode = -1;
   emit lineSelected(sourceNode, receiverNode, slot);
}

/**
 * @brief ViewNodeScene::nodeSettingsUpdated
 * @param id
 */
void ViewNodeScene::nodeSettingsUpdated(int id)
{
   nodeItems.value(id)->settingsUpdated();
}

/**
 * @brief ViewNodeScene::generatorUpdated
 * @param id
 */
void ViewNodeScene::generatorUpdated(int id)
{
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->settingsUpdated();
   }
}

/**
 * @brief ViewNodeScene::imageUpdated
 * @param id
 */
void ViewNodeScene::imageUpdated(int id)
{
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->imageUpdated();
   }
}

/**
 * @brief ViewNodeScene::nodeRemoved
 * @param id
 */
void ViewNodeScene::nodeRemoved(int id)
{
   ViewNodeItem* nodeItem = nodeItems.value(id);
   if (!nodeItem) {
      return;
   }
   QSetIterator<ViewNodeLine*> startIterator(nodeItem->getStartLines());
   while (startIterator.hasNext()) {
      ViewNodeLine* startLine = startIterator.next();
      nodesDisconnected(startLine->sourceItemId, startLine->receiverItemId, startLine->slot);
   }
   QMapIterator<int, ViewNodeLine*> endIterator(nodeItem->getEndLines());
   while (endIterator.hasNext()) {
      ViewNodeLine* endLine = endIterator.next().value();
      nodesDisconnected(endLine->sourceItemId, endLine->receiverItemId, endLine->slot);
   }
   nodeItems.remove(id);
   removeItem(nodeItem);
   delete nodeItem;
}

/**
 * @brief ViewNodeScene::imageAvailable
 * @param id
 * @param size
 */
void ViewNodeScene::imageAvailable(int id, QSize size)
{
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->imageAvailable(size);
   }
}

/**
 * @brief ViewNodeScene::settingsUpdated
 */
void ViewNodeScene::settingsUpdated(void)
{
   dropItem = NULL;
   QMapIterator<int, ViewNodeItem*> nodeItemIterator(nodeItems);
   while (nodeItemIterator.hasNext()) {
      nodeItemIterator.next().value()->setThumbnailSize(project->getThumbnailSize());
   }
   QMapIterator<std::tuple<int, int, int>, ViewNodeLine*> nodeConnectionsIterator(nodeConnections);
   while (nodeConnectionsIterator.hasNext()) {
      nodeConnectionsIterator.next().value()->updatePos();
   }
   if (project->getSettingsManager()) {
      QColor backgroundColor = project->getSettingsManager()->getBackgroundColor();
      Qt::BrushStyle brushStyle = Qt::BrushStyle(project->getSettingsManager()->getBackgroundBrush());
      if (brushStyle == Qt::NoBrush) {
         brushStyle = Qt::SolidPattern;
      }
      setBackgroundBrush(QBrush(backgroundColor, brushStyle));
      QListIterator<QGraphicsView*> viewsIterator(views());
      while (viewsIterator.hasNext()) {
         ViewNodeView* view = dynamic_cast<ViewNodeView*>(viewsIterator.next());
         if (view) {
            view->setDefaultZoom(project->getSettingsManager()->getDefaultZoom());
         }
      }
      update();
   }
}

/**
 * @brief ViewNodeScene::endLineDrawing
 * @param endNodeId
 */
void ViewNodeScene::endLineDrawing(int endNodeId)
{
   if (lineDrawing) {
      ViewNodeItem* startNode = getItem(startLineNode);
      ViewNodeItem* endNode = getItem(endNodeId);
      if (startNode) {
         startNode->clearOverlays();
      }
      if (endNode) {
         endNode->clearOverlays();
      }
      if (project->getNode(endNodeId)) {
         project->getNode(endNodeId)->setSourceSlot(-1, startLineNode);
      }
      lineItem->prepareGeometryChange();
      delete lineItem;
      lineDrawing = false;
   }
}

/**
 * @brief ViewNodeScene::startLineDrawing
 * @param nodeId
 */
void ViewNodeScene::startLineDrawing(int nodeId)
{
   if (lineDrawing) {
      endLineDrawing(-1);
   }
   lineDrawing = true;
   lineItem = new ViewNodeLine(this, nodeId, -1, -1);
   lineItem->setColor(Qt::blue);
   lineItem->setWidth(10);
   startLineNode = nodeId;
   getItem(nodeId)->showConnectable(true);
}

/**
 * @brief ViewNodeScene::mousePressEvent
 * @param mouseEvent
 */
void ViewNodeScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (lineDrawing && mouseEvent->button() == Qt::LeftButton) {
      endLineDrawing(-1);
   }
   QGraphicsScene::mousePressEvent(mouseEvent);
}

/**
 * @brief ViewNodeScene::mouseMoveEvent
 * @param mouseEvent
 */
void ViewNodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (lineDrawing) {
      QGraphicsItem* focusItem = itemAt(mouseEvent->scenePos(), QTransform());
      ViewNodeItem* focusNode = dynamic_cast<ViewNodeItem*> (focusItem);
      int foundNodeId = -1;
      int currentHighlighted = lineItem->getEndItemId();
      if (focusNode && focusNode->posInImage(mouseEvent->scenePos()-focusNode->pos())) {
         foundNodeId = focusNode->getId();
         if (startLineNode != foundNodeId && currentHighlighted != foundNodeId) {
            if (getItem(currentHighlighted)) {
               getItem(currentHighlighted)->clearOverlays();
            }
            TextureNodePtr texNode = project->getNode(foundNodeId);
            if (texNode && texNode->slotAvailable(-1)) {
               focusNode->showConnectable(true);
            } else {
               focusNode->showUnconnectable(true);
            }
         }
      }
      if (startLineNode != currentHighlighted && currentHighlighted != foundNodeId) {
         if (getItem(currentHighlighted)) {
            getItem(currentHighlighted)->clearOverlays();
         }
      }
      lineItem->setNodes(startLineNode, foundNodeId);
      lineItem->setPos(mouseEvent->scenePos(), mouseEvent->scenePos());
   }
   QGraphicsScene::mouseMoveEvent(mouseEvent);
}

/**
 * @brief ViewNodeScene::mouseReleaseEvent
 * @param mouseEvent
 */
void ViewNodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (mouseEvent->button() != Qt::LeftButton) {
      return;
   }
   if (lineDrawing) {
      QGraphicsItem* focusItem = itemAt(mouseEvent->scenePos(), QTransform());
      ViewNodeItem* focusNode = dynamic_cast<ViewNodeItem*> (focusItem);
      int foundNodeId = -1;
      if (focusNode != NULL && focusNode->posInImage(mouseEvent->scenePos()-focusNode->pos())) {
         foundNodeId = focusNode->getId();
      }
      endLineDrawing(foundNodeId);
   }
   QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

/**
 * @brief ViewNodeScene::contextMenuEvent
 * @param event
 */
void ViewNodeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
   QMapIterator<int, ViewNodeItem*> nodeItemsIterator(nodeItems);
   nodeItemsIterator.toBack();
   while (nodeItemsIterator.hasPrevious()) {
      ViewNodeItem* focusNode = nodeItemsIterator.previous().value();
      if (focusNode->posInImage(event->scenePos()-focusNode->pos())) {
         focusNode->contextMenuEvent(event);
         return;
      }
   }
   // Create a menu with all the generators grouped in submenus.
   QMenu menu;
   QMenu* filterMenu = menu.addMenu("&Filters");
   QMenu* generatorMenu = menu.addMenu("&Generators");
   QMenu* combinerMenu = menu.addMenu("&Combiners");
   // Mapping from chosen menu action to new texture generator
   QMap<QAction*, TextureGeneratorPtr> actions;
   QMapIterator<QString, TextureGeneratorPtr> generatorsIterator(project->getGenerators());
   while (generatorsIterator.hasNext()) {
      TextureGeneratorPtr currGenerator = generatorsIterator.next().value();
      QMenu* menuToBeUsed;
      switch (currGenerator->getType()) {
      case TextureGenerator::Type::Combiner:
         menuToBeUsed = combinerMenu;
         break;
      case TextureGenerator::Type::Filter:
         menuToBeUsed = filterMenu;
         break;
      default:
         menuToBeUsed = generatorMenu;
         break;
      }
      QAction* newAction = menuToBeUsed->addAction(QString("New %1 node").arg(currGenerator->getName()));
      actions[newAction] = currGenerator;
   }
   menu.addSeparator();
   QAction* pasteAction = menu.addAction(QString("Paste node"));

   QAction* selectedAction = menu.exec(event->screenPos());
   if (actions.contains(selectedAction)) {
      project->newNode(0, actions[selectedAction])->setPos(event->scenePos());
   } else if (selectedAction == pasteAction) {
      project->pasteNode();
   }
   QGraphicsScene::contextMenuEvent(event);
}

/**
 * @brief ViewNodeScene::dragEnterEvent
 * @param event
 */
void ViewNodeScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
   if (!dropItem) {
      dropItem = new QGraphicsRectItem;
      dropItem->setRect(QRect(QPoint(0, 40), project->getThumbnailSize()));
      dropItem->setBrush(QBrush(Qt::DiagCrossPattern));
      addItem(dropItem);
   }
   dropItem->setPos(event->scenePos());
}

/**
 * @brief ViewNodeScene::dragMoveEvent
 * @param event
 */
void ViewNodeScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
   if (dropItem) {
      dropItem->setPos(event->scenePos());
   }
}

/**
 * @brief ViewNodeScene::dragLeaveEvent
 */
void ViewNodeScene::dragLeaveEvent(QGraphicsSceneDragDropEvent*)
{
   if (dropItem) {
      removeItem(dropItem);
      delete dropItem;
      dropItem = NULL;
   }
}

/**
 * @brief ViewNodeScene::dropEvent
 * @param event
 */
void ViewNodeScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
   event->acceptProposedAction();
   if (event->buttons() & Qt::LeftButton) {
      if (dropItem) {
         removeItem(dropItem);
         delete dropItem;
         dropItem = NULL;
      }
      QString toAdd = event->mimeData()->text();
      TextureNodePtr texNode = project->newNode();
      if (texNode->setGenerator(toAdd)) {
         texNode->setPos(event->scenePos());
      } else {
         project->removeNode(texNode->getId());
      }
   }
}
