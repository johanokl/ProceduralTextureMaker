/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "base/textureproject.h"
#include "gui/mainwindow.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodescene.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

/**
 * @brief ViewNodeItem::ViewNodeItem
 * @param scene
 * @param newNode
 */
ViewNodeItem::ViewNodeItem(ViewNodeScene* scene, const TextureNodePtr& newNode)
{
   imageValid = false;
   setFlag(QGraphicsItem::ItemIsSelectable);

   id = newNode->getId();
   this->scene = scene;
   texNode = newNode;
   mousePressed = false;
   isConnectable = false;
   isUnconnectable = false;
   highlighterWidth = 6;
   borderWidth = 2;

   setThumbnailSize(scene->getTextureProject()->getThumbnailSize());
   setAcceptHoverEvents(true);
   positionUpdated();
   imageUpdated();
   positionUpdated();
   settingsUpdated();
}

/**
 * @brief ViewNodeItem::setThumbnailSize
 * @param size
 * Sets the image size to be used by this widget.
 */
void ViewNodeItem::setThumbnailSize(QSize size)
{
   prepareGeometryChange();
   pixmap = QPixmap();
   update();
   thumbnailSize = size;
   imageAvailable(thumbnailSize);
}

/**
 * @brief ViewNodeItem::shape
 * @return The painter path surrounding this line.
 * Used for deciding which object to select when the user clicks in the ViewNodeView.
 */
QPainterPath ViewNodeItem::shape() const
{
   QPainterPath path;
   QPolygon polygon;
   polygon << QPoint(0, 0);
   polygon << QPoint(thumbnailSize.width() + borderWidth * 2,
                     0);
   polygon << QPoint(thumbnailSize.width() + borderWidth * 2,
                     thumbnailSize.height() + borderWidth * 2);
   polygon << QPoint(0,
                     thumbnailSize.height() + borderWidth * 2);
   path.addPolygon(polygon);
   return path;
}

/**
 * @brief ViewNodeItem::boundingRect
 * @return QRectF used for clipping.
 */
QRectF ViewNodeItem::boundingRect() const
{
   return QRectF(-highlighterWidth, -highlighterWidth - 13,
                 thumbnailSize.width() + borderWidth * 2 + highlighterWidth * 2,
                 thumbnailSize.height() + borderWidth * 2 + highlighterWidth * 2 + 13);
}

/**
 * @brief ViewNodeItem::posInImage
 * @param pos Position in widget.
 * @return true if the position is within the image section of the widget.
 */
bool ViewNodeItem::posInImage(QPointF pos) const
{
   return QRectF(0, 0,
                 thumbnailSize.width() + borderWidth * 2,
                 thumbnailSize.height() + borderWidth * 2)
         .contains(pos);
}

/**
 * @brief ViewNodeItem::positionUpdated
 * Updates the node's and all connected lines' positions.
 */
void ViewNodeItem::positionUpdated()
{
   setPos(texNode->getPos());
   QSetIterator<ViewNodeLine*> startIterator(startLines);
   while (startIterator.hasNext()) {
      startIterator.next()->updatePos();
   }
   QMapIterator<int, ViewNodeLine*> endIterator(endLines);
   while (endIterator.hasNext()) {
      endIterator.next().value()->updatePos();
   }
}

/**
 * @brief ViewNodeItem::settingsUpdated
 */
void ViewNodeItem::settingsUpdated()
{
   titleString = texNode->getName().append(" (%1)").arg(texNode->getGeneratorName());
   setToolTip(titleString);
}

/**
 * @brief ViewNodeItem::imageUpdated
 * Called when the old image is no longer valid.
 */
void ViewNodeItem::imageUpdated()
{
   imageValid = false;
   update();
}

/**
 * @brief ViewNodeItem::showConnectable
 * @param isConnectable
 * Adds or removes an overlay indicating to the user that
 * this node can or can't be connected.
 */
void ViewNodeItem::showConnectable(bool isConnectable)
{
   if (this->isConnectable != isConnectable) {
      this->isConnectable = isConnectable;
      update();
   }
}

/**
 * @brief ViewNodeItem::showUnconnectable
 * @param isUnconnectable
 * Adds or removes an overlay indicating to the user that
 * this node can't be connected.
 */
void ViewNodeItem::showUnconnectable(bool isUnconnectable)
{
   if (this->isUnconnectable != isUnconnectable) {
      this->isUnconnectable = isUnconnectable;
      update();
   }
}

/**
 * @brief ViewNodeItem::clearOverlays
 * Resets the node's state.
 */
