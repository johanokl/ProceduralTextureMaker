
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODELINE_H
#define VIEWNODELINE_H

#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QPen>
class ViewNodeItem;
class ViewNodeScene;

/// @brief The ViewNodeLine class
///
/// Draws a styled line in a ViewNodeScene.
class ViewNodeLine : public QGraphicsLineItem {
   friend class ViewNodeScene;
   friend class ViewNodeItem;

public:
   ~ViewNodeLine() override = default;
   void updatePos();
   inline int getSlot() const { return slot; }
   inline int getStartItemId() const { return sourceItemId; }
   inline int getEndItemId() const { return receiverItemId; }
   void setHighlighted(bool highlighted);
   void setWidth(int width);
   void setLineWidths(int normalWidth, int highlightedWidth);
   void setArrowSize(int size);
   void setColor(const QColor& color);
   void setNodes(int sourceNodeId, int receiverNodeId);
   void setPos(QPointF startPos, QPointF endPos);
   QRectF boundingRect() const override;
   QPainterPath shape() const override;

protected:
   void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
   void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
   ViewNodeLine(ViewNodeScene* scene, int sourceItem, int receiverItem, int slot);
   QPointF getNodeIntersection(ViewNodeItem* item, const QPointF& insidePos,
                               const QPointF& outsidePos) const;
   QPolygonF createArrowHead(qreal size) const;

   ViewNodeScene* nodescene;
   int sourceItemId;
   int receiverItemId;
   int slot;

   QPointF sourcePos;
   QPointF receiverPos;
   QPointF lineStart;
   QPointF lineControl;
   QPointF arrowTip;
   QPointF arrowDirection;

   QPolygonF arrowHead;
   QPainterPath linePath;
   QPen myPen;
   QColor baseColor;
   int normalWidth;
   int highlightedWidth;
   int arrowSize;
   bool infocus;
   bool highlighted;
};

#endif  // VIEWNODELINE_H
