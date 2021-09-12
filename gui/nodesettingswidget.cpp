/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "base/textureproject.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/qdoubleslider.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

/**
 * @brief NodeSettingsWidget::NodeSettingsWidget
 * @param widgetmanager
 * @param id Node id
 */
NodeSettingsWidget::NodeSettingsWidget(ItemInfoPanel* widgetmanager, int id)
{
   this->widgetmanager = widgetmanager;
   this->id = id;
   saveDisabled = false;
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
                    [=]() { if (texNode->getName() != nodeNameLineEdit->text()) { this->saveSettings(); }});

   sourceButtonsWidget = new QGroupBox("Source nodes");
   sourceButtonsLayout = new QGridLayout;
   sourceButtonsWidget->setLayout(sourceButtonsLayout);
   contentsLayout->addWidget(sourceButtonsWidget);

   for (int i = 0; i < 10; i++) {
      QLabel* slotLabel = new QLabel("");
      sourceButtonsLayout->addWidget(slotLabel, i, 0);
      auto* slotButton = new QPushButton;
      sourceButtonsLayout->addWidget(slotButton, i, 1);
      QObject::connect(slotButton, &QPushButton::clicked,
                       [=]() { texNode->setSourceSlot(i, 0); });
      sourceSlotButtons.push_back(slotButton);
      sourceSlotLabels.push_back(slotLabel);
      slotButton->hide();
      slotLabel->hide();
   }
   swapSlotButton = new QPushButton("Swap slots");
   sourceButtonsLayout->addWidget(swapSlotButton, sourceSlotButtons.size(), 1);
   QObject::connect(swapSlotButton, &QPushButton::clicked,
                    this, &NodeSettingsWidget::swapSlots);
   swapSlotButton->hide();

   settingsWidget = new QGroupBox("Generator settings");
   settingsLayout = createGroupLayout();
   settingsWidget->setLayout(settingsLayout);
   contentsLayout->addWidget(settingsWidget);

   auto* spaceritem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
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
   auto* layout = new QFormLayout;
   layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
   layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
   layout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
   layout->setLabelAlignment(Qt::AlignRight);
   return layout;
}

/**
 * @brief NodeSettingsWidget::saveSettings
 *
 * Reads all widget's values and sends them to the
 * TextureNode object's set settings function.
 */
