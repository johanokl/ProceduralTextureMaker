/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODESCENE_H
#define VIEWNODESCENE_H

#include <QGraphicsScene>
#include <QPair>
#include <QMap>
#include <tuple>
#include "core/texturenode.h"

class TextureProject;
class ViewNodeItem;
class ViewNodeLine;
class TextureGenerator;
class MainWindow;
class QMenu;
class QAction;
class QGraphicsRectItem;

/**
 * @brief The ViewNodeScene class
 *
 * The visualization of a TextureProject node scene.
 *
 * Manages the ViewNodeItem and ViewNodeLine object as well as
 * user's actions on it via the scene widget.
 */
class ViewNodeScene : public QGraphicsScene
{
   Q_OBJECT
   friend class ViewNodeItem;

public:
   ViewNodeScene(MainWindow* parent);
   virtual ~ViewNodeScene();
   ViewNodeScene* clone();
   TextureProject* getTextureProject();
   virtual void clear();
   void endLineDrawing(int endNode);
   void startLineDrawing(int nodeId);
   ViewNodeItem* getItem(int id);

   void setSelectedNode(int id);
   int getSelectedNode() { return selectedNode; }
   void setSelectedLine(int sourceNode, int receiverNode, int slot);
   void connectNodes(int sourceNodeId, int receiverNodeId, int slotId);
   void removeConnection(int sourceNodeId, int receiverNodeId);

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
   void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
   void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
   void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
   void dropEvent(QGraphicsSceneDragDropEvent* event);

public slots:
   void addNode(TextureNodePtr newNode);
   void nodeRemoved(int id);
   void positionUpdated(int id);
   void nodeSettingsUpdated(int id);
   void imageUpdated(int id);
   void imageAvailable(int id, QSize size);
   void nodesConnected(int sourceid, int slot, int receiverid);
   void nodesDisconnected(int sourceid, int slot, int receiverid);
   void generatorUpdated(int id);
   void settingsUpdated();

signals:
   void nodeSelected(int id);
   void lineSelected(int sourceNode, int receiverNode, int slot);

private:
   MainWindow* parent;
   TextureProject* project;
   QMap<int, ViewNodeItem*> nodeItems;
   QMap<std::tuple<int, int, int>, ViewNodeLine*> nodeConnections;

   QGraphicsRectItem* dropItem;
   int startLineNode;
   bool lineDrawing;
   ViewNodeLine* lineItem;
   int selectedNode;
};

#endif // VIEWNODESCENE_H
