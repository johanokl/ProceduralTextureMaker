
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/settingsmanager.h"
#include "base/textureproject.h"
#include "base/editmanager.h"
#include "gui/clipboardoperations.h"
#include "gui/mainwindow.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodescene.h"
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>

ViewNodeScene::ViewNodeScene(MainWindow& mainWindow)
    : mainWindow(mainWindow), project(*mainWindow.getTextureProject()) {
   lineWidth = QSettings().value("lineWidth", 3).toInt();
   if (lineWidth < 1 || lineWidth > 8) {
      lineWidth = 3;
   }
   highlightedLineWidth = lineWidth + 1;
   arrowSize = QSettings().value("arrowSize", 12).toInt();
   if (arrowSize < 8 || arrowSize > 16) {
      arrowSize = 12;
   }
   headerSize = QSettings().value("headerSize", 24).toInt();
   if (headerSize != 0 && (headerSize < 8 || headerSize > 48)) {
      headerSize = 24;
   }
   QObject::connect(&project, &TextureProject::nodeAdded, this, &ViewNodeScene::addNode);
   QObject::connect(&project, &TextureProject::nodeRemoved, this, &ViewNodeScene::nodeRemoved);
   QObject::connect(&project, &TextureProject::nodesConnected, this,
                    &ViewNodeScene::nodesConnected);
   QObject::connect(&project, &TextureProject::nodesDisconnected, this,
                    &ViewNodeScene::nodesDisconnected);
   QObject::connect(project.getSettingsManager(), &SettingsManager::settingsUpdated, this,
                    &ViewNodeScene::settingsUpdated);

   QBrush backgroundBrush;
   backgroundBrush.setStyle(Qt::SolidPattern);
   backgroundBrush.setColor(QColor(200, 200, 200));
   setBackgroundBrush(backgroundBrush);
   settingsUpdated();
}

std::unique_ptr<ViewNodeScene> ViewNodeScene::clone() const {
   auto newscene = std::make_unique<ViewNodeScene>(mainWindow);
   QMapIterator<int, ViewNodeItem*> nodesIter(nodeItems);
   while (nodesIter.hasNext()) {
      TextureNodePtr ptr = nodesIter.next().value()->getTextureNode();
      newscene->addNode(ptr);
      newscene->imageAvailable(ptr->getId(), getTextureProject().getThumbnailSize());
   }
   QMapIterator<std::tuple<int, int, QString>, ViewNodeLine*> connectionsIter(nodeConnections);
   while (connectionsIter.hasNext()) {
      std::tuple<int, int, QString> connection = connectionsIter.next().key();
      newscene->nodesConnected(std::get<0>(connection), std::get<1>(connection),
                               std::get<2>(connection));
   }
   return newscene;
}

ViewNodeItem* ViewNodeScene::getItem(int id) const {
   if (id < 0) {
      return nullptr;
   }
   if (!nodeItems.contains(id)) {
      return nullptr;
   }
   return nodeItems.value(id);
}

void ViewNodeScene::setLineWidths(int normalWidth, int highlightedWidth) {
   lineWidth = normalWidth;
   highlightedLineWidth = highlightedWidth;
   QMapIterator<std::tuple<int, int, QString>, ViewNodeLine*> nodeConnectionsIterator(
       nodeConnections);
   while (nodeConnectionsIterator.hasNext()) {
      nodeConnectionsIterator.next().value()->setLineWidths(lineWidth, highlightedLineWidth);
   }
   if (lineItem) {
      lineItem->setLineWidths(highlightedLineWidth, highlightedLineWidth + 1);
   }
   update();
}

void ViewNodeScene::setArrowSize(int arrowSize) {
   if (arrowSize < 8 || arrowSize > 16) {
      arrowSize = 12;
   }
   this->arrowSize = arrowSize;
   QMapIterator<std::tuple<int, int, QString>, ViewNodeLine*> nodeConnectionsIterator(
       nodeConnections);
   while (nodeConnectionsIterator.hasNext()) {
      nodeConnectionsIterator.next().value()->setArrowSize(arrowSize);
   }
   if (lineItem) {
      lineItem->setArrowSize(arrowSize);
   }
   update();
}

