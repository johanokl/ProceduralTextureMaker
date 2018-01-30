/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QMap>
#include "core/texturenode.h"

class TextureProject;
class ItemInfoPanel;
class QLabel;
class QPushButton;
class QSpacerItem;
class QGroupBox;
class QLineEdit;

/**
 * @brief The ConnectionWidget class
 *
 * Simple widget that displays a connections. Currently just lists
 * the names of the source and receiver nodes with a button for
 * disconnecting them.
 *
 * @todo Display the nodes' images similar to PreviewPanel
 */
class ConnectionWidget : public QWidget
{
   Q_OBJECT

 public:
   ConnectionWidget(ItemInfoPanel* widgetmanager);
   virtual ~ConnectionWidget() {}
   void setNodes(int sourceNodeId, int receiverNodeId, int slot);

public slots:
   void disconnectNodes();

private:
   ItemInfoPanel* widgetmanager;
   int sourceNodeId;
   int receiverNodeId;
   int slot;

   QGroupBox* nodeInfoWidget;
   QGridLayout* nodeInfoLayout;
   QLabel* nodeSourceLabel;
   QLabel* nodeReceiverLabel;
   QLabel* nodeSlotLabel;
   QVBoxLayout* layout;
};

#endif // CONNECTIONWIDGET_H
