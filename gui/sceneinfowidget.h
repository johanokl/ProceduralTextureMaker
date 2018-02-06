/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SCENEINFOWIDGET_H
#define SCENEINFOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "core/texturenode.h"

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
   SceneInfoWidget(ItemInfoPanel* widgetmanager);
   virtual ~SceneInfoWidget() {}
   void updateNumNodes();

private:
   ItemInfoPanel* widgetmanager;

   QGroupBox* nodeInfoWidget;
   QGridLayout* nodeInfoLayout;
   QLabel* numNodesLabel;
   QVBoxLayout* layout;
};

#endif // SCENEINFOWIDGET_H
