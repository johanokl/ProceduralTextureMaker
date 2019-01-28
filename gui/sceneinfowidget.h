/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SCENEINFOWIDGET_H
#define SCENEINFOWIDGET_H

#include "base/texturenode.h"
#include <QVBoxLayout>
#include <QWidget>

class ItemInfoPanel;
class QLabel;
class QGroupBox;
class QVBoxLayout;

/**
 * @brief The SceneInfoWidget class
 *
 * Simple widget which currently just displays the number of nodes
 * in the scene. For having something to display in the info panel
 * when the scene's empty.
 */
class SceneInfoWidget : public QWidget
{
   Q_OBJECT

 public:
   explicit SceneInfoWidget(ItemInfoPanel* widgetmanager);
   ~SceneInfoWidget() override = default;
   void updateNumNodes();

private:
   ItemInfoPanel* widgetmanager;

   QGroupBox* nodeInfoWidget;
   QGridLayout* nodeInfoLayout;
   QLabel* numNodesLabel;
   QVBoxLayout* layout;
};

#endif // SCENEINFOWIDGET_H
