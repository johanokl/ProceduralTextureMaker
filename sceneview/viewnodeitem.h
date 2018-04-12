/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODEITEM_H
#define VIEWNODEITEM_H

#include "core/texturenode.h"
#include <QGraphicsItem>

class ViewNodeScene;
class TextureNode;
class ViewNodeLine;

/**
 * @brief The ViewNodeItem class
 *
 * Renders the node as a widget in a ViewNodeView scene.
 * Contains functions that let the user interact with the node and
 * change connections, positions etc.
 */
class ViewNodeItem : public QGraphicsItem
{
   friend class ViewNodeLine;
   friend class ViewNodeScene;

public:
   ViewNodeItem(ViewNodeScene* scene, const TextureNodePtr& newNode);
   ~ViewNodeItem() override = default;
   void paint (QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
   int getId() const { return id; }
   TextureNodePtr getTextureNode() { return texNode; }
   bool posInImage(QPointF pos) const;
   QRectF boundingRect() const override;
   QPainterPath shape() const override;
   QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
   void showConnectable(bool showConnectable);
   void showUnconnectable(bool showUnconnectable);
   void clearOverlays();
   QSet<ViewNodeLine*> getStartLines() const { return startLines; }
   QMap<int, ViewNodeLine*> getEndLines() const { return endLines; }
   void addConnectionLine(ViewNodeLine* line);
   void removeConnectionLine(ViewNodeLine* line);

public slots:
   void positionUpdated();
   void settingsUpdated();
   void imageUpdated();
   void imageAvailable(QSize size);
   void generatorUpdated();
   void setThumbnailSize(QSize);

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent*  event) override;
   void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
   int id;

   ViewNodeScene* scene;
   QPixmap pixmap;
   TextureNodePtr texNode;
   QSize thumbnailSize;
   QString titleString;

   int highlightSlot;
   QPointF mousePressedPos;
   QPointF mousePressedItemPos;
   int highlighterWidth;
   int borderWidth;

   QSet<ViewNodeLine*> startLines;
   QMap<int, ViewNodeLine*> endLines;

   bool imageValid;
   bool isUnconnectable;
   bool isConnectable;
   bool mousePressed;
};

#endif // VIEWNODEITEM_H
