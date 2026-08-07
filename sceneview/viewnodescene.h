
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODESCENE_H
#define VIEWNODESCENE_H

#include "base/texturenode.h"
#include <QGraphicsScene>
#include <QMap>
#include <memory>
#include <tuple>
class TextureProject;
class ViewNodeItem;
class ViewNodeLine;
class TextureGenerator;
class MainWindow;
class QAction;
class QGraphicsRectItem;
class QPainter;

/// @brief Visualizes a texture project as an interactive node graph.
/// Manages node items, connection lines, selection, and graph editing gestures.
class ViewNodeScene : public QGraphicsScene {
   Q_OBJECT
   friend class ViewNodeItem;

public:
   /// @brief Creates a graph scene for a main window's texture project.
   /// @param mainWindow Main window that supplies the project and handles scene actions.
   explicit ViewNodeScene(MainWindow& mainWindow);

   /// @brief Destroys the graph scene and its graphics items.
   ~ViewNodeScene() override = default;

   /// @brief Creates a new scene with the same graph presentation.
   /// @return Owned scene containing copies of the current items.
   std::unique_ptr<ViewNodeScene> clone() const;

   /// @brief Returns the texture project represented by the scene.
   /// @return Borrowed texture project.
   TextureProject& getTextureProject() const { return project; }

   /// @brief Returns the main window that coordinates the scene.
   /// @return Borrowed main window.
   MainWindow& getMainWindow() const { return mainWindow; }

   /// @brief Clears the represented texture project.
   void clearProject();

   /// @brief Completes the pending connection at a receiver node.
   /// @param endNode Receiver node identifier.
   void endLineDrawing(int endNode);

   /// @brief Starts drawing a connection from a source node.
   /// @param nodeId Source node identifier.
   void startLineDrawing(int nodeId);

   /// @brief Returns the graphics item for a texture node.
   /// @param id Node identifier.
   /// @return Node item, or nullptr when the node is not present.
   ViewNodeItem* getItem(int id) const;

   /// @brief Sets the normal and highlighted connection widths.
   /// @param normalWidth Normal line width in pixels.
   /// @param highlightedWidth Highlighted line width in pixels.
   void setLineWidths(int normalWidth, int highlightedWidth);

   /// @brief Sets the arrowhead size for all connections.
   /// @param arrowSize Arrowhead size in scene coordinates.
   void setArrowSize(int arrowSize);

   /// @brief Sets the title-area height for all node items.
   /// @param headerSize Header height in pixels, or zero to hide it.
   void setHeaderSize(int headerSize);

   /// @brief Selects a node and notifies interested widgets.
   /// @param id Node identifier, or a negative value to clear the selection.
   void setSelectedNode(int id);

   /// @brief Returns the selected node identifier.
   /// @return Selected node identifier, or a negative value when none is selected.
   int getSelectedNode() const { return selectedNode; }

   /// @brief Selects a connection and notifies interested widgets.
   /// @param sourceNode Source node identifier.
   /// @param receiverNode Receiver node identifier.
   /// @param slot Receiver input slot name.
   void setSelectedLine(int sourceNode, int receiverNode, const QString& slot);

protected:
   /// @brief Paints the base scene color and optional brush pattern.
   void drawBackground(QPainter* painter, const QRectF& rect) override;

   /// @brief Updates selection when the scene is clicked.
   /// @param mouseEvent Mouse press event.
   void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

   /// @brief Updates a connection while it is being drawn.
   /// @param mouseEvent Mouse release event.
   void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

   /// @brief Moves the pending connection and updates connection overlays.
   /// @param mouseEvent Mouse move event.
   void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

   /// @brief Displays graph-level editing actions.
   /// @param event Context-menu event.
   void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

   /// @brief Accepts supported generator drag operations.
   /// @param event Drag-enter event.
   void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;

   /// @brief Updates the generator drop indicator.
   /// @param event Drag-move event.
   void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

   /// @brief Removes the generator drop indicator when a drag leaves the scene.
   /// @param event Drag-leave event.
   void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;

