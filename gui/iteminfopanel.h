
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef ITEMINFOPANEL_H
#define ITEMINFOPANEL_H

#include "base/texturenode.h"
#include <QMap>
#include <QPointer>
#include <QSize>
#include <QWidget>
class TextureProject;
class NodeSettingsWidget;
class ConnectionWidget;
class SceneInfoWidget;
class EditManager;

/// @brief Displays scene, node, or connection information for the current graph selection.
class ItemInfoPanel : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates an information panel for a texture project.
   /// @param parent Parent widget.
   /// @param textureProject Project whose graph information is displayed.
   ItemInfoPanel(QWidget* parent, TextureProject* textureProject, EditManager* editManager);

   /// @brief Destroys the information panel and its cached widgets.
   ~ItemInfoPanel() override = default;

   /// @brief Returns the project displayed by this panel.
   TextureProject* getTextureProject() { return texproject; }

   /// @brief Returns the edit manager used by this panel.
   EditManager* getEditManager() { return editManager; }

public slots:
   /// @brief Displays the scene information or settings for the selected node.
   /// @param id Node identifier, or 0 to display scene information.
   void setActiveNode(int id);

   /// @brief Displays information about a selected connection.
   /// @param sourceNodeId Source node identifier.
   /// @param receiverNodeId Receiver node identifier.
   /// @param slot Receiver input slot identifier.
   void setActiveLine(int sourceNodeId, int receiverNodeId, QString slot);

   /// @brief Removes cached information for a deleted node.
   /// @param id Deleted node identifier.
   void removeNode(int id);

   /// @brief Refreshes the scene information after a node is added.
   /// @param node Added node.
   void addNode(const TextureNodePtr& node);

   /// @brief Refreshes source controls for a changed node.
   /// @param id Changed node identifier.
   void sourceUpdated(int id);

   /// @brief Refreshes the panel after a connection is removed.
   /// @param sourceNodeId Source node identifier.
   /// @param receiverNodeId Receiver node identifier.
   /// @param slot Receiver input slot identifier.
   void nodesDisconnected(int sourceNodeId, int receiverNodeId, QString slot);

private:
   /// @brief Project whose selection details are displayed.
   TextureProject* texproject{nullptr};
   /// @brief Edit manager for the displayed project.
   EditManager* editManager{nullptr};
   /// @brief Cached settings widgets indexed by node identifier.
   QMap<int, QPointer<NodeSettingsWidget>> nodes;
   /// @brief Widget displaying the selected connection.
   ConnectionWidget* lineWidget{nullptr};
   /// @brief Currently displayed node settings widget.
   QPointer<NodeSettingsWidget> currWidget;
   /// @brief Widget displayed when no graph item is selected.
   SceneInfoWidget* sceneWidget{nullptr};
   /// @brief Identifier of the currently displayed node.
   int currNodeId{0};
   /// @brief Source, receiver, and slot of the currently displayed connection.
   std::tuple<int, int, QString> currLine{0, 0, QString()};
};

#endif  // ITEMINFOPANEL_H
