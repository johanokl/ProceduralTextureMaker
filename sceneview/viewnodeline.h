
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODELINE_H
#define VIEWNODELINE_H

#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QPen>
#include <QString>
class ViewNodeItem;
class ViewNodeScene;

/// @brief Draws and manages a connection between two scene nodes.
class ViewNodeLine : public QGraphicsLineItem {
   friend class ViewNodeScene;
   friend class ViewNodeItem;

public:
   /// @brief Destroys the connection line.
   ~ViewNodeLine() override = default;

   /// @brief Updates the line endpoints to match the connected node positions.
   void updatePos();

   /// @brief Returns the receiver input slot used by the connection.
   /// @return Receiver input slot name.
   inline QString getSlot() const { return slot; }

   /// @brief Returns the source node identifier.
   /// @return Source node identifier.
   inline int getStartItemId() const { return sourceItemId; }

   /// @brief Returns the receiver node identifier.
   /// @return Receiver node identifier.
   inline int getEndItemId() const { return receiverItemId; }

   /// @brief Returns whether endpoint labels are currently shown.
   bool endpointLabelsVisible() const {
      return highlighted && ((displayReceiverNames && !sourceLabelText.isEmpty()) ||
                             (displaySourceNames && !receiverLabelText.isEmpty()));
   }

   /// @brief Returns the source-edge endpoint label.
   QString getSourceLabelText() const { return sourceLabelText; }

   /// @brief Returns the receiver-edge endpoint label.
   QString getReceiverLabelText() const { return receiverLabelText; }

   /// @brief Returns the configured endpoint-label font size.
   int getLabelFontSize() const { return labelFontSize; }

   /// @brief Returns whether the source-name label is currently visible.
   bool sourceNameLabelVisible() const {
      return highlighted && displaySourceNames && !receiverLabelText.isEmpty();
   }

   /// @brief Returns whether the receiver-name label is currently visible.
   bool receiverNameLabelVisible() const {
      return highlighted && displayReceiverNames && !sourceLabelText.isEmpty();
   }

   /// @brief Sets whether the connection is highlighted.
   /// @param highlighted Whether to use the highlighted appearance.
   void setHighlighted(bool highlighted);

   /// @brief Configures endpoint-label size and name visibility.
   /// @param fontSize Label font size in pixels.
   /// @param showSourceNames Whether to show source names at receiving edges.
   /// @param showReceiverNames Whether to show receiver names at source edges.
   void setLabelSettings(int fontSize, bool showSourceNames, bool showReceiverNames);

   /// @brief Sets the normal line width and derives its highlighted width.
   /// @param width Normal line width in pixels.
   void setWidth(int width);

   /// @brief Sets the normal and highlighted line widths.
   /// @param normalWidth Normal line width in pixels.
   /// @param highlightedWidth Highlighted line width in pixels.
   void setLineWidths(int normalWidth, int highlightedWidth);

   /// @brief Sets the arrowhead size.
   /// @param size Arrowhead size in scene coordinates.
   void setArrowSize(int size);

   /// @brief Sets the base line color.
   /// @param color Base color used when the line is not focused.
   void setColor(const QColor& color);

   /// @brief Sets the nodes connected by the line.
   /// @param sourceNodeId Source node identifier.
   /// @param receiverNodeId Receiver node identifier.
   void setNodes(int sourceNodeId, int receiverNodeId);

   /// @brief Sets fallback endpoints for an incomplete connection.
   /// @param startPos Optional source position.
   /// @param endPos Optional receiver position.
   void setPos(QPointF startPos, QPointF endPos);

   /// @brief Returns the rectangle containing the connection path and arrowhead.
   /// @return Bounding rectangle used for clipping and redraws.
   QRectF boundingRect() const override;

   /// @brief Returns the selectable shape around the connection.
   /// @return Painter path surrounding the line and arrowhead.
   QPainterPath shape() const override;

protected:
   /// @brief Draws the connection using its current state.
   /// @param painter Painter used to draw the item.
   /// @param option Item style options.
   /// @param widget Widget being painted, when applicable.
   void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   /// @brief Selects the connection when it is clicked.
   /// @param event Mouse press event.
   void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   /// @brief Applies the focused appearance when the pointer enters the line.
   /// @param event Hover event.
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

   /// @brief Restores the normal appearance when the pointer leaves the line.
   /// @param event Hover event.
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
   /// @brief Creates a connection line managed by a node scene.
   /// @param scene Scene that owns and coordinates the line.
   /// @param sourceItem Source node identifier, or zero for an incomplete line.
   /// @param receiverItem Receiver node identifier, or zero for an incomplete line.
   /// @param slot Receiver input slot name.
   ViewNodeLine(ViewNodeScene& scene, int sourceItem, int receiverItem, QString slot);

   /// @brief Updates endpoint label text and rectangles from the live node model.
   void updateLabelGeometry();

   /// @brief Finds where a connection intersects a node outline.
   /// @param item Node whose outline is tested.
   /// @param insidePos Position inside the node.
   /// @param outsidePos Position outside the node.
   /// @return Intersection with the node outline.
   QPointF getNodeIntersection(ViewNodeItem* item, const QPointF& insidePos,
                               const QPointF& outsidePos) const;

   /// @brief Creates an arrowhead aligned with the current connection direction.
   /// @param size Arrowhead size in scene coordinates.
   /// @return Arrowhead polygon.
   QPolygonF createArrowHead(qreal size) const;

   /// @brief Scene that owns and coordinates the connection.
   ViewNodeScene& nodescene;
   /// @brief Source node identifier.
   int sourceItemId;
   /// @brief Receiver node identifier.
   int receiverItemId;
   /// @brief Receiver input slot name.
   QString slot;
   /// @brief Current source endpoint in scene coordinates.
   QPointF sourcePos;
   /// @brief Current receiver endpoint in scene coordinates.
   QPointF receiverPos;
   /// @brief Start of the painted curve.
   QPointF lineStart;
   /// @brief Control point of the painted curve.
   QPointF lineControl;
   /// @brief Tip of the connection arrow.
   QPointF arrowTip;
   /// @brief Unit vector pointing along the connection arrow.
   QPointF arrowDirection;
   /// @brief Polygon used to draw and select the arrowhead.
   QPolygonF arrowHead;
   /// @brief Curved path between the connected nodes.
   QPainterPath linePath;
   /// @brief Text shown beside the source endpoint while highlighted.
   QString sourceLabelText;
   /// @brief Text shown beside the receiver endpoint while highlighted.
   QString receiverLabelText;
   /// @brief Source endpoint label rectangle.
   QRectF sourceLabelRect;
   /// @brief Receiver endpoint label rectangle.
   QRectF receiverLabelRect;
   /// @brief Font size used for endpoint labels.
   int labelFontSize;
   /// @brief Whether the receiving-edge label is drawn.
   bool displaySourceNames;
   /// @brief Whether the source-edge label is drawn.
   bool displayReceiverNames;
   /// @brief Pen used to draw the connection.
   QPen myPen;
   /// @brief Line color used when the connection is not focused.
   QColor baseColor;
   /// @brief Width of a normal connection line.
   int normalWidth;
   /// @brief Width of a highlighted connection line.
   int highlightedWidth;
   /// @brief Arrowhead size in scene coordinates.
   int arrowSize;
   /// @brief Whether the pointer is currently over the line.
   bool infocus;
   /// @brief Whether connected-node highlighting is active.
   bool highlighted;
};

#endif  // VIEWNODELINE_H
