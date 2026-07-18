
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodescene.h"
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

/// @brief Constructor for the ViewNodeLine class.
/// @param scene
/// @param sourceItem
/// @param receiverItem
/// @param slot
ViewNodeLine::ViewNodeLine(ViewNodeScene* scene, int sourceItem, int receiverItem, int slot) {
   this->slot = slot;
   nodescene = scene;
   infocus = false;
   highlighted = false;
   sourceItemId = 0;
   receiverItemId = 0;
   baseColor = QColor("#24313d");
   normalWidth = 3;
   highlightedWidth = 4;
   arrowSize = 12;
   setNodes(sourceItem, receiverItem);
   setAcceptHoverEvents(true);
   setZValue(-1);
   setFlag(QGraphicsItem::ItemIsSelectable, true);
   myPen = QPen(baseColor, normalWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
   myPen.setCosmetic(true);
   setPen(myPen);
   updatePos();
}

/// @brief ViewNodeLine::setNodes
/// @param sourceItem
/// @param receiverItem
void ViewNodeLine::setNodes(int sourceItem, int receiverItem) {
   sourceItemId = sourceItem;
   receiverItemId = receiverItem;
   updatePos();
}

/// @brief Sets the line's position, to be used for lines where there's no start or end node.
/// @param startPos Start position (optional)
/// @param endPos End position (optional)
void ViewNodeLine::setPos(QPointF startPos, QPointF endPos) {
   if (!startPos.isNull()) {
      sourcePos = startPos;
   }
   if (!endPos.isNull()) {
      receiverPos = endPos;
   }
   updatePos();
}

/// @brief ViewNodeLine::boundingRect
/// @return QRectF used for clipping.
QRectF ViewNodeLine::boundingRect() const {
   qreal extra = qMax(256.0, arrowSize * 8.0);
   return linePath.controlPointRect().normalized().adjusted(-extra, -extra, extra, extra);
}

/// @brief If the line should be highlighted or not.
/// @param highlighted
void ViewNodeLine::setHighlighted(bool highlighted) {
   this->highlighted = highlighted;
   if (highlighted) {
      setColor(QColor("#1687b8"));
   } else {
      setColor(QColor("#24313d"));
   }
}

/// @brief Sets the line's width.
/// @param width
void ViewNodeLine::setWidth(int width) {
   normalWidth = width;
   highlightedWidth = width + 1;
   myPen.setWidth(infocus || highlighted ? highlightedWidth : normalWidth);
   setPen(myPen);
   updatePos();
}

/// @brief ViewNodeLine::setLineWidths
/// @param normalWidth Width for regular lines.
/// @param highlightedWidth Width for highlighted lines.
void ViewNodeLine::setLineWidths(int normalWidth, int highlightedWidth) {
   this->normalWidth = normalWidth;
   this->highlightedWidth = highlightedWidth;
   myPen.setWidth(infocus || highlighted ? highlightedWidth : normalWidth);
   setPen(myPen);
   update();
}

/// @brief ViewNodeLine::setArrowSize
/// @param size Size of the line arrow.
void ViewNodeLine::setArrowSize(int size) {
   if (size < 8 || size > 16) {
      size = 12;
   }
   if (arrowSize == size) {
      return;
   }
   arrowSize = size;
   updatePos();
}

/// @brief Sets the line's color.
/// @param color
void ViewNodeLine::setColor(const QColor& color) {
   baseColor = color;
   myPen.setColor(infocus ? QColor("#4fc3f7") : baseColor);
   setPen(myPen);
   update();
}

/// @brief Used for deciding which object to select when the user clicks in the ViewNodeView.
/// @return The painter path surrounding this line.
QPainterPath ViewNodeLine::shape() const {
   QPainterPath path = linePath;
   path.addPolygon(arrowHead);
   QPainterPathStroker stroker;
   stroker.setWidth(20);
   stroker.setCapStyle(Qt::RoundCap);
   stroker.setJoinStyle(Qt::RoundJoin);
   QPainterPath stroke = stroker.createStroke(path);
   QPainterPath fullPath = path + stroke;
   return fullPath;
}

/// @brief ViewNodeLine::createArrowHead
/// @param size Size of the arrow in scene coordinates.
/// @return The arrowhead polygon.
QPolygonF ViewNodeLine::createArrowHead(qreal size) const {
   if (arrowDirection.isNull()) {
      return QPolygonF();
   }
   QPointF normal(-arrowDirection.y(), arrowDirection.x());
   QPointF arrowBase = arrowTip - arrowDirection * size;
   QPointF arrowP1 = arrowBase + normal * size * 0.45;
   QPointF arrowP2 = arrowBase - normal * size * 0.45;
   QPolygonF arrowPolygon;
   arrowPolygon << arrowTip << arrowP1 << arrowP2;
   return arrowPolygon;
}

/// @brief ViewNodeLine::getNodeIntersection
/// @param item The node item.
/// @param insidePos Position inside the node.
/// @param outsidePos Position outside the node.
/// @return Intersection with the node outline.
QPointF ViewNodeLine::getNodeIntersection(ViewNodeItem* item, const QPointF& insidePos,
                                          const QPointF& outsidePos) const {
   QLineF centerLine(insidePos, outsidePos);
   if (!item || centerLine.length() <= 0) {
      return insidePos;
   }
   QPolygonF polygon = item->shape().toFillPolygon(QTransform());
   if (polygon.count() < 2) {
      return insidePos;
   }

   QPointF intersection = insidePos;
   qreal shortestDistance = -1;
   for (int i = 0; i < polygon.count(); ++i) {
      QPointF p1 = item->mapToScene(polygon.at(i));
      QPointF p2 = item->mapToScene(polygon.at((i + 1) % polygon.count()));
      QLineF polyLine(p1, p2);
      QPointF currentIntersection;
      QLineF::IntersectType intersectType = polyLine.intersects(centerLine, &currentIntersection);
      if (intersectType == QLineF::BoundedIntersection) {
         qreal distance = QLineF(insidePos, currentIntersection).length();
         if (shortestDistance < 0 || distance < shortestDistance) {
            shortestDistance = distance;
            intersection = currentIntersection;
         }
      }
   }
   return intersection;
}

/// @brief ViewNodeLine::mousePressEvent
/// @param mouseEvent
void ViewNodeLine::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (mouseEvent->button() == Qt::LeftButton) {
      if (!isSelected()) {
         nodescene->clearSelection();
         setSelected(true);
         nodescene->setSelectedLine(sourceItemId, receiverItemId, slot);
      }
   }
   QGraphicsLineItem::mousePressEvent(mouseEvent);
}

