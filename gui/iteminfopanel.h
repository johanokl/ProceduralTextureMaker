/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef ITEMINFOPANEL_H
#define ITEMINFOPANEL_H

#include <QWidget>
#include <QMap>
#include <QSize>
#include "core/texturenode.h"

class TextureProject;
class NodeSettingsWidget;
class ConnectionWidget;
class SceneInfoWidget;

/**
 * @brief The ItemInfoPanel class
 *
 * Controls which info widget to display at a given moment. Either the scene node,
 * when no node is selected in the graph, the node settings widget or the line
 * node connection info widget.
 */
class ItemInfoPanel : public QWidget
{
   Q_OBJECT

public:
   ItemInfoPanel(QWidget* parent, TextureProject* textureProject);
   virtual ~ItemInfoPanel() {}
   TextureProject* getTextureProject() { return texproject; }

public slots:
   void setActiveNode(int id);
   void setActiveLine(int sourceNodeId, int receiverNodeId, int slot);
   void removeNode(int id);
   void addNode(TextureNodePtr);
   void sourceUpdated(int id);

private:
   TextureProject* texproject;
   QMap<int, NodeSettingsWidget*> nodes;

   ConnectionWidget* lineWidget;
   NodeSettingsWidget* currWidget;
   SceneInfoWidget* sceneWidget;
   int currId;
};


#endif // ITEMINFOPANEL_H
