/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODESCENE_H
#define VIEWNODESCENE_H

#include "base/texturenode.h"
#include <QGraphicsScene>
#include <QMap>
#include <tuple>

class TextureProject;
class ViewNodeItem;
class ViewNodeLine;
class TextureGenerator;
class MainWindow;
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
   explicit ViewNodeScene(MainWindow* parent);
   ~ViewNodeScene() override = default;
   ViewNodeScene* clone() const;
   TextureProject* getTextureProject() const { return project; }
   MainWindow* getParent() const { return parent; }
   virtual void clear();
   void endLineDrawing(int endNode);
   void startLineDrawing(int nodeId);
   ViewNodeItem* getItem(int id) const;

   void setSelectedNode(int id);
   int getSelectedNode() const { return selectedNode; }
   void setSelectedLine(int sourceNode, int receiverNode, int slot);
   void connectNodes(int sourceNodeId, int receiverNodeId, int slotId);
   void removeConnection(int sourceNodeId, int receiverNodeId);

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
   void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
   void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
   void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
   void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
   void dropEvent(QGraphicsSceneDragDropEvent* event) override;
   void keyPressEvent(QKeyEvent* event) override;

public slots:
   void addNode(const TextureNodePtr& newNode);
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
   ViewNodeLine* lineItem;
   int startLineNode;
   int selectedNode;
   std::tuple<int, int, int> selectedLine;
   bool lineDrawing;
};

#endif // VIEWNODESCENE_H
