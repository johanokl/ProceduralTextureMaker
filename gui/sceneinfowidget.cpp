/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "core/textureproject.h"
#include "gui/iteminfopanel.h"
#include "gui/sceneinfowidget.h"
#include <QGroupBox>
#include <QLabel>

/**
 * @brief SceneInfoWidget::SceneInfoWidget
 * @param widgetmanager Parent
 */
SceneInfoWidget::SceneInfoWidget(ItemInfoPanel* widgetmanager)
{
   this->widgetmanager = widgetmanager;

   layout = new QVBoxLayout(this);
   this->setLayout(layout);

   nodeInfoWidget = new QGroupBox("Scene info");
   nodeInfoLayout = new QGridLayout();
   nodeInfoWidget->setLayout(nodeInfoLayout);
   nodeInfoLayout->addWidget(new QLabel("Number of nodes: "), 0, 0);
   numNodesLabel = new QLabel("0", this);
   nodeInfoLayout->addWidget(numNodesLabel, 0, 1);
   layout->addWidget(nodeInfoWidget);

   layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

/**
 * @brief SceneInfoWidget::updateNumNodes
 * Updates the text label with the number of nodes.
 */
void SceneInfoWidget::updateNumNodes()
{
   int num = widgetmanager->getTextureProject()->getNumNodes();
   numNodesLabel->setText(QString("%1").arg(num));
}
