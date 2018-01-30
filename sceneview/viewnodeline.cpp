/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QLineF>
#include <QPen>
#include <QPointF>
#include <QPainter>
#include <QTransform>
#include <QtMath>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include "core/textureproject.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodescene.h"

/**
 * @brief ViewNodeLine::ViewNodeLine
 * @param scene
 * @param sourceItem
 * @param receiverItem
 * @param slot
 */
ViewNodeLine::ViewNodeLine(ViewNodeScene* scene, int sourceItem, int receiverItem, int slot)
{
   this->slot = slot;
   this->scene = scene;
   infocus = false;
   setNodes(sourceItem, receiverItem);
   setAcceptHoverEvents(true);
   scene->addItem(this);
   setZValue(-1);
   setFlag(QGraphicsItem::ItemIsSelectable, true);
   myPen = QPen(Qt::black, 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
   setPen(myPen);
   updatePos();
}

/**
 * @brief ViewNodeLine::~ViewNodeLine
 */
ViewNodeLine::~ViewNodeLine()
{
   prepareGeometryChange(); // Needed to prevent graphics artifacts.
   scene->removeItem(this);
}

/**
 * @brief ViewNodeLine::setNodes
 * @param sourceItem
 * @param receiverItem
 */
void ViewNodeLine::setNodes(int sourceItem, int receiverItem)
{
   sourceItemId = sourceItem;
   receiverItemId = receiverItem;
   updatePos();
}

/**
 * @brief ViewNodeLine::setPos
 * @param startPos Start position (optional)
 * @param endPos End position (optional)
 *
 * Sets the line's position, to be used for lines where
 * there's no start or end node.
 */
void ViewNodeLine::setPos(QPointF startPos, QPointF endPos)
{
   if (!startPos.isNull()) {
      sourcePos = startPos;
   }
   if (!endPos.isNull()) {
      receiverPos = endPos;
   }
   updatePos();
}

/**
 * @brief ViewNodeLine::boundingRect
 * @return QRectF used for clipping.
 */
QRectF ViewNodeLine::boundingRect() const
{
   qreal extra = (pen().width() + 20) / 2.0;
   return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(), line().p2().y() - line().p1().y()))
         .normalized()
         .adjusted(-extra, -extra, extra, extra);
}

/**
 * @brief ViewNodeLine::setHighlighted
 * @param highlighted
 * If the line should be highlighted.
 */
void ViewNodeLine::setHighlighted(bool highlighted)
{
   if (highlighted) {
      setWidth(12);
      setColor(Qt::darkGreen);
   } else {
      setWidth(10);
      setColor(Qt::black);
   }
}

/**
 * @brief ViewNodeLine::setWidth
 * @param width
 * Sets the line's width.
 */
void ViewNodeLine::setWidth(int width)
{
   myPen.setWidth(width);
   setPen(myPen);
   updatePos();
}

/**
 * @brief ViewNodeLine::setColor
 * @param color
 * Sets the line's color.
 */
void ViewNodeLine::setColor(QColor color)
{
   myPen.setColor(color);
   setPen(myPen);
   updatePos();
}

/**
 * @brief ViewNodeLine::shape
 * @return The painter path surrounding this line.
 * Used for deciding which object to select when the user clicks in the ViewNodeView.
 */
QPainterPath ViewNodeLine::shape() const
{
   QPainterPath path = QGraphicsLineItem::shape();
   path.addPolygon(arrowHead);
   QPainterPathStroker stroker;
   stroker.setWidth(20);
   stroker.setJoinStyle(Qt::MiterJoin); // and other adjustments you need
   path = (stroker.createStroke(path) + path).simplified();
   return path;
}

/**
 * @brief ViewNodeLine::mousePressEvent
 * @param mouseEvent
 */
void ViewNodeLine::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
   if (mouseEvent->button() == Qt::LeftButton) {
      if (!isSelected()) {
         scene->clearSelection();
         setSelected(true);
         scene->setSelectedLine(sourceItemId, receiverItemId, slot);
      }
   }
   QGraphicsLineItem::mousePressEvent(mouseEvent);
}

