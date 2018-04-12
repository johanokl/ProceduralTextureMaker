/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "core/textureproject.h"
#include "gui/connectionwidget.h"
#include "gui/iteminfopanel.h"
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

/**
 * @brief ConnectionWidget::ConnectionWidget
 * @param widgetmanager Parent.
 */
ConnectionWidget::ConnectionWidget(ItemInfoPanel* widgetmanager)
{
   this->widgetmanager = widgetmanager;

   layout = new QVBoxLayout(this);
   this->setLayout(layout);

   nodeInfoWidget = new QGroupBox("Connection");
   nodeInfoLayout = new QGridLayout();
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
   QObject::connect(disconnectNodesButton, &QPushButton::clicked,
                    this, &ConnectionWidget::disconnectNodes);

   auto* spaceritem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   layout->addItem(spaceritem);

   sourceNodeId = -1;
   receiverNodeId = -1;
   slot = -1;
}

/**
 * @brief ConnectionWidget::disconnectNodes
 */
void ConnectionWidget::disconnectNodes()
{
   TextureProject* project = widgetmanager->getTextureProject();
   if (project == nullptr) {
      return;
   }
   TextureNodePtr node = project->getNode(receiverNodeId);
   if (node.isNull()) {
      return;
   }
   node.data()->setSourceSlot(slot, 0);
   this->hide();
}

/**
 * @brief ConnectionWidget::setNodes
 * @param sourceNodeId Source node id
 * @param receiverNodeId Receiver node id
 * @param slot Receiver's slot id
 *
 * Updates the widget to display a new connection. Called when the user selects
 * a different connection (so that we don't need to create a new widget for
 * every single connection).
 */
void ConnectionWidget::setNodes(int sourceNodeId, int receiverNodeId, int slot)
{
   this->sourceNodeId = sourceNodeId;
   this->receiverNodeId = receiverNodeId;
   this->slot = slot;

   nodeSourceLabel->setText(QString("%1").arg(sourceNodeId));
   nodeReceiverLabel->setText(QString("%1").arg(receiverNodeId));
   nodeSlotLabel->setText(QString("%1").arg(slot + 1));

   TextureProject* project = widgetmanager->getTextureProject();
   if (project != nullptr) {
      TextureNodePtr node;
      node = project->getNode(sourceNodeId);
      if (!node.isNull()) {
         nodeSourceLabel->setText(QString("%1\n(%2)")
                                  .arg(node->getName(), node->getGeneratorName()));
      }
      node = project->getNode(receiverNodeId);
      if (!node.isNull()) {
         nodeReceiverLabel->setText(QString("%1\n(%2)")
                                    .arg(node->getName(), node->getGeneratorName()));
      }
   }
}


