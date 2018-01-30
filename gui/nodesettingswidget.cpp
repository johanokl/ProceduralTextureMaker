/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPushButton>
#include <QLabel>
#include <QSpacerItem>
#include <QString>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QSlider>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QtGlobal>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QScrollArea>
#include <algorithm>
#include "gui/nodesettingswidget.h"
#include "gui/iteminfopanel.h"
#include "gui/qdoubleslider.h"
#include "core/textureproject.h"
#include "core/texturenode.h"
#include "generators/texturegenerator.h"
#include "global.h"

/**
 * @brief NodeSettingsWidget::NodeSettingsWidget
 * @param widgetmanager
 * @param id Node id
 */
NodeSettingsWidget::NodeSettingsWidget(ItemInfoPanel* widgetmanager, int id)
{
   this->widgetmanager = widgetmanager;
   this->id = id;
   texNode = widgetmanager->getTextureProject()->getNode(id);

   layout = new QVBoxLayout(this);
   this->setLayout(layout);

   scrollarea = new QScrollArea;
   scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   scrollarea->setWidgetResizable(true);
   contents = new QWidget;
   contentsLayout = new QVBoxLayout(contents);
   contentsLayout->setContentsMargins(0, 0, 0, 0);
   layout->setContentsMargins(0, 0, 0, 0);
   scrollarea->setFrameShape(QFrame::NoFrame);
   layout->addWidget(scrollarea);
   scrollarea->setWidget(contents);

   nodeInfoWidget = new QGroupBox("Node info");
   nodeInfoLayout = createGroupLayout();
   nodeInfoWidget->setLayout(nodeInfoLayout);
   nodeNameLineEdit = new QLineEdit;
   nodeInfoLayout->addRow("Name:", nodeNameLineEdit);

   generatorNameLabel = new QLabel;
   nodeInfoLayout->addRow("Generator:", generatorNameLabel);

   contentsLayout->addWidget(nodeInfoWidget);
   QObject::connect(static_cast<QLineEdit*>(nodeNameLineEdit),
                    static_cast<void (QLineEdit::*)(void)>(&QLineEdit::returnPressed),
                    [=](void) { if (texNode->getName() != nodeNameLineEdit->text()) { this->saveSettings(); }});

   sourceButtonsWidget = new QGroupBox("Source nodes");
   sourceButtonsLayout = new QGridLayout;
   sourceButtonsWidget->setLayout(sourceButtonsLayout);
   contentsLayout->addWidget(sourceButtonsWidget);

   for (int i = 0; i < 5; i++) {
      QLabel* slotLabel = new QLabel("");
      sourceButtonsLayout->addWidget(slotLabel, i, 0);
      slotLabel->hide();
      QPushButton* slotButton = new QPushButton;
      SourceRemover* newRemover = new SourceRemover(this, i);
      sourceButtonsLayout->addWidget(slotButton, i, 1);
      QObject::connect(slotButton, SIGNAL(clicked()), newRemover, SLOT(activate()));
      slotButton->hide();
      sourceSlotButtons.push_back(slotButton);
      sourceSlotLabels.push_back(slotLabel);
   }
   swapSlotButton = new QPushButton("Swap slots");
   sourceButtonsLayout->addWidget(swapSlotButton, sourceSlotButtons.size(), 1);
   QObject::connect(swapSlotButton, SIGNAL(clicked()), this, SLOT(swapSlots()));
   swapSlotButton->hide();

   settingsWidget = new QGroupBox("Generator settings");
   settingsLayout = createGroupLayout();
   settingsWidget->setLayout(settingsLayout);
   contentsLayout->addWidget(settingsWidget);

   QSpacerItem* spaceritem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   contentsLayout->addItem(spaceritem);

   generatorUpdated();
   settingsUpdated();
   slotsUpdated();
}

/**
 * @brief NodeSettingsWidget::createGroupLayout
 * @return New layout
 *
 * Creates and styles a new layout.
 */
QFormLayout* NodeSettingsWidget::createGroupLayout()
{
   QFormLayout* layout = new QFormLayout;
   layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
   layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
   layout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
   layout->setLabelAlignment(Qt::AlignRight);
   return layout;
}

/**
 * @brief NodeSettingsWidget::saveSettings
 * @param id Not used.
 *
 * Reads all widget's values and sends them to the
 * TextureNode object's set settings function.
 */