void NodeSettingsWidget::saveSettings()
{
   if (saveDisabled) {
      return;
   }
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
      if (!texNode->getGenerator()->getSettings().contains(settingsId)) {
         continue;
      }
      QWidget* settingWidget = settingElementIterator.value();
      switch (texNode->getGenerator()->getSettings().value(settingsId).defaultvalue.type()) {
      case QVariant::Type::Int: {
         QSpinBox *spinbox = dynamic_cast<QSpinBox*>(settingWidget);
         if (spinbox) {
            nodeSettings[settingsId] = spinbox->value();
         }
         break;
      }
      case QVariant::Type::Double: {
         QDoubleSpinBox *doublespinbox = dynamic_cast<QDoubleSpinBox*>(settingWidget);
         if (doublespinbox) {
            nodeSettings[settingsId] = doublespinbox->value();
         }
         break;
      }
      case QVariant::Type::Bool: {
         QCheckBox *checkbox = dynamic_cast<QCheckBox*>(settingWidget);
         if (checkbox) {
            nodeSettings[settingsId] = checkbox->isChecked();
         }
         break;
      }
      case QVariant::Type::Color: {
         QPushButton *pushbutton = dynamic_cast<QPushButton*>(settingWidget);
         if (pushbutton) {
            nodeSettings[settingsId] = QColor(settingValues[settingsId]);
         }
         break;
      }
      case QVariant::Type::String: {
         QLineEdit *lineedit = dynamic_cast<QLineEdit*>(settingWidget);
         if (lineedit) {
            nodeSettings[settingsId] = lineedit->text();
         }
         break;
      }
      case QVariant::Type::StringList: {
         QComboBox *combobox  = dynamic_cast<QComboBox*>(settingWidget);
         if (combobox) {
            nodeSettings[settingsId] = combobox->currentText();
         }
         break;
      }
      default:
        INFO_MSG("Type not found. Property id=" + settingsId);
      }
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
void NodeSettingsWidget::colorDialog(const QString& settingsId)
{
   QColor initColor = Qt::white;
   if (settingValues.contains(settingsId)) {
      initColor = QColor(settingValues[settingsId]);
   }
   const QColor color = QColorDialog::getColor(initColor, this, "Select Color", QColorDialog::ShowAlphaChannel);
   if (color.isValid()) {
      auto* button = dynamic_cast<QPushButton*>(settingElements[settingsId]);
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
void NodeSettingsWidget::styleButton(QPushButton* button, const QColor& color)
{
   if (button) {
      QString fontColor("#ffffff");
      // Cool formula for how humans percieve colors, if they are dark or light.
      if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
         fontColor = "#000000";
      }
      button->setStyleSheet(QString("background-color: %1; color: %2")
                            .arg(color.name(), fontColor));
      button->setText(color.name().append(QString("%1").arg(color.alpha(), 2, 16, QLatin1Char('0'))));
   }
}

/**
 * @brief settingsComperator
 * Comparator function for qSort, used for sorting the settings based on the order attribute.
 */
bool settingsComperator(const TextureGeneratorSetting& v1, const TextureGeneratorSetting& v2)
{
   return v1.order < v2.order;
}

/**
 * @brief operator ==
 * Helper function needed for QMap.
 */
bool operator==(const TextureGeneratorSetting& lhs, const TextureGeneratorSetting& rhs)
{
   return lhs.name == rhs.name && lhs.order == rhs.order;
}

/**
 * @brief NodeSettingsWidget::generatorUpdated
 *
 * A generator has been defined for the node. Creates all the settings
 * value widgets for the generator.
 */
void NodeSettingsWidget::generatorUpdated()
{
   TextureGeneratorPtr generator = texNode->getGenerator();
   generatorNameLabel->setText(generator->getName());

   if (generator->getNumSourceSlots() > 0) {
      sourceButtonsWidget->show();
   } else {
      sourceButtonsWidget->hide();
   }
   for (int i = 0; i < generator->getNumSourceSlots() &&
        i <  (sourceSlotLabels.count() - 1); i++) {
      sourceSlotLabels[i]->setText(generator->getSlotName(i) + ":");
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
   std::sort(settingsvalues.begin(), settingsvalues.end(), settingsComperator);
   QListIterator<TextureGeneratorSetting> settingsIterator(settingsvalues);

   while (settingsIterator.hasNext()) {
      TextureGeneratorSetting currSetting = settingsIterator.next();
      QString settingsId = settings.key(currSetting);
      QWidget* newWidget;
      switch (currSetting.defaultvalue.type()) {
      case QVariant::Type::String:
         newWidget = new QLineEdit;
         QObject::connect(static_cast<QLineEdit*>(newWidget), &QLineEdit::returnPressed,
                          this, &NodeSettingsWidget::saveSettings);
         break;
      case QVariant::Type::StringList:
         newWidget = new QComboBox;
         static_cast<QComboBox*>(newWidget)->addItems(currSetting.defaultvalue.toStringList());
         static_cast<QComboBox*>(newWidget)->setCurrentIndex(currSetting.defaultindex);
         QObject::connect(static_cast<QComboBox*>(newWidget),
                          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                          [=](int i) { Q_UNUSED(i); this->saveSettings(); });
         break;
      case QVariant::Type::Bool:
         newWidget = new QCheckBox;
         QObject::connect(static_cast<QCheckBox*>(newWidget),
                          static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled),
                          [=](bool i) { Q_UNUSED(i); this->saveSettings(); });
         break;
      case QVariant::Type::Double:
         newWidget = new QDoubleSpinBox;
         static_cast<QDoubleSpinBox*>(newWidget)->setSingleStep(0.1);
         QObject::connect(static_cast<QDoubleSpinBox*>(newWidget),
                          static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                          [=](double i) { Q_UNUSED(i); this->saveSettings(); });
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
         QObject::connect(static_cast<QSpinBox*>(newWidget),
                          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                          [=](int i) { Q_UNUSED(i); this->saveSettings(); });
      }
      QLabel* newLabel = new QLabel(currSetting.name + ":");
      settingLabels[settingsId] = newLabel;
      settingElements[settingsId] = newWidget;
      settingsLayout->addRow(newLabel, newWidget);
      if (!currSetting.max.isNull()) {
         auto* doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(newWidget);
         auto* spinBox = dynamic_cast<QSpinBox*>(newWidget);
         if (doubleSpinBox) {
            auto* newSlider = new QDoubleSlider();
            newSlider->blockSignals(true);
            doubleSpinBox->blockSignals(true);
            newSlider->setDoubleMinimum(currSetting.min.toDouble());
            newSlider->setDoubleMaximum(currSetting.max.toDouble());
            settingsLayout->addRow("", newSlider);
            settingSliders[settingsId] = newSlider;
            doubleSpinBox->setMinimum(currSetting.min.toDouble());
            doubleSpinBox->setMaximum(currSetting.max.toDouble());
            QObject::connect(newSlider, &QDoubleSlider::doubleValueChanged,
                             doubleSpinBox, &QDoubleSpinBox::setValue);
            QObject::connect(doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                             [=](double value) {
               newSlider->blockSignals(true);
               newSlider->setDoubleValue(value);
               newSlider->blockSignals(false);
            });
            doubleSpinBox->blockSignals(false);
            newSlider->blockSignals(false);
         } else if (spinBox) {
            auto* newSlider = new QSlider(Qt::Horizontal);
            spinBox->blockSignals(true);
            newSlider->blockSignals(true);
            newSlider->setMinimum(currSetting.min.toInt());
            newSlider->setMaximum(currSetting.max.toInt());
            settingsLayout->addRow("", newSlider);
            settingSliders[settingsId] = newSlider;
            spinBox->setMinimum(currSetting.min.toInt());
            spinBox->setMaximum(currSetting.max.toInt());
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                             newSlider, &QSlider::setValue);
            QObject::connect(newSlider, &QSlider::valueChanged,
                             spinBox, &QSpinBox::setValue);
            spinBox->blockSignals(false);
            newSlider->blockSignals(false);
         }
      }
      QString groupId;
      if (!(currSetting.group.isEmpty()) &&
          ((settingsIterator.hasNext() && settingsIterator.peekNext().group != currSetting.group) ||
           !settingsIterator.hasNext())) {
         auto* groupCheckbox = new QCheckBox;
         groupId = currSetting.group + "groupcheckbox";
         QLabel* newLabel = new QLabel(QString("Align ").append(currSetting.group).append(":"));
         settingLabels[groupId] = newLabel;
         settingElements[groupId] = groupCheckbox;
         settingsLayout->addRow(newLabel, groupCheckbox);
         QObject::connect(groupCheckbox, &QCheckBox::toggled,
                          [=](bool val) { this->setGroupAlignment(currSetting.group, val); });

      }
      if (!currSetting.enabler.isEmpty()) {
         auto* enablerWidget = dynamic_cast<QCheckBox*>(settingElements[currSetting.enabler]);
         if (enablerWidget) {
            if (settingElements[settingsId]) {
               settingElements[settingsId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled,
                                settingElements[settingsId], &QWidget::setEnabled);
            }
            if (settingSliders[settingsId]) {
               settingSliders[settingsId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled,
                                settingSliders[settingsId], &QWidget::setEnabled);
            }
            if (settingElements[groupId]) {
               settingElements[groupId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled,
                                settingElements[groupId], &QWidget::setEnabled);
            }
         }
      }
   }
   this->settingsUpdated();
}

/**
 * @brief NodeSettingsWidget::toogleAligned
 * @param group
 * @param aligned
 */
void NodeSettingsWidget::setGroupAlignment(const QString& group, bool aligned)
{
   TextureGeneratorSettings settings = texNode->getGenerator()->getSettings();
   QList<TextureGeneratorSetting> settingsvalues = settings.values();
   std::sort(settingsvalues.begin(), settingsvalues.end(), settingsComperator);
   QListIterator<TextureGeneratorSetting> settingsIterator(settingsvalues);
   QString firstsetting;
   while (settingsIterator.hasNext()) {
      TextureGeneratorSetting currSetting = settingsIterator.next();
      QString settingsId = settings.key(currSetting);
      if (currSetting.group == group) {
         if (firstsetting.isEmpty()) {
            firstsetting = settingsId;
            continue;
         }
         QWidget* currwidget = settingElements[settingsId];
         QWidget* firstwidget = settingElements[firstsetting];
         if (currwidget && firstwidget) {
            firstwidget->disconnect(currwidget);
            firstwidget->disconnect(this);
            if (aligned) {
               if (currSetting.defaultvalue.type() == QVariant::Type::Int) {
                  QObject::connect(static_cast<QSpinBox*>(firstwidget),
                                   static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                                   static_cast<QSpinBox*>(currwidget), &QSpinBox::setValue);
                  QObject::connect(static_cast<QSpinBox*>(firstwidget),
                                   static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                                   [=](int i) { Q_UNUSED(i); this->saveSettings(); });
               } else if (currSetting.defaultvalue.type() == QVariant::Type::Double) {
                  QObject::connect(static_cast<QDoubleSpinBox*>(firstwidget),
                                   static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                                   static_cast<QDoubleSpinBox*>(currwidget), &QDoubleSpinBox::setValue);
                  QObject::connect(static_cast<QDoubleSpinBox*>(firstwidget),
                                   static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                                   [=](double i) { Q_UNUSED(i); this->saveSettings(); });
               }
            }
            currwidget->setEnabled(!aligned);
            currwidget->blockSignals(aligned);
         }
         QWidget* currslider = settingSliders[settingsId];
         QWidget* firstslider = settingSliders[firstsetting];
         if (currslider && firstslider) {
            firstslider->disconnect(currslider);
            if (aligned) {
               if (currSetting.defaultvalue.type() == QVariant::Type::Int) {
                  QObject::connect(static_cast<QSlider*>(firstslider), &QSlider::valueChanged,
                                   static_cast<QSlider*>(currslider), &QSlider::setValue);
               } else if (currSetting.defaultvalue.type() == QVariant::Type::Double) {
                  QObject::connect(static_cast<QDoubleSlider*>(firstslider), &QDoubleSlider::doubleValueChanged,
                                   static_cast<QDoubleSlider*>(currslider), &QDoubleSlider::setDoubleValue);
               }
            }
            currslider->setEnabled(!aligned);
            currslider->blockSignals(aligned);
         }
      }
   }
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
         currLabel->setText(texNode->getGenerator()->getSlotName(i) + ":");
         int connectedNode = texNode->getSources().value(i);
         if (connectedNode != 0) {
            currLabel->setText(currLabel->text().append(" ")
                               .append(widgetmanager->getTextureProject()->
                                       getNode(connectedNode)->getName()));
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
      if (!texNode->getGenerator()->getSettings().contains(settingsId)) {
         continue;
      }
      QVariant defaultvalue = texNode->getSettings().value(settingsId);
      QVariant nodevalue = defaultvalue;
      if (texNode->getSettings().contains(settingsId)) {
         nodevalue = texNode->getSettings().value(settingsId);
      }

      QWidget* settingsWidget = settingElementIterator.value();
      saveDisabled = true;
      int index;
      switch (defaultvalue.type()) {
      case QVariant::Type::Int: {
         QSpinBox *spinbox = dynamic_cast<QSpinBox*>(settingsWidget);
         if (spinbox) {
            spinbox->setValue(nodevalue.toInt());
         }
         break;
      }
      case QVariant::Type::Double: {
         QDoubleSpinBox *doublespinbox = dynamic_cast<QDoubleSpinBox*>(settingsWidget);
         if (doublespinbox) {
            doublespinbox->setValue(nodevalue.toDouble());
         }
         break;
      }
      case QVariant::Type::Color: {
         QPushButton *pushbutton = dynamic_cast<QPushButton*>(settingsWidget);
         if (pushbutton) {
            settingValues[settingsId] = nodevalue.toString();
            styleButton(pushbutton, QColor(nodevalue.toString()));
         }
         break;
      }
      case QVariant::Type::Bool: {
         QCheckBox *checkbox = dynamic_cast<QCheckBox*>(settingsWidget);
         if (checkbox) {
            checkbox->setChecked(nodevalue.toBool());
         }
         break;
      }
      case QVariant::Type::String: {
         QLineEdit *lineedit = dynamic_cast<QLineEdit*>(settingsWidget);
         if (lineedit) {
            lineedit->setText(nodevalue.toString());
         }
         break;
      }
      case QVariant::Type::StringList: {
         QComboBox *combobox = dynamic_cast<QComboBox*>(settingsWidget);
         if (combobox) {
            index = combobox->findText(nodevalue.toString());
            if (index >= 0) {
               combobox->setCurrentIndex(index);
            }
         }
         break;
      }
      default: {
         INFO_MSG("Type not found. Property id=" + settingsId);
      }
      }
      saveDisabled = false;
   }
}
