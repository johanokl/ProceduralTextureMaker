/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODEITEM_H
#define VIEWNODEITEM_H

#include <QGraphicsItem>
#include "core/texturenode.h"

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
   ViewNodeItem(ViewNodeScene* scene, TextureNodePtr newNode);
   virtual ~ViewNodeItem() {}
   void paint (QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
   int getId() const { return id; }
   TextureNodePtr getTextureNode() { return texNode; }
   bool posInImage(QPointF pos) const;
   QRectF boundingRect() const;
   QPainterPath shape() const;
   QVariant itemChange(GraphicsItemChange change, const QVariant &value);
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
   void mousePressEvent(QGraphicsSceneMouseEvent* event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent*  event);
   void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
   void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
   void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

private:
   int id;

   ViewNodeScene* scene;
   QPixmap pixmap;
   TextureNodePtr texNode;
   QSize thumbnailSize;

   bool imageValid;
   bool isUnconnectable;
   bool isConnectable;

   QString titleString;

   bool mousePressed;
   int highlightSlot;
   QPointF mousePressedPos;
   QPointF mousePressedItemPos;
   int highlighterWidth;
   int borderWidth;

   QSet<ViewNodeLine*> startLines;
   QMap<int, ViewNodeLine*> endLines;
};

#endif // VIEWNODEITEM_H