void NodeSettingsWidget::saveSettings()
{
   if (texNode->getName() != nodeNameLineEdit->text()) {
      texNode->setName(nodeNameLineEdit->text());
      QSetIterator<int> receiveriter = texNode->getReceivers();
      while (receiveriter.hasNext()) {
         widgetmanager->sourceUpdated(receiveriter.next());
      }
   }
   TextureNodeSettings nodeSettings = texNode->getSettings();
   QMapIterator<QString, QWidget*> settingElementIterator(settingElements);
   while (settingElementIterator.hasNext()) {
      settingElementIterator.next();
      QString settingsId = settingElementIterator.key();
      QWidget* settingsWidget = settingElementIterator.value();
      QString settingsNewValueString;
      QPushButton* settingsWidgetPushButton = dynamic_cast<QPushButton*>(settingsWidget);
      QLineEdit* settingsWidgetLineEdit = dynamic_cast<QLineEdit*>(settingsWidget);
      QDoubleSpinBox* settingsWidgetDoubleSpinbox = dynamic_cast<QDoubleSpinBox*>(settingsWidget);
      QSpinBox* settingsWidgetSpinbox = dynamic_cast<QSpinBox*>(settingsWidget);
      QComboBox* settingsWidgetCombobox = dynamic_cast<QComboBox*>(settingsWidget);
      if (settingsWidgetPushButton) {
         settingsNewValueString = settingValues[settingsId];
      } else if (settingsWidgetLineEdit) {
         settingsNewValueString = settingsWidgetLineEdit->text();
      } else if (settingsWidgetDoubleSpinbox) {
         settingsNewValueString = QString::number(settingsWidgetDoubleSpinbox->value());
      } else if (settingsWidgetSpinbox) {
         settingsNewValueString = QString::number(settingsWidgetSpinbox->value());
      } else if (settingsWidgetCombobox) {
         settingsNewValueString = settingsWidgetCombobox->currentText();
      }
      QVariant settingVariant;
      switch (texNode->getGenerator()->getSettings().value(settingsId).defaultvalue.type()) {
      case QVariant::Type::Int:
         settingVariant = QVariant(settingsNewValueString.toInt());
         break;
      case QVariant::Type::Double:
         settingVariant = QVariant(settingsNewValueString.toDouble());
         break;
      case QVariant::Type::Color:
         settingVariant = QVariant(QColor(settingsNewValueString));
         break;
      case QVariant::Type::String:
      case QVariant::Type::StringList:
         settingVariant = QVariant(settingsNewValueString);
         break;
      default:
         INFO_MSG("Type not found");
      }
      nodeSettings[settingsId] = settingVariant;
   }
   texNode->setSettings(nodeSettings);
}

/**
 * @brief NodeSettingsWidget::swapSlots
 * Moves the sources around between the slots.
 */
void NodeSettingsWidget::swapSlots()
{
   QMap<int, int> sources = texNode->getSources();
   for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
      texNode->setSourceSlot(i, 0);
   }
   for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
      int source = sources.value((i + 1) % (texNode->getNumSourceSlots()));
      texNode->setSourceSlot(i, source);
   }
}

/**
 * @brief NodeSettingsWidget::colorDialog
 * @param settingsId The name of the setting.
 *
 * Opens a color selection dialog popup and saves the result.
 */
void NodeSettingsWidget::colorDialog(QString settingsId)
{
   QColor initColor = Qt::white;
   if (settingValues.contains(settingsId)) {
      initColor = QColor(settingValues[settingsId]);
   }
   const QColor color = QColorDialog::getColor(initColor, this, "Select Color", QColorDialog::ShowAlphaChannel);
   if (color.isValid()) {
      QPushButton* button = dynamic_cast<QPushButton*>(settingElements[settingsId]);
      settingValues[settingsId] = color.name(QColor::HexArgb);
      styleButton(button, color);
      this->saveSettings();
   }
}

/**
 * @brief NodeSettingsWidget::styleButton
 * @param button The QPushButton to be styled.
 * @param color Background color
 *
 * Styles the color selection buttons. The foreground text color is
 * white or black depending on the background color.
 */
void NodeSettingsWidget::styleButton(QPushButton* button, QColor color)
{
   if (!button) {
      return;
   }
   QString fontColor("#ffffff");
   // Cool formula for how humans percieve colors, if they are dark or light.
   if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
      fontColor = "#000000";
   }
   button->setStyleSheet(QString("background-color: %1; color: %2")
                         .arg(color.name()).arg(fontColor));
   button->setText(color.name().append(QString("%1").arg(color.alpha(), 2, 16, QLatin1Char('0'))));
}

/**
 * @brief settingsComperator
 * Comparator function for qSort, used for sorting the settings based on the order attribute.
 */
bool settingsComperator(const TextureGeneratorSetting &v1, const TextureGeneratorSetting &v2)
{
   return v1.order < v2.order;
}

/**
 * @brief operator ==
 * Helper function needed for QMap.
 */
