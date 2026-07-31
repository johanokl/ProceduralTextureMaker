
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
class TextureProject;
class ItemInfoPanel;
class QLabel;
class QVBoxLayout;
class QGroupBox;
class QGridLayout;

/// @brief Displays a connection between two nodes. Currently, it lists
/// the names of the source and receiver nodes with a button for
/// disconnecting them.
/// @todo Display the nodes' images similar to PreviewImagePanel
class ConnectionWidget : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates the widget and its layout without selecting a connection.
   /// @param widgetmanager Parent panel that manages connection and node information widgets.
   explicit ConnectionWidget(ItemInfoPanel* widgetmanager);

   /// @brief Destroys the connection widget.
   ~ConnectionWidget() override = default;

   /// @brief Updates the widget to display a different connection.
   /// @param sourceNodeId Source node identifier.
   /// @param receiverNodeId Receiver node identifier.
   /// @param slot Receiver input slot identifier.
   void setNodes(int sourceNodeId, int receiverNodeId, int slot);

public slots:
   /// @brief Disconnects the displayed nodes and hides the widget.
   void disconnectNodes();

private:
   /// @brief Information panel that owns and coordinates this widget.
   ItemInfoPanel* widgetmanager;
   /// @brief Identifier of the displayed source node.
   int sourceNodeId;
   /// @brief Identifier of the displayed receiver node.
   int receiverNodeId;
   /// @brief Input slot used by the displayed connection.
   int slot;
   /// @brief Group box containing the connection details.
   QGroupBox* nodeInfoWidget;
   /// @brief Grid layout arranging the connection labels.
   QGridLayout* nodeInfoLayout;
   /// @brief Label displaying the source node.
   QLabel* nodeSourceLabel;
   /// @brief Label displaying the receiver node.
   QLabel* nodeReceiverLabel;
   /// @brief Label displaying the receiver input slot.
   QLabel* nodeSlotLabel;
   /// @brief Main vertical layout for the widget.
   QVBoxLayout* layout;
};

#endif  // CONNECTIONWIDGET_H
