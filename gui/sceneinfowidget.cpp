
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/iteminfopanel.h"
#include "gui/sceneinfowidget.h"
#include <QGroupBox>
#include <QLabel>

SceneInfoWidget::SceneInfoWidget(ItemInfoPanel& widgetmanager)
    : QWidget(&widgetmanager), widgetmanager(widgetmanager) {
   auto* layout = new QVBoxLayout(this);
   setLayout(layout);

   auto* nodeInfoWidget = new QGroupBox("Scene info");
   auto* nodeInfoLayout = new QGridLayout();
   nodeInfoWidget->setLayout(nodeInfoLayout);
   nodeInfoLayout->addWidget(new QLabel("Number of nodes: "), 0, 0);
   numNodesLabel = new QLabel("0", this);
   nodeInfoLayout->addWidget(numNodesLabel, 0, 1);
   layout->addWidget(nodeInfoWidget);

   layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void SceneInfoWidget::updateNumNodes() {
   int num = widgetmanager.getTextureProject()->getNumNodes();
   numNodesLabel->setText(QString("%1").arg(num));
}