void ViewNodeScene::setHeaderSize(int headerSize) {
   if (headerSize != 0 && (headerSize < 8 || headerSize > 48)) {
      headerSize = 24;
   }
   this->headerSize = headerSize;
   QMapIterator<int, ViewNodeItem*> nodeItemIterator(nodeItems);
   while (nodeItemIterator.hasNext()) {
      nodeItemIterator.next().value()->setHeaderSize(headerSize);
   }
   QMapIterator<std::tuple<int, int, QString>, ViewNodeLine*> nodeConnectionsIterator(
       nodeConnections);
   while (nodeConnectionsIterator.hasNext()) {
      nodeConnectionsIterator.next().value()->updatePos();
   }
   update();
}

void ViewNodeScene::clearProject() { project.clear(); }

void ViewNodeScene::nodesConnected(int sourceid, int receiverid, QString slot) {
   ViewNodeItem* sourceNode = nodeItems.value(sourceid);
   ViewNodeItem* receiverNode = nodeItems.value(receiverid);
   if (!sourceNode || !receiverNode) {
      return;
   }
   std::tuple<int, int, QString> key(sourceid, receiverid, slot);
   if (nodeConnections.contains(key)) {
      return;
   }
   auto* newLine = new ViewNodeLine(*this, sourceid, receiverid, slot);
   newLine->setLineWidths(lineWidth, highlightedLineWidth);
   newLine->setArrowSize(arrowSize);
   newLine->setLabelSettings(connectionLabelSize, displaySourceNames, displayReceiverNames);
   sourceNode->addConnectionLine(newLine);
   receiverNode->addConnectionLine(newLine);
   newLine->update();
   addItem(newLine);
   nodeConnections.insert(key, newLine);
}

void ViewNodeScene::nodesDisconnected(int sourceid, int receiverid, QString slot) {
   auto key = std::tuple<int, int, QString>(sourceid, receiverid, slot);
   if (key == selectedLine) {
      selectedLine = std::tuple<int, int, QString>(-1, 0, QString());
   }
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

void ViewNodeScene::addNode(const TextureNodePtr& newNode) {
   auto* newItem = new ViewNodeItem(*this, newNode);
   newItem->setHeaderSize(headerSize);
   nodeItems.insert(newNode->getId(), newItem);
   QObject::connect(newNode.data(), &TextureNode::positionUpdated, this,
                    &ViewNodeScene::positionUpdated);
   QObject::connect(newNode.data(), &TextureNode::settingsUpdated, this,
                    &ViewNodeScene::nodeSettingsUpdated);
   QObject::connect(newNode.data(), &TextureNode::nameUpdated, this,
                    &ViewNodeScene::nodeSettingsUpdated);
   QObject::connect(newNode.data(), &TextureNode::imageUpdated, this, &ViewNodeScene::imageUpdated);
   QObject::connect(newNode.data(), &TextureNode::imageAvailable, this,
                    &ViewNodeScene::imageAvailable);
   QObject::connect(newNode.data(), &TextureNode::generatorUpdated, this,
                    &ViewNodeScene::generatorUpdated);
   addItem(newItem);
   newItem->settingsUpdated();
   newItem->imageAvailable(project.getThumbnailSize());
   update();
}

void ViewNodeScene::positionUpdated(int id) {
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->positionUpdated();
   }
}

void ViewNodeScene::setSelectedNode(int id) {
   selectedNode = id;
   selectedLine = std::tuple<int, int, QString>(-1, 0, QString());
   emit nodeSelected(id);
}

void ViewNodeScene::setSelectedLine(int sourceNode, int receiverNode, const QString& slot) {
   selectedNode = -1;
   selectedLine = std::tuple<int, int, QString>(sourceNode, receiverNode, slot);
   emit lineSelected(sourceNode, receiverNode, slot);
}

void ViewNodeScene::nodeSettingsUpdated(int id) {
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->settingsUpdated();
   }
}

void ViewNodeScene::generatorUpdated(int id) {
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->settingsUpdated();
   }
}

void ViewNodeScene::imageUpdated(int id) {
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->imageUpdated();
   }
}