bool operator==(const TextureGeneratorSetting& lhs, const TextureGeneratorSetting& rhs)
{
   return lhs.name == rhs.name && lhs.order == lhs.order;
}

/**
 * @brief NodeSettingsWidget::generatorUpdated
 *
 * A generator has been defined for the node. Creates all the settings
 * value widgets for the generator.
 */
void NodeSettingsWidget::generatorUpdated()
{
   TextureGenerator* generator = texNode->getGenerator();
   generatorNameLabel->setText(generator->getName());

   if (generator->getNumSourceSlots() > 0) {
      sourceButtonsWidget->show();
   } else {
      sourceButtonsWidget->hide();
   }
   for (int i = 0; i < generator->getNumSourceSlots() &&
         i <  (sourceSlotLabels.count() - 1); i++) {
      sourceSlotLabels[i]->setText(generator->getSlotName(i).append(":"));
   }

   // Remove all previous settings value widget, if any.
   QMapIterator<QString, QLabel*> settingLabelIterator(settingLabels);
   while (settingLabelIterator.hasNext()) {
      settingsLayout->removeWidget(settingLabelIterator.next().value());
   }
   settingLabels.clear();
   QMapIterator<QString, QWidget*> settingElementIterator(settingElements);
   while (settingElementIterator.hasNext()) {
      settingsLayout->removeWidget(settingElementIterator.next().value());
   }
   settingElements.clear();
   QMapIterator<QString, QWidget*> settingSlidersIterator(settingSliders);
   while (settingSlidersIterator.hasNext()) {
      settingsLayout->removeWidget(settingSlidersIterator.next().value());
   }
   settingSliders.clear();

   TextureGeneratorSettings settings = generator->getSettings();
   QList<TextureGeneratorSetting> settingsvalues = settings.values();
   qSort(settingsvalues.begin(), settingsvalues.end(), settingsComperator);
   QListIterator<TextureGeneratorSetting> settingsIterator(settingsvalues);

   while (settingsIterator.hasNext()) {
      TextureGeneratorSetting currSetting = settingsIterator.next();
      QString settingsId = settings.key(currSetting);
      QLabel* newLabel = new QLabel(currSetting.name.append(":"));
      QWidget* newWidget;
      switch (currSetting.defaultvalue.type()) {
      case QVariant::Type::String:
         newWidget = new QLineEdit;
         QObject::connect(newWidget, SIGNAL(returnPressed(int)), this, SLOT(saveSettings()));
         break;
      case QVariant::Type::StringList:
         newWidget = new QComboBox;
         static_cast<QComboBox*>(newWidget)->addItems(currSetting.defaultvalue.toStringList());
         static_cast<QComboBox*>(newWidget)->setCurrentIndex(currSetting.defaultindex);
         QObject::connect(newWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(saveSettings()));
         break;
      case QVariant::Type::Double:
         newWidget = new QDoubleSpinBox;
         QObject::connect(newWidget, SIGNAL(valueChanged(double)), this, SLOT(saveSettings()));
         break;
      case QVariant::Type::Color:
         newWidget = new QPushButton("Color");
         QObject::connect(static_cast<QPushButton*>(newWidget),
                          static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                          [=](bool) {
            this->colorDialog(settingsId);
            static_cast<QPushButton*>(newWidget)->setChecked(false);
            static_cast<QPushButton*>(newWidget)->setDown(false);
         });
         break;
      default:
         newWidget = new QSpinBox;
         QObject::connect(newWidget, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
      }
      settingsLayout->addRow(newLabel, newWidget);
      if (!currSetting.max.isNull()) {
         QDoubleSpinBox* doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(newWidget);
         QSpinBox* spinBox = dynamic_cast<QSpinBox*>(newWidget);
         if (doubleSpinBox) {
            QDoubleSlider* newSlider = new QDoubleSlider();
            newSlider->blockSignals(true);
            newSlider->setDoubleMinimum(currSetting.min.toDouble());
            newSlider->setDoubleMaximum(currSetting.max.toDouble());
            settingsLayout->addRow("", newSlider);
            settingSliders[settingsId] = newSlider;
            doubleSpinBox->setMinimum(currSetting.min.toDouble());
            doubleSpinBox->setMaximum(currSetting.max.toDouble());
            QObject::connect(newSlider, SIGNAL(doubleValueChanged(double)), doubleSpinBox, SLOT(setValue(double)));
            QObject::connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                             [=](double value) {
               newSlider->blockSignals(true);
               newSlider->setDoubleValue(value);
               newSlider->blockSignals(false);
            });
            newSlider->blockSignals(false);
         } else if (spinBox) {
            QSlider* newSlider = new QSlider(Qt::Horizontal);
            spinBox->blockSignals(true);
            newSlider->blockSignals(true);
            newSlider->setMinimum(currSetting.min.toInt());
            newSlider->setMaximum(currSetting.max.toInt());
            settingsLayout->addRow("", newSlider);
            settingSliders[settingsId] = newSlider;
            spinBox->setMinimum(currSetting.min.toInt());
            spinBox->setMaximum(currSetting.max.toInt());
            QObject::connect(spinBox, SIGNAL(valueChanged(int)), newSlider, SLOT(setValue(int)));
            QObject::connect(newSlider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            spinBox->blockSignals(false);
            newSlider->blockSignals(true);
         }
      }
      settingLabels[settingsId] = newLabel;
      settingElements[settingsId] = newWidget;
   }

   this->settingsUpdated();
}

/**
 * @brief NodeSettingsWidget::slotsUpdated
 * A sources has been added or removed. Updates the slot buttons and labels.
 */
void NodeSettingsWidget::slotsUpdated()
{
   int numInList = sourceSlotButtons.count();
   int numSlots = texNode->getNumSourceSlots();
   int numConnected = 0;
   for (int i = 0; i < numInList; i++) {
      QPushButton* currButton = sourceSlotButtons[i];
      QLabel* currLabel = sourceSlotLabels[i];
      if (i >= numSlots) {
         currLabel->hide();
         currButton->hide();
      } else {
         currButton->show();
         currLabel->show();
         currLabel->setText(texNode->getGenerator()->getSlotName(i).append(":"));
         int connectedNode = texNode->getSources().value(i);
         if (connectedNode != 0) {
            currLabel->setText(currLabel->text().append(" ")
                               .append(widgetmanager->getTextureProject()->getNode(connectedNode)->getName()));
            currButton->setText("Clear");
            currButton->setFlat(false);
            numConnected++;
         } else {
            currButton->setText("Empty");
            currButton->setFlat(true);
         }
      }
   }
   if (numSlots > 1 && numConnected > 0) {
      swapSlotButton->setVisible(true);
   } else {
      swapSlotButton->setVisible(false);
   }
}

/**
 * @brief NodeSettingsWidget::removeSourceSlot
 * @param slot
 */
void NodeSettingsWidget::removeSourceSlot(int slot)
{
   texNode->setSourceSlot(slot, 0);
}

/**
 * @brief NodeSettingsWidget::settingsUpdated
 *
 * Called when the node's settings have been changed.
 * Updates the widgets' values.
 */
void NodeSettingsWidget::settingsUpdated()
{
   nodeNameLineEdit->setText(texNode->getName());

   QMapIterator<QString, QWidget*> settingElementIterator(settingElements);
   while (settingElementIterator.hasNext()) {
      settingElementIterator.next();
      QString settingsId = settingElementIterator.key();
      QWidget* settingsWidget = settingElementIterator.value();

      QVariant value;
      if (texNode->getSettings().contains(settingsId)) {
         value = texNode->getSettings().value(settingsId);
      } else if (texNode->getGenerator()->getSettings().contains(settingsId)) {
         value = texNode->getSettings().value(settingsId);
      }
      QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(settingsWidget);
      QDoubleSpinBox* doubleSpinbox = dynamic_cast<QDoubleSpinBox*>(settingsWidget);
      QSpinBox* spinbox = dynamic_cast<QSpinBox*>(settingsWidget);
      QPushButton* pushbutton = dynamic_cast<QPushButton*>(settingsWidget);
      QComboBox* combobox = dynamic_cast<QComboBox*>(settingsWidget);

      settingsWidget->blockSignals(true);
      if (lineEdit) {
         lineEdit->setText(value.toString());
      } else if (combobox) {
         int index = combobox->findText(value.toString());
         if (index >= 0) {
            combobox->setCurrentIndex(index);
         }
      } else if (doubleSpinbox) {
         doubleSpinbox->setValue(value.toDouble());
         QDoubleSlider* doubleslider = dynamic_cast<QDoubleSlider*>(settingSliders[settingsId]);
         if (doubleslider) {
            doubleslider->blockSignals(true);
            doubleslider->setDoubleValue(value.toDouble());
            doubleslider->blockSignals(false);
         }
      } else if (spinbox) {
         spinbox->setValue(value.toInt());
         QSlider* slider = dynamic_cast<QSlider*>(settingSliders[settingsId]);
         if (slider) {
            slider->blockSignals(true);
            slider->setValue(value.toInt());
            slider->blockSignals(false);
         }
      } else if (pushbutton && value.type() == QVariant::Type::Color) {
         settingValues[settingsId] = value.toString();
         styleButton(pushbutton, QColor(value.toString()));
      }
      settingsWidget->blockSignals(false);
   }
}
