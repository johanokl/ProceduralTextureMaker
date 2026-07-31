
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SCENEINFOWIDGET_H
#define SCENEINFOWIDGET_H

#include "base/texturenode.h"
#include <QVBoxLayout>
#include <QWidget>
class ItemInfoPanel;
class QLabel;
class QGroupBox;
class QVBoxLayout;

/// @brief Displays basic scene information when no graph item is selected.
/// Currently, the widget displays the number of nodes in the scene.
class SceneInfoWidget : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates a scene information widget.
   /// @param widgetmanager Information panel that owns this widget.
   explicit SceneInfoWidget(ItemInfoPanel* widgetmanager);

   /// @brief Destroys the scene information widget.
   ~SceneInfoWidget() override = default;

   /// @brief Updates the displayed node count from the current project.
   void updateNumNodes();

private:
   /// @brief Information panel that owns this widget.
   ItemInfoPanel* widgetmanager;
   /// @brief Group box containing scene information.
   QGroupBox* nodeInfoWidget;
   /// @brief Grid layout arranging scene information.
   QGridLayout* nodeInfoLayout;
   /// @brief Label displaying the current node count.
   QLabel* numNodesLabel;
   /// @brief Main vertical layout for the widget.
   QVBoxLayout* layout;
};

#endif  // SCENEINFOWIDGET_H
