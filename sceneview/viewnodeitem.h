
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODEITEM_H
#define VIEWNODEITEM_H

#include "base/texturenode.h"
#include <QGraphicsItem>
class ViewNodeScene;
class TextureNode;
class ViewNodeLine;

/// @brief Renders an interactive texture node within a graph scene.
/// Supports selection, positioning, connection editing, and thumbnail updates.
class ViewNodeItem : public QGraphicsItem {
   friend class ViewNodeLine;
   friend class ViewNodeScene;

public:
   /// @brief Creates a scene item for a texture node.
   /// @param scene Scene that owns and coordinates the item.
   /// @param newNode Texture node represented by the item.
   ViewNodeItem(ViewNodeScene& scene, const TextureNodePtr& newNode);

   /// @brief Destroys the node item.
   ~ViewNodeItem() override = default;

   /// @brief Draws the node card and its current interaction overlays.
   /// @param painter Painter used to draw the item.
   /// @param option Item style options.
   /// @param widget Widget being painted, when applicable.
   void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   /// @brief Returns the represented node identifier.
   /// @return Node identifier.
   int getId() const { return id; }

   /// @brief Returns the represented texture node.
   /// @return Shared texture node.
   TextureNodePtr getTextureNode() { return texNode; }

   /// @brief Tests whether a position lies within the thumbnail area.
   /// @param pos Position in item coordinates.
   /// @return True when the position is inside the thumbnail.
   bool posInImage(QPointF pos) const;

   /// @brief Returns the rectangle containing the node and its overlays.
   /// @return Bounding rectangle used for clipping and redraws.
   QRectF boundingRect() const override;

   /// @brief Returns the selectable shape of the node card.
   /// @return Painter path surrounding the node and active overlays.
   QPainterPath shape() const override;

   /// @brief Responds to selection changes and refreshes connected lines.
   /// @param change Graphics item property being changed.
   /// @param value New property value.
   /// @return Value accepted by the base graphics item.
   QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

   /// @brief Shows or hides the valid-connection overlay.
   /// @param showConnectable Whether the node can accept the pending connection.
   void showConnectable(bool showConnectable);

   /// @brief Shows or hides the invalid-connection overlay.
   /// @param showUnconnectable Whether the node rejects the pending connection.
   void showUnconnectable(bool showUnconnectable);

   /// @brief Removes all connection-state overlays.
   void clearOverlays();

   /// @brief Returns lines that originate at this node.
   /// @return Set of outgoing connection lines.
   QSet<ViewNodeLine*> getStartLines() const { return startLines; }

   /// @brief Returns lines that terminate at this node.
   /// @return Incoming connection lines indexed by input slot.
   QMap<int, ViewNodeLine*> getEndLines() const { return endLines; }

   /// @brief Registers a connection line with the node.
   /// @param line Connection line to register.
   void addConnectionLine(ViewNodeLine* line);

   /// @brief Removes a registered connection line from the node.
   /// @param line Connection line to remove.
   void removeConnectionLine(ViewNodeLine* line);

public slots:
   /// @brief Updates the item and its connections after the node moves.
   void positionUpdated();

   /// @brief Refreshes the title after node settings change.
   void settingsUpdated();

   /// @brief Marks the current thumbnail as stale.
   void imageUpdated();

   /// @brief Displays a newly rendered thumbnail when its size matches.
   /// @param size Available image size.
   void imageAvailable(QSize size);

   /// @brief Refreshes the item after its generator changes.
   void generatorUpdated();

   /// @brief Sets the thumbnail size displayed by the item.
   /// @param size Thumbnail dimensions.
   void setThumbnailSize(QSize size);

   /// @brief Sets the height of the node title area.
   /// @param size Header height in pixels, or zero to hide it.
   void setHeaderSize(int size);

protected:
   /// @brief Selects the node or starts drawing a connection.
   /// @param event Mouse press event.
   void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

   /// @brief Moves the node while it is being dragged.
   /// @param mouseEvent Mouse move event.
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

   /// @brief Ends an active node drag.
   /// @param event Mouse release event.
   void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

   /// @brief Suppresses the default hover-move behavior.
   /// @param event Hover event.
   void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

   /// @brief Highlights connections when the pointer enters the node.
   /// @param event Hover event.
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

   /// @brief Removes connection highlighting when the pointer leaves the node.
   /// @param event Hover event.
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

   /// @brief Displays actions for the selected node.
   /// @param event Context-menu event.
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
   /// @brief Updates every connection line attached to the node.
   void updateConnectionLines();

   /// @brief Identifier of the represented texture node.
   int id;
   /// @brief Scene that owns and coordinates the node item.
   ViewNodeScene& scene;
   /// @brief Rendered thumbnail displayed by the node.
   QPixmap pixmap;
   /// @brief Texture node represented by the item.
   TextureNodePtr texNode;
   /// @brief Dimensions of the rendered thumbnail.
   QSize thumbnailSize;
   /// @brief Text displayed in the node header.
   QString titleString;
   /// @brief Input slot highlighted during connection drawing.
   int highlightSlot;
   /// @brief Scene position at which the current drag began.
   QPointF mousePressedPos;
   /// @brief Node position at which the current drag began.
   QPointF mousePressedItemPos;
   /// @brief Width of the connection-state overlay.
   int highlighterWidth;
   /// @brief Width of the node card border.
   int borderWidth;
   /// @brief Height of the node title area.
   int headerSize;
   /// @brief Non-owning references to scene-owned lines originating at the node.
   QSet<ViewNodeLine*> startLines;
   /// @brief Non-owning references to scene-owned incoming lines by input slot.
   QMap<int, ViewNodeLine*> endLines;
   /// @brief Whether the displayed thumbnail matches the current node state.
   bool imageValid;
   /// @brief Whether the node rejects the pending connection.
   bool isUnconnectable;
   /// @brief Whether the node accepts the pending connection.
   bool isConnectable;
   /// @brief Whether the node is currently being dragged.
   bool mousePressed;
};

#endif  // VIEWNODEITEM_H
