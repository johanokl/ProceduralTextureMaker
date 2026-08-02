
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "base/editmanager.h"
#include "gui/connectionwidget.h"
#include "gui/iteminfopanel.h"
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ConnectionWidget::ConnectionWidget(ItemInfoPanel& widgetmanager)
    : QWidget(&widgetmanager), widgetmanager(widgetmanager) {
   auto* layout = new QVBoxLayout(this);
   setLayout(layout);

   auto* nodeInfoWidget = new QGroupBox("Connection");
   auto* nodeInfoLayout = new QGridLayout();
   nodeInfoWidget->setLayout(nodeInfoLayout);
   nodeSourceLabel = new QLabel();
   nodeReceiverLabel = new QLabel();
   nodeSlotLabel = new QLabel();
   nodeInfoLayout->addWidget(new QLabel("Source: "), 0, 0);
   nodeInfoLayout->addWidget(new QLabel("Receiver: "), 1, 0);
   nodeInfoLayout->addWidget(new QLabel("Slot: "), 2, 0);
   nodeInfoLayout->addWidget(nodeSourceLabel, 0, 1);
   nodeInfoLayout->addWidget(nodeReceiverLabel, 1, 1);
   nodeInfoLayout->addWidget(nodeSlotLabel, 2, 1);
   layout->addWidget(nodeInfoWidget);

   QPushButton* disconnectNodesButton = new QPushButton("Disconnect");
   layout->addWidget(disconnectNodesButton);
   QObject::connect(disconnectNodesButton, &QPushButton::clicked, this,
                    &ConnectionWidget::disconnectNodes);

   auto* spaceritem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   layout->addItem(spaceritem);
}

void ConnectionWidget::disconnectNodes() {
   TextureProject* project = widgetmanager.getTextureProject();
   if (project == nullptr) {
      return;
   }
   auto editManager = widgetmanager.getEditManager();
   if (editManager != nullptr) {
      editManager->setConnection(receiverNodeId, slot, 0);
   }
   this->hide();
}

void ConnectionWidget::setNodes(int sourceNodeId, int receiverNodeId, const QString& slot) {
   this->sourceNodeId = sourceNodeId;
   this->receiverNodeId = receiverNodeId;
   this->slot = slot;

   nodeSourceLabel->setText(QString("%1").arg(sourceNodeId));
   nodeReceiverLabel->setText(QString("%1").arg(receiverNodeId));
   nodeSlotLabel->setText(slot);

   TextureProject* project = widgetmanager.getTextureProject();
   if (project != nullptr) {
      TextureNodePtr node;
      node = project->getNode(sourceNodeId);
      if (!node.isNull()) {
         nodeSourceLabel->setText(
             QString("%1\n(%2)").arg(node->getName(), node->getGeneratorName()));
      }
      node = project->getNode(receiverNodeId);
      if (!node.isNull()) {
         nodeReceiverLabel->setText(
             QString("%1\n(%2)").arg(node->getName(), node->getGeneratorName()));
      }
   }
}