/**
 * @brief ViewNodeLine::hoverEnterEvent
 * @param event
 * Highlights the line.
 */
void ViewNodeLine::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
   setCursor(Qt::ArrowCursor);
   infocus = true;
   QGraphicsItem::hoverEnterEvent(event);
   update();
}

/**
 * @brief ViewNodeLine::hoverLeaveEvent
 * @param event
 * Removes the highlight.
 */
void ViewNodeLine::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
   infocus = false;
   QGraphicsItem::hoverLeaveEvent(event);
   update();
}

/**
 * @brief ViewNodeLine::updatePos
 * Updates the line's start and end positions to the node's current positions.
 */
void ViewNodeLine::updatePos()
{
   prepareGeometryChange();
   ViewNodeItem* sourceItem = scene->getItem(sourceItemId);
   ViewNodeItem* receiverItem = scene->getItem(receiverItemId);
   if (sourceItem == receiverItem) {
      return;
   }
   QSize thumbSize = scene->getTextureProject()->getThumbnailSize();
   if (sourceItem) {
      sourcePos = sourceItem->pos() + QPointF(thumbSize.width() / 2, thumbSize.height() / 2 + 40);
   }
   if (receiverItem) {
      receiverPos = receiverItem->pos() + QPointF(thumbSize.width() / 2, thumbSize.height() / 2 + 40);
   }
   if (receiverItem) {
      QLineF centerLine = QLineF(sourcePos, receiverPos);
      QPolygonF polygon = receiverItem->shape().toFillPolygon(QTransform());
      QPointF p1 = polygon.first() + receiverItem->pos();
      QLineF polyLine;
      for (int i = 1; i < polygon.count(); ++i) {
         QPointF p2 = polygon.at(i) + receiverItem->pos();
         polyLine = QLineF(p1, p2);
         QLineF::IntersectType intersectType = polyLine.intersect(centerLine, &receiverPos);
         if (intersectType == QLineF::BoundedIntersection) {
            break;
         }
         p1 = p2;
      }
   }
   setLine(QLineF(receiverPos, sourcePos));

   double angle = qAcos(line().dx() / line().length());
   if (line().dy() >= 0) {
      angle = (M_PI * 2) - angle;
   }

   int arrowSize = 20;
   QPointF arrowP1 = line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                           cos(angle + M_PI / 3) * arrowSize);
   QPointF arrowP2 = line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                           cos(angle + M_PI - M_PI / 3) * arrowSize);
   arrowHead.clear();
   arrowHead << line().p1() << arrowP1 << arrowP2;
   update();
}

/**
 * @brief ViewNodeLine::paint
 * @param painter Qt's painter instance
 * Draws the line with the style set by the current state.
 */
void ViewNodeLine::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
   ViewNodeItem* sourceItem = scene->getItem(sourceItemId);
   ViewNodeItem* receiverItem = scene->getItem(receiverItemId);
   if (receiverPos.isNull()) {
      return;
   }
   if (!sourceItem && sourcePos.isNull()) {
      return;
   }
   if (!sourceItem && !receiverItem) {
      return;
   }
   if (sourceItem && receiverItem && sourceItem->collidesWithItem(receiverItem)) {
      return;
   }
   QColor tmpColor = myPen.color();
   if (infocus) {
      myPen.setColor(QColor(255, 0, 0));
   }
   painter->setPen(myPen);
   painter->drawLine(line());
   painter->drawPolygon(arrowHead);
   myPen.setColor(tmpColor);

   if (isSelected()) {
      painter->setPen(QPen(Qt::blue, 20, Qt::SolidLine));
      QLineF myLine = line();
      myLine.translate(0, 4.0);
      painter->drawLine(myLine);
      myLine.translate(0,-8.0);
      painter->drawLine(myLine);
   }
}
