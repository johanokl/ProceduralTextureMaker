/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef NODESETTINGSWIDGET_H
#define NODESETTINGSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QMap>
#include "core/texturenode.h"

class TextureProject;
class ItemInfoPanel;
class QLabel;
class QPushButton;
class SourceRemover;
class QSpacerItem;
class QGroupBox;
class QLineEdit;
class QFormLayout;
class QSlider;
class QDoubleSlider;
class QScrollArea;

/**
 * @brief The NodeSettingsWidget class
 *
 * Vertical panel for changing the values sent to the node's generator.
 * Has widgets for displaying and setting integer, double, color and
 * string/enum values.
 */
class NodeSettingsWidget : public QWidget
{
   Q_OBJECT

 public:
   NodeSettingsWidget(ItemInfoPanel* widgetmanager, int id);
   virtual ~NodeSettingsWidget() {}

public slots:
   void settingsUpdated();
   void slotsUpdated();
   void generatorUpdated();
   void saveSettings();
   void removeSourceSlot(int slot);
   void colorDialog(QString settingsId);
   void swapSlots();

private:
   QFormLayout* createGroupLayout();
   void styleButton(QPushButton* button, QColor color);

   ItemInfoPanel* widgetmanager;
   int id;
   TextureNodePtr texNode;

   QMap<QString, QLabel*> settingLabels;
   QMap<QString, QWidget*> settingElements;
   QMap<QString, QWidget*> settingSliders;
   QMap<QString, QString> settingValues;

   QScrollArea* scrollarea;
   QWidget* contents;
   QVBoxLayout* contentsLayout;

   QFormLayout* settingsLayout;
   QGroupBox* settingsWidget;

   QLabel* generatorNameLabel;
   QGroupBox* nodeInfoWidget;
   QFormLayout* nodeInfoLayout;
   QLineEdit* nodeNameLineEdit;

   QGroupBox* sourceButtonsWidget;
   QGridLayout* sourceButtonsLayout;
   QList<QLabel*> sourceSlotLabels;
   QPushButton* swapSlotButton;
   QList<QPushButton*> sourceSlotButtons;
   QVBoxLayout* layout;
};

class SourceRemover : public QObject
{
   Q_OBJECT
public:
   SourceRemover(NodeSettingsWidget* node, int slot) : node(node), slot(slot) {}
   NodeSettingsWidget* node;
   int slot;
public slots:
   void activate() {
      node->removeSourceSlot(slot);
   }
};


#endif // NODESETTINGSWIDGET_H