void ViewNodeItem::clearOverlays()
{
   if (this->isConnectable) {
      this->isConnectable = false;
      update();
   }
   if (this->isUnconnectable) {
      this->isUnconnectable = false;
      update();
   }
}

/**
 * @brief ViewNodeItem::addConnectionLine
 * @param line
 * Called when nodes are connected.
 */
void ViewNodeItem::addConnectionLine(ViewNodeLine* line)
{
   if (line->getStartItemId() == id) {
      startLines.insert(line);
   } else {
      endLines.insert(line->getSlot(), line);
   }
}

/**
 * @brief ViewNodeItem::removeConnectionLine
 * @param line
 * Called when nodes are disconnected.
 */
void ViewNodeItem::removeConnectionLine(ViewNodeLine* line)
{
   if (line->getStartItemId() == id) {
      startLines.remove(line);
   } else {
      endLines.remove(line->getSlot());
   }
}

/**
 * @brief ViewNodeItem::imageAvailable
 * @param size Image size
 *
 * Called when an image is rendered. If the image size is the one used by the
 * scene then the widget is redrawn with the updated image.
 */
void ViewNodeItem::imageAvailable(QSize size)
{
   if (size == thumbnailSize && texNode->isTextureInCache(thumbnailSize)) {
      QImage tempimage = QImage(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_ARGB32);
      memcpy(tempimage.bits(),
             texNode->getImage(thumbnailSize)->getData(),
             thumbnailSize.width() * thumbnailSize.height() * sizeof(TexturePixel));
      pixmap = QPixmap::fromImage(tempimage);
      imageValid = true;
      update();
   }
}

/**
 * @brief ViewNodeItem::contextMenuEvent
 * @param event -
 * Called when the user right-clicks on the node. Displays a context menu
 * with various actions.
 */
void ViewNodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
   event->accept();
   QMenu menu;
   QMap<int, QAction*> actions;
   for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
      if (texNode->getSources().value(i) != 0) {
         actions[i] = menu.addAction(QString("Remove source for slot %1").arg(i + 1));
      }
   }
   menu.addSeparator();
   QAction* copyNodeAction = menu.addAction(QString("Copy node"));
   QAction* cutNodeAction = menu.addAction(QString("Cut"));
   menu.addSeparator();
   QAction* exportImageAction = menu.addAction(QString("Save selected image"));
   menu.addSeparator();
   QAction* removeNodeAction = menu.addAction(QString("Remove node %1").arg(texNode->getName()));

   QAction* selectedAction = menu.exec(event->screenPos());
   if (removeNodeAction == selectedAction) {
      scene->getTextureProject()->removeNode(id);
   } else if (copyNodeAction == selectedAction) {
      scene->getTextureProject()->copyNode(id);
   } else if (cutNodeAction == selectedAction) {
      scene->getTextureProject()->cutNode(id);
   } else if (exportImageAction == selectedAction) {
      scene->getParent()->saveImage(id);
   } else {
      for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
         if (actions[i] == selectedAction) {
            texNode->setSourceSlot(i, 0);
         }
      }
   }
   event->accept();
}

/**
 * @brief ViewNodeItem::mousePressEvent
 * @param mouseEvent
 * Selects the node and updates the state.
 * If the node is already selected and the user is holding the Ctrl key,
 * the line drawing operation is started.
 */
void ViewNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (mouseEvent->button() == Qt::LeftButton) {
      if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
         scene->startLineDrawing(id);
      } else {
         if (!isSelected()) {
            scene->clearSelection();
            setSelected(true);
            scene->setSelectedNode(id);
         }
         mousePressed = true;
         mousePressedPos = mouseEvent->scenePos();
         mousePressedItemPos = texNode->getPos();
      }
   }
   mouseEvent->accept();
}

/**
 * @brief ViewNodeItem::itemChange
 * @param change -
 * @param value 0 the node is no longer selected.
 * @return -
 */
QVariant ViewNodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
   if (change == ItemSelectedChange && value.toInt() == 0) {
      scene->setSelectedNode(-1);
   }
   return QGraphicsItem::itemChange(change, value);
}

/**
 * @brief ViewNodeItem::mouseReleaseEvent
 * @param event
 * Used for enabling changing the node's position by draggign.
 */
void ViewNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent*  event)
{
   if (mousePressed &&
       QApplication::keyboardModifiers() != Qt::ControlModifier &&
       event->button() == Qt::LeftButton) {
      mousePressed = false;
   }
}

/**
 * @brief ViewNodeItem::mouseMoveEvent
 * @param mouseEvent
 * Changes the node's position when the user is dragging it.
 */
void ViewNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
      return;
   }
   if (mousePressed) {
      texNode->setPos(mousePressedItemPos + (mouseEvent->scenePos() - mousePressedPos));
   }
   QGraphicsItem::mouseMoveEvent(mouseEvent);
}

/**
 * @brief ViewNodeItem::hoverEnterEvent
 * @param event
 * Highlights and changes colors on all lines connected to this node.
 */
void ViewNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
   setCursor(Qt::ArrowCursor);
   QSetIterator<ViewNodeLine*> startLineIterator(startLines);
   while (startLineIterator.hasNext()) {
      startLineIterator.next()->setHighlighted(true);
   }
   QMapIterator<int, ViewNodeLine*> endLineIterator(endLines);
   while (endLineIterator.hasNext()) {
      endLineIterator.next().value()->setHighlighted(true);
   }
   QGraphicsItem::hoverEnterEvent(event);
}

/**
 * @brief ViewNodeItem::hoverLeaveEvent
 * @param event
 * Resets all lines connected to this node to their old states.
 */
void ViewNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
   QSetIterator<ViewNodeLine*> startLineIterator(startLines);
   while (startLineIterator.hasNext()) {
      startLineIterator.next()->setHighlighted(false);
   }
   QMapIterator<int, ViewNodeLine*> endLineIterator(endLines);
   while (endLineIterator.hasNext()) {
      endLineIterator.next().value()->setHighlighted(false);
   }
   QGraphicsItem::hoverLeaveEvent(event);
}

/**
 * @brief ViewNodeItem::hoverMoveEvent
 * Callback function for when the mouse pointer's hovering over the node.
 * Doesn't do anything but we need to define this to prevent Qt from
 * using its own function.
 */
void ViewNodeItem::hoverMoveEvent(QGraphicsSceneHoverEvent*)
{
   // Left empty
}

/**
 * @brief ViewNodeItem::paint
 * @param painter Qt's painter instance
 *
 * Overloaded paint function for drawing the node.
 */
void ViewNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
   QPen slotPen(QColor(0, 0, 0, 255));
   slotPen.setWidth(borderWidth);
   slotPen.setJoinStyle(Qt::MiterJoin);
   painter->setPen(slotPen);

   QBrush backBrush(QColor(220, 220, 220, 255));
   backBrush.setStyle(Qt::SolidPattern);
   painter->setBrush(backBrush);
   painter->drawRect(borderWidth / 2,
                     borderWidth / 2,
                     thumbnailSize.width() + borderWidth,
                     thumbnailSize.height() + borderWidth);
   painter->setBrush(Qt::NoBrush);

   QFont font;
   font.setPixelSize(12);
   font.setStyleHint(QFont::StyleHint::Helvetica);
   font.setFamily(font.defaultFamily());
   painter->setFont(font);
   painter->drawText(QRect(0, -13 - (isSelected() ? highlighterWidth : 0),
                           thumbnailSize.width(), 12), titleString);
   painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter->setPen(Qt::NoPen);
   painter->drawPixmap(borderWidth,
                       borderWidth,
                       pixmap);

   if (!imageValid) {
      QBrush updateBrush(QColor(230, 100, 0, 255));
      updateBrush.setStyle(Qt::DiagCrossPattern);
      painter->setBrush(updateBrush);
      painter->drawRect(borderWidth,
                        borderWidth,
                        thumbnailSize.width(),
                        thumbnailSize.height());
   }

   if (isConnectable || isUnconnectable || isSelected()) {
      painter->setBrush(Qt::NoBrush);
      QPen selectedPen;
      if (isConnectable || isUnconnectable) {
         selectedPen.setColor(Qt::red);
         selectedPen.setWidth(highlighterWidth);
      } else {
         selectedPen.setColor(QColor(0, 255, 0, 255));
         selectedPen.setWidth(highlighterWidth);
      }
      selectedPen.setJoinStyle(Qt::MiterJoin);
      painter->setPen(selectedPen);
      if (isUnconnectable) {
         painter->drawLine(0,
                           0,
                           thumbnailSize.width() + borderWidth * 2,
                           thumbnailSize.height() + borderWidth * 2);
         painter->drawLine(thumbnailSize.width() + borderWidth * 2,
                           0,
                           0,
                           thumbnailSize.height() + borderWidth * 2);
      }
      painter->drawRect(-highlighterWidth / 2,
                        -highlighterWidth / 2,
                        2 * borderWidth + thumbnailSize.width() + highlighterWidth,
                        2 * borderWidth + thumbnailSize.height() + highlighterWidth);
   }
}