void ViewNodeScene::nodeRemoved(int id) {
   ViewNodeItem* nodeItem = nodeItems.value(id);
   if (!nodeItem) {
      return;
   }
   QSetIterator<ViewNodeLine*> startIterator(nodeItem->getStartLines());
   while (startIterator.hasNext()) {
      ViewNodeLine* startLine = startIterator.next();
      nodesDisconnected(startLine->sourceItemId, startLine->receiverItemId, startLine->slot);
   }
   QMapIterator<QString, ViewNodeLine*> endIterator(nodeItem->getEndLines());
   while (endIterator.hasNext()) {
      ViewNodeLine* endLine = endIterator.next().value();
      nodesDisconnected(endLine->sourceItemId, endLine->receiverItemId, endLine->slot);
   }
   nodeItems.remove(id);
   delete nodeItem;
}

void ViewNodeScene::imageAvailable(int id, QSize size) {
   ViewNodeItem* node = nodeItems.value(id);
   if (node) {
      node->imageAvailable(size);
   }
}

void ViewNodeScene::settingsUpdated() {
   auto settingsManager = project.getSettingsManager();
   if (settingsManager != nullptr) {
      connectionLabelSize = settingsManager->getConnectionLabelSize();
      displaySourceNames = settingsManager->getDisplaySourceNames();
      displayReceiverNames = settingsManager->getDisplayReceiverNames();
   }
   if (dropItem) {
      QSize itemSize = project.getThumbnailSize() + QSize(4, 4);
      dropItem->setRect(QRect(QPoint(0, 0), itemSize));
   }
   QMapIterator<int, ViewNodeItem*> nodeItemIterator(nodeItems);
   while (nodeItemIterator.hasNext()) {
      nodeItemIterator.next().value()->setThumbnailSize(project.getThumbnailSize());
   }
   QMapIterator<std::tuple<int, int, QString>, ViewNodeLine*> nodeConnectionsIterator(
       nodeConnections);
   while (nodeConnectionsIterator.hasNext()) {
      ViewNodeLine* line = nodeConnectionsIterator.next().value();
      line->setLabelSettings(connectionLabelSize, displaySourceNames, displayReceiverNames);
      line->updatePos();
   }
   if (settingsManager != nullptr) {
      QColor backgroundColor = settingsManager->getBackgroundColor();
      auto brushStyle = Qt::BrushStyle(settingsManager->getBackgroundBrush());
      if (brushStyle == Qt::NoBrush) {
         brushStyle = Qt::SolidPattern;
      }
      setBackgroundBrush(QBrush(backgroundColor, brushStyle));
      update();
   }
}

void ViewNodeScene::endLineDrawing(int endNodeId) {
   if (lineDrawing) {
      ViewNodeItem* startNode = getItem(startLineNode);
      ViewNodeItem* endNode = getItem(endNodeId);
      if (startNode) {
         startNode->clearOverlays();
      }
      if (endNode) {
         endNode->clearOverlays();
      }
      if (project.getNode(endNodeId)) {
         mainWindow.getEditManager().setConnectionToFirstAvailable(endNodeId, startLineNode);
      }
      if (lineItem) {
         lineItem->prepareGeometryChange();
         delete lineItem;
         lineItem = nullptr;
      }
      lineDrawing = false;
   }
}

void ViewNodeScene::startLineDrawing(int nodeId) {
   if (lineDrawing) {
      endLineDrawing(-1);
   }
   ViewNodeItem* startItem = getItem(nodeId);
   if (!startItem) {
      return;
   }
   lineDrawing = true;
   lineItem = new ViewNodeLine(*this, nodeId, -1, QString());
   lineItem->setColor(Qt::blue);
   lineItem->setLineWidths(highlightedLineWidth, highlightedLineWidth + 1);
   lineItem->setArrowSize(arrowSize);
   addItem(lineItem);
   startLineNode = nodeId;
   startItem->showConnectable(true);
}

void ViewNodeScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (lineDrawing && mouseEvent->button() == Qt::LeftButton) {
      endLineDrawing(-1);
   }
   QGraphicsScene::mousePressEvent(mouseEvent);
}

void ViewNodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (lineDrawing) {
      QGraphicsItem* focusItem = itemAt(mouseEvent->scenePos(), QTransform());
      auto* focusNode = dynamic_cast<ViewNodeItem*>(focusItem);
      int foundNodeId = -1;
      int currentHighlighted = lineItem->getEndItemId();
      if (focusNode && focusNode->posInImage(mouseEvent->scenePos() - focusNode->pos())) {
         foundNodeId = focusNode->getId();
         if (startLineNode != foundNodeId && currentHighlighted != foundNodeId) {
            if (getItem(currentHighlighted)) {
               getItem(currentHighlighted)->clearOverlays();
            }
            TextureNodePtr texNode = project.getNode(foundNodeId);
            if (texNode && !texNode->getFirstAvailableSourceSlot().isNull()) {
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
      lineItem->setPos(mouseEvent->scenePos(), mouseEvent->scenePos());
      lineItem->setNodes(startLineNode, foundNodeId);
   }
   QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void ViewNodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (mouseEvent->button() != Qt::LeftButton) {
      return;
   }
   if (lineDrawing) {
      QGraphicsItem* focusItem = itemAt(mouseEvent->scenePos(), QTransform());
      auto* focusNode = dynamic_cast<ViewNodeItem*>(focusItem);
      int foundNodeId = -1;
      if (focusNode != nullptr &&
          focusNode->posInImage(mouseEvent->scenePos() - focusNode->pos())) {
         foundNodeId = focusNode->getId();
      }
      endLineDrawing(foundNodeId);
   }
   QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ViewNodeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
   QMapIterator<int, ViewNodeItem*> nodeItemsIterator(nodeItems);
   nodeItemsIterator.toBack();
   while (nodeItemsIterator.hasPrevious()) {
      ViewNodeItem* focusNode = nodeItemsIterator.previous().value();
      if (focusNode->posInImage(event->scenePos() - focusNode->pos())) {
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
   QHash<QAction*, TextureGeneratorPtr> actions;
   QMapIterator<QString, TextureGeneratorPtr> generatorsIterator(project.getGenerators());
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
      QAction* newAction =
          menuToBeUsed->addAction(QString("New %1 node").arg(currGenerator->getName()));
      actions[newAction] = currGenerator;
   }
   menu.addSeparator();
   QAction* pasteAction = menu.addAction(QString("Paste node"));

   QAction* selectedAction = menu.exec(event->screenPos());
   if (actions.contains(selectedAction)) {
      mainWindow.getEditManager().addNode(actions[selectedAction], event->scenePos());
   } else if (selectedAction == pasteAction) {
      pasteNodesFromClipboard(mainWindow.getEditManager());
   } else {
      QGraphicsScene::contextMenuEvent(event);
   }
}

void ViewNodeScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
   if (!dropItem) {
      dropItem = new QGraphicsRectItem;
      QSize itemSize = project.getThumbnailSize();
      itemSize.setWidth(itemSize.width() + 4);
      itemSize.setHeight(itemSize.height() + 4);
      dropItem->setRect(QRect(QPoint(0, 0), itemSize));
      dropItem->setBrush(QBrush(Qt::DiagCrossPattern));
   }
   dropItem->setPos(event->scenePos());
   if (!dropItem->scene()) {
      addItem(dropItem);
   }
}

void ViewNodeScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
   if (dropItem) {
      dropItem->setPos(event->scenePos());
   }
}

void ViewNodeScene::dragLeaveEvent(QGraphicsSceneDragDropEvent*) { clearDropItem(); }

void ViewNodeScene::dropEvent(QGraphicsSceneDragDropEvent* event) {
   event->acceptProposedAction();
   clearDropItem();
   QString toAdd = event->mimeData()->text();
   TextureGeneratorPtr generator = project.getGenerator(toAdd);
   if (generator.isNull()) {
      return;
   }
   mainWindow.getEditManager().addNode(generator, event->scenePos());
}

void ViewNodeScene::clearDropItem() {
   delete dropItem;
   dropItem = nullptr;
}

void ViewNodeScene::keyPressEvent(QKeyEvent* event) {
   switch (event->key()) {
      case Qt::Key_Delete: {
         TextureNodePtr node = project.getNode(selectedNode);
         if (node && QMessageBox::question(
                         &mainWindow, "Remove", QString("Remove node %1?").arg(node->getName()),
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            mainWindow.getEditManager().removeNode(selectedNode);
            break;
         }
         TextureNodePtr lineSecondNode = project.getNode(std::get<1>(selectedLine));
         if (lineSecondNode) {
            mainWindow.getEditManager().setConnection(std::get<1>(selectedLine),
                                                      std::get<2>(selectedLine), 0);
         }
         break;
      }
      default:
         break;
   }
   QGraphicsScene::keyPressEvent(event);
}
