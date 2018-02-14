/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODELINE_H
#define VIEWNODELINE_H

#include <QGraphicsLineItem>

class ViewNodeItem;
class ViewNodeScene;

/**
 * @brief The ViewNodeLine class
 *
 * Draws a styled line in a ViewNodeScene.
 */
class ViewNodeLine : public QGraphicsLineItem
{
   friend class ViewNodeScene;
   friend class ViewNodeItem;

public:
   virtual ~ViewNodeLine();
   virtual void updatePos();
   inline int getSlot() const { return slot; }
   inline int getStartItemId() const { return sourceItemId; }
   inline int getEndItemId() const { return receiverItemId; }
   void setHighlighted(bool highlighted);
   void setWidth(int width);
   void setColor(QColor color);
   void setNodes(int sourceNodeId, int receiverNodeId);
   void setPos(QPointF startPos, QPointF endPos);
   QRectF boundingRect() const override;
   QPainterPath shape() const override;

protected:
   void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
   void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
   ViewNodeLine(ViewNodeScene* scene, int sourceItem, int receiverItem, int slot);
   int sourceItemId;
   int receiverItemId;
   int slot;
   ViewNodeScene* scene;
   QPointF sourcePos;
   QPointF receiverPos;

   bool infocus;
   QPen myPen;
   QPolygonF arrowHead;
};

#endif // VIEWNODELINE_H