   /// @brief Creates a texture node from a dropped generator.
   /// @param event Drop event.
   void dropEvent(QGraphicsSceneDragDropEvent* event) override;

   /// @brief Handles keyboard shortcuts while the scene has focus.
   /// @param event Key press event.
   void keyPressEvent(QKeyEvent* event) override;

public slots:
   /// @brief Adds a graphics item for a newly created texture node.
   /// @param newNode Texture node to display.
   void addNode(const TextureNodePtr& newNode);

   /// @brief Removes a node item and its connection lines.
   /// @param id Removed node identifier.
   void nodeRemoved(int id);

   /// @brief Updates a node item after its model position changes.
   /// @param id Updated node identifier.
   void positionUpdated(int id);

   /// @brief Updates a node item after its settings change.
   /// @param id Updated node identifier.
   void nodeSettingsUpdated(int id);

   /// @brief Marks a node's displayed image as stale.
   /// @param id Updated node identifier.
   void imageUpdated(int id);

   /// @brief Displays a newly available node image.
   /// @param id Updated node identifier.
   /// @param size Available image size.
   void imageAvailable(int id, QSize size);

   /// @brief Adds a line for a newly created project connection.
   /// @param sourceid Source node identifier.
   /// @param receiverid Receiver node identifier.
   /// @param slot Receiver input slot name.
   void nodesConnected(int sourceid, int receiverid, QString slot);

   /// @brief Removes the line for a deleted project connection.
   /// @param sourceid Source node identifier.
   /// @param receiverid Receiver node identifier.
   /// @param slot Receiver input slot name.
   void nodesDisconnected(int sourceid, int receiverid, QString slot);

   /// @brief Refreshes a node after its generator changes.
   /// @param id Updated node identifier.
   void generatorUpdated(int id);

   /// @brief Applies project and scene settings to all graphics items.
   void settingsUpdated();

signals:
   /// @brief Notifies observers that the selected node changed.
   /// @param id Selected node identifier, or a negative value when none is selected.
   void nodeSelected(int id);

   /// @brief Notifies observers that the selected connection changed.
   /// @param sourceNode Source node identifier.
   /// @param receiverNode Receiver node identifier.
   /// @param slot Receiver input slot name.
   void lineSelected(int sourceNode, int receiverNode, QString slot);

private:
   /// @brief Removes and deletes the active generator drop indicator.
   void clearDropItem();

   /// @brief Main window that supplies the project and handles scene actions.
   MainWindow& mainWindow;
   /// @brief Texture project represented by the scene.
   TextureProject& project;
   /// @brief Non-owning index of scene-owned node items by identifier.
   QMap<int, ViewNodeItem*> nodeItems;
   /// @brief Non-owning index of scene-owned lines by source, receiver, and slot.
   QMap<std::tuple<int, int, QString>, ViewNodeLine*> nodeConnections;
   /// @brief Observer to the scene-owned generator drop indicator.
   QGraphicsRectItem* dropItem{nullptr};
   /// @brief Observer to the scene-owned connection line currently being drawn.
   ViewNodeLine* lineItem{nullptr};
   /// @brief Source node identifier of the pending connection.
   int startLineNode{0};
   /// @brief Currently selected node identifier.
   int selectedNode{1};
   /// @brief Width of normal connection lines.
   int lineWidth{3};
   /// @brief Width of highlighted connection lines.
   int highlightedLineWidth{4};
   /// @brief Arrowhead size for connection lines.
   int arrowSize{12};
   /// @brief Font size used for connection endpoint labels.
   int connectionLabelSize{12};
   /// @brief Whether receiving-edge labels show source node names.
   bool displaySourceNames{true};
   /// @brief Whether source-edge labels show receiver node names.
   bool displayReceiverNames{false};
   /// @brief Height of node title areas.
   int headerSize{24};
   /// @brief Source, receiver, and slot of the selected connection.
   std::tuple<int, int, QString> selectedLine{-1, 0, QString()};
   /// @brief Whether an interactive connection operation is active.
   bool lineDrawing{false};
   /// @brief Optional brush painted over the solid scene background.
   QBrush backgroundPatternBrush;
};

#endif  // VIEWNODESCENE_H
