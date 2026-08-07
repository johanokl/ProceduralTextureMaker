
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/connectionwidget.h"
#include "gui/generatorinfowidget.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/sceneinfowidget.h"
#include <QPushButton>
#include <QVBoxLayout>

ItemInfoPanel::ItemInfoPanel(QWidget* parent, TextureProject* project, EditManager* editManager)
    : QWidget(parent), texproject(project), editManager(editManager) {
   sceneWidget = new SceneInfoWidget(*this);
   lineWidget = new ConnectionWidget(*this);
   generatorWidget = new GeneratorInfoWidget(this);
   auto* layout = new QVBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(lineWidget);
   layout->addWidget(sceneWidget);
   layout->addWidget(generatorWidget);
   setLayout(layout);
   lineWidget->hide();
   generatorWidget->hide();
   sceneWidget->show();
   QObject::connect(texproject, &TextureProject::nodeRemoved, this, &ItemInfoPanel::removeNode);
   QObject::connect(texproject, &TextureProject::nodesDisconnected, this,
                    &ItemInfoPanel::nodesDisconnected);
   QObject::connect(texproject, &TextureProject::nodeAdded, this, &ItemInfoPanel::addNode);
   QObject::connect(texproject, &TextureProject::generatorRemoved, this,
                    [this](const TextureGeneratorPtr& generator) {
                       if (currGenerator == generator) {
                          setActiveNode(0);
                       }
                    });
}

void ItemInfoPanel::addNode(const TextureNodePtr& node) {
   // Update the scene info widget's labels.
   sceneWidget->updateNumNodes();
   QObject::connect(node.data(), &TextureNode::nameUpdated, this, [this, node](int) {
      QSetIterator<int> receivers = node->getReceivers();
      while (receivers.hasNext()) {
         sourceUpdated(receivers.next());
      }
   });
}

void ItemInfoPanel::nodesDisconnected(int sourceNodeId, int receiverNodeId, QString slot) {
   if (currLine == std::tuple<int, int, QString>(sourceNodeId, receiverNodeId, slot)) {
      // The removed connection is currently visible
      setActiveNode(-1);
   }
}

void ItemInfoPanel::removeNode(int id) {
   if (currNodeId == id) {
      // The removed node is currently visible
      setActiveNode(0);
   }
   if (nodes.value(id) != nullptr) {
      NodeSettingsWidget* widget = nodes[id].data();
      widget->hide();
      layout()->removeWidget(widget);
      delete widget;
      nodes.remove(id);
   }
   sceneWidget->updateNumNodes();
}

void ItemInfoPanel::sourceUpdated(int id) {
   NodeSettingsWidget* nodeptr = nodes.value(id).data();
   if (nodeptr != nullptr) {
      nodeptr->slotsUpdated();
   }
}

void ItemInfoPanel::setActiveNode(int id) {
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   lineWidget->hide();
   generatorWidget->hide();
   currGenerator.clear();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(0, 0, QString());

   TextureNodePtr texNode = texproject->getNode(id);
   if (texNode.isNull()) {
      sceneWidget->show();
      return;
   }
   sceneWidget->hide();
   if (!nodes.contains(id)) {
      auto* newWidget = new NodeSettingsWidget(*this, id);
      QObject::connect(texNode.data(), &TextureNode::slotsUpdated, newWidget,
                       &NodeSettingsWidget::slotsUpdated);
      QObject::connect(texNode.data(), &TextureNode::generatorUpdated, newWidget,
                       &NodeSettingsWidget::generatorUpdated);
      QObject::connect(texNode.data(), &TextureNode::settingsUpdated, newWidget,
                       &NodeSettingsWidget::settingsUpdated);
      QObject::connect(texNode.data(), &TextureNode::nameUpdated, newWidget,
                       &NodeSettingsWidget::settingsUpdated);
      nodes[id] = newWidget;
      layout()->addWidget(newWidget);
   }
   currNodeId = id;
   currWidget = nodes[id];
   currWidget->show();
}

void ItemInfoPanel::setActiveLine(int sourceNodeId, int receiverNodeId, QString slot) {
   if (sourceNodeId == -1 || receiverNodeId == -1) {
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   sceneWidget->hide();
   generatorWidget->hide();
   currGenerator.clear();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(sourceNodeId, receiverNodeId, slot);
   lineWidget->setNodes(sourceNodeId, receiverNodeId, slot);
   lineWidget->show();
}

void ItemInfoPanel::setActiveGenerator(const TextureGeneratorPtr& generator) {
   if (generator.isNull()) {
      setActiveNode(0);
      return;
   }
   if (currWidget != nullptr) {
      currWidget->hide();
   }
   sceneWidget->hide();
   lineWidget->hide();
   currWidget = nullptr;
   currNodeId = 0;
   currLine = std::tuple<int, int, QString>(0, 0, QString());
   currGenerator = generator;
   generatorWidget->setGenerator(generator);
   generatorWidget->show();
}