/// @brief Highlights the line.
/// @param event
void ViewNodeLine::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
   setCursor(Qt::ArrowCursor);
   infocus = true;
   myPen.setWidth(highlightedWidth);
   myPen.setColor(QColor("#4fc3f7"));
   setPen(myPen);
   QGraphicsItem::hoverEnterEvent(event);
   update();
}

/// @brief Removes the highlight.
/// @param event
void ViewNodeLine::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
   infocus = false;
   myPen.setWidth(highlighted ? highlightedWidth : normalWidth);
   myPen.setColor(baseColor);
   setPen(myPen);
   QGraphicsItem::hoverLeaveEvent(event);
   update();
}

/// @brief Updates the line's start and end positions to the node's current positions.
void ViewNodeLine::updatePos() {
   prepareGeometryChange();
   ViewNodeItem* sourceItem = nodescene->getItem(sourceItemId);
   ViewNodeItem* receiverItem = nodescene->getItem(receiverItemId);
   if (sourceItem == receiverItem) {
      return;
   }
   QSize thumbSize = nodescene->getTextureProject()->getThumbnailSize();
   bool hasSourcePos = sourceItem || !sourcePos.isNull();
   bool hasReceiverPos = receiverItem || !receiverPos.isNull();
   QPointF sourceCenter = sourcePos;
   QPointF receiverCenter = receiverPos;
   if (sourceItem) {
      sourceCenter =
          sourceItem->pos() + QPointF(2 + thumbSize.width() / 2, 2 + thumbSize.height() / 2);
      sourcePos = sourceCenter;
   }
   if (receiverItem) {
      receiverCenter =
          receiverItem->pos() + QPointF(2 + thumbSize.width() / 2, 2 + thumbSize.height() / 2);
      receiverPos = receiverCenter;
   }
   if (hasSourcePos && hasReceiverPos) {
      if (receiverItem) {
         receiverPos = getNodeIntersection(receiverItem, receiverCenter, sourceCenter);
      }
   }
   setLine(QLineF(receiverPos, sourcePos));

   QPointF start = line().p2();
   QPointF end = line().p1();
   QLineF baseLine(start, end);
   QPointF control = (start + end) / 2;
   if (baseLine.length() > 0) {
      QPointF normal(-baseLine.dy() / baseLine.length(), baseLine.dx() / baseLine.length());
      qreal curveOffset = qMin(70.0, qMax(25.0, baseLine.length() * 0.15));
      control += normal * curveOffset;
   }
   lineStart = start;
   lineControl = control;
   linePath = QPainterPath(lineStart);
   linePath.quadTo(lineControl, end);

   QLineF arrowLine(control, end);
   qreal arrowLineLength = arrowLine.length();
   if (arrowLineLength <= 0) {
      arrowHead.clear();
      arrowTip = QPointF();
      arrowDirection = QPointF();
      update();
      return;
   }
   arrowTip = end;
   arrowDirection = (end - control) / arrowLineLength;
   arrowHead = createArrowHead(arrowSize);
   update();
}

/// @brief Draws the line with the style set by the current state.
/// @param painter Qt's painter instance
void ViewNodeLine::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
   ViewNodeItem* sourceItem = nodescene->getItem(sourceItemId);
   ViewNodeItem* receiverItem = nodescene->getItem(receiverItemId);
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
   painter->setRenderHint(QPainter::Antialiasing, true);
   qreal viewScale =
       QStyleOptionGraphicsItem::levelOfDetailFromTransform(painter->worldTransform());
   if (viewScale <= 0) {
      viewScale = 1.0;
   }
   qreal arrowPaintSize = arrowSize / viewScale;
   QPointF lineEnd = arrowTip - arrowDirection * arrowPaintSize;
   QPainterPath paintedLinePath(lineStart);
   paintedLinePath.quadTo(lineControl, lineEnd);
   QPolygonF paintedArrowHead = createArrowHead(arrowPaintSize);
   if (isSelected()) {
      QPen selectedPen(QColor("#4fc3f7"), highlightedWidth + 4, Qt::SolidLine, Qt::FlatCap,
                       Qt::MiterJoin);
      selectedPen.setCosmetic(true);
      painter->setPen(selectedPen);
      painter->drawPath(paintedLinePath);
   }
   QPen linePen(myPen);
   linePen.setCapStyle(Qt::FlatCap);
   painter->setPen(linePen);
   painter->drawPath(paintedLinePath);
   painter->setPen(Qt::NoPen);
   painter->setBrush(myPen.color());
   painter->drawPolygon(paintedArrowHead);
}
