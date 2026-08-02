
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "base/editmanager.h"
#include "gui/iteminfopanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/qdoubleslider.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMetaType>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QVBoxLayout>

NodeSettingsWidget::NodeSettingsWidget(ItemInfoPanel& widgetmanager, int id)
    : QWidget(&widgetmanager), widgetmanager(widgetmanager), id(id) {
   setObjectName("nodeSettingsInspector");
   texNode = widgetmanager.getTextureProject()->getNode(id);

   auto* layout = new QVBoxLayout(this);
   setLayout(layout);

   auto* scrollarea = new QScrollArea;
   scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   scrollarea->setWidgetResizable(true);
   auto* contents = new QWidget;
   contents->setObjectName("nodeSettingsContents");
   auto* contentsLayout = new QVBoxLayout(contents);
   contentsLayout->setContentsMargins(12, 4, 12, 12);
   contentsLayout->setSpacing(4);
   layout->setContentsMargins(0, 0, 0, 0);
   scrollarea->setFrameShape(QFrame::NoFrame);
   layout->addWidget(scrollarea);
   scrollarea->setWidget(contents);

   auto* nodeInfoWidget = new QGroupBox("Node");
   nodeInfoWidget->setProperty("inspectorSection", true);
   auto* nodeInfoLayout = createGroupLayout();
   nodeInfoWidget->setLayout(nodeInfoLayout);
   nodeNameLineEdit = new QLineEdit;
   nodeInfoLayout->addRow("Name:", nodeNameLineEdit);

   generatorNameLabel = new QLabel;
   generatorNameLabel->setObjectName("generatorBadge");
   nodeInfoLayout->addRow("Generator:", generatorNameLabel);

   contentsLayout->addWidget(nodeInfoWidget);
   QObject::connect(nodeNameLineEdit, &QLineEdit::editingFinished, [this]() {
      auto editManager = this->widgetmanager.getEditManager();
      if (editManager != nullptr) {
         editManager->renameNode(this->id, nodeNameLineEdit->text());
      }
   });

   sourceButtonsWidget = new QGroupBox("Inputs");
   sourceButtonsWidget->setProperty("inspectorSection", true);
   sourceButtonsLayout = new QGridLayout;
   sourceButtonsLayout->setContentsMargins(0, 4, 0, 2);
   sourceButtonsLayout->setHorizontalSpacing(10);
   sourceButtonsLayout->setVerticalSpacing(8);
   sourceButtonsLayout->setColumnStretch(1, 1);
   sourceButtonsWidget->setLayout(sourceButtonsLayout);
   contentsLayout->addWidget(sourceButtonsWidget);

   for (int i = 0; i < 10; i++) {
      QLabel* slotLabel = new QLabel("");
      sourceButtonsLayout->addWidget(slotLabel, i, 0);
      auto* slotButton = new QPushButton;
      sourceButtonsLayout->addWidget(slotButton, i, 1);
      QObject::connect(slotButton, &QPushButton::clicked, [this, i]() {
         auto editManager = this->widgetmanager.getEditManager();
         const QStringList sourceSlots = this->texNode->getSourceSlots();
         if (editManager != nullptr && i < sourceSlots.size()) {
            editManager->setConnection(this->id, sourceSlots.at(i), 0);
         }
      });
      sourceSlotButtons.push_back(slotButton);
      sourceSlotLabels.push_back(slotLabel);
      slotButton->hide();
      slotLabel->hide();
   }
   swapSlotButton = new QPushButton("Swap slots");
   sourceButtonsLayout->addWidget(swapSlotButton, sourceSlotButtons.size(), 1);
   QObject::connect(swapSlotButton, &QPushButton::clicked, this, &NodeSettingsWidget::swapSlots);
   swapSlotButton->hide();

   auto* settingsWidget = new QGroupBox("Properties");
   settingsWidget->setProperty("inspectorSection", true);
   settingsLayout = new QVBoxLayout;
   settingsLayout->setContentsMargins(0, 4, 0, 2);
   settingsLayout->setSpacing(2);
   settingsWidget->setLayout(settingsLayout);
   contentsLayout->addWidget(settingsWidget);

   auto* spaceritem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   contentsLayout->addItem(spaceritem);

   generatorUpdated();
   settingsUpdated();
   slotsUpdated();
}

QFormLayout* NodeSettingsWidget::createGroupLayout() {
   auto* layout = new QFormLayout;
   layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
   layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
   layout->setFormAlignment(Qt::AlignTop);
   layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   layout->setHorizontalSpacing(12);
   layout->setVerticalSpacing(9);
   return layout;
}

QWidget* NodeSettingsWidget::createPropertyRow(QLabel* label, QWidget* editor, QWidget* slider) {
   auto* propertyWidget = new QWidget;
   propertyWidget->setProperty("inspectorProperty", true);
   auto* propertyLayout = new QVBoxLayout(propertyWidget);
   propertyLayout->setContentsMargins(0, 4, 0, 7);
   propertyLayout->setSpacing(6);
   label->setProperty("inspectorLabel", true);

   if (slider != nullptr) {
      auto* valueLayout = new QHBoxLayout;
      valueLayout->setContentsMargins(0, 0, 0, 0);
      valueLayout->setSpacing(8);
      valueLayout->addWidget(label);
      valueLayout->addStretch();
      editor->setMinimumWidth(108);
      editor->setMaximumWidth(128);
      valueLayout->addWidget(editor);
      propertyLayout->addLayout(valueLayout);
      propertyLayout->addWidget(slider);
   } else if (auto* checkbox = dynamic_cast<QCheckBox*>(editor)) {
      QString text = label->text();
      if (text.endsWith(':')) {
         text.chop(1);
      }
      checkbox->setText(text);
      label->hide();
      propertyLayout->addWidget(checkbox);
   } else {
      propertyLayout->addWidget(label);
      propertyLayout->addWidget(editor);
   }
   return propertyWidget;
}

bool NodeSettingsWidget::saveSettings() {
   if (saveDisabled) {
      return false;
   }
   TextureNodeSettings nodeSettings = texNode->getSettings();
   const TextureNodeSettings oldSettings = nodeSettings;
   QMapIterator<QString, QWidget*> settingElementIterator(settingElements);
   while (settingElementIterator.hasNext()) {
      settingElementIterator.next();
      QString settingsId = settingElementIterator.key();
      if (!texNode->getGenerator()->getSettings().contains(settingsId)) {
         continue;
      }
      QWidget* settingWidget = settingElementIterator.value();
      switch (texNode->getGenerator()->getSettings().value(settingsId).defaultvalue.typeId()) {
         case QMetaType::Int: {
            QSpinBox* spinbox = dynamic_cast<QSpinBox*>(settingWidget);
            if (spinbox) {
               nodeSettings[settingsId] = spinbox->value();
            }
            break;
         }
         case QMetaType::Double: {
            QDoubleSpinBox* doublespinbox = dynamic_cast<QDoubleSpinBox*>(settingWidget);
            if (doublespinbox) {
               nodeSettings[settingsId] = doublespinbox->value();
            }
            break;
         }
         case QMetaType::Bool: {
            QCheckBox* checkbox = dynamic_cast<QCheckBox*>(settingWidget);
            if (checkbox) {
               nodeSettings[settingsId] = checkbox->isChecked();
            }
            break;
         }
         case QMetaType::QColor: {
            QPushButton* pushbutton = dynamic_cast<QPushButton*>(settingWidget);
            if (pushbutton) {
               nodeSettings[settingsId] = QColor(settingValues[settingsId]);
            }
            break;
         }
         case QMetaType::QString: {
            QLineEdit* lineedit = dynamic_cast<QLineEdit*>(settingWidget);
            if (lineedit) {
               nodeSettings[settingsId] = lineedit->text();
            } else if (auto* textedit = dynamic_cast<QPlainTextEdit*>(settingWidget)) {
               nodeSettings[settingsId] = textedit->toPlainText();
            }
            break;
         }
         case QMetaType::QStringList: {
            QComboBox* combobox = dynamic_cast<QComboBox*>(settingWidget);
            if (combobox) {
               nodeSettings[settingsId] = combobox->currentText();
            }
            break;
         }
         default:
            INFO_MSG("Type not found. Property id=" + settingsId);
      }
   }
   auto editManager = widgetmanager.getEditManager();
   if (editManager == nullptr) {
      return false;
   }
   for (auto setting = nodeSettings.cbegin(); setting != nodeSettings.cend(); ++setting) {
      if (oldSettings.value(setting.key()) == setting.value()) {
         continue;
      }
      const bool merge = setting.value().typeId() != QMetaType::QColor;
      editManager->changeSetting(id, setting.key(), setting.value(), merge);
   }
   return true;
}

void NodeSettingsWidget::swapSlots() {
   auto editManager = widgetmanager.getEditManager();
   if (editManager != nullptr) {
      editManager->swapSources(id);
   }
}

void NodeSettingsWidget::colorDialog(const QString& settingsId) {
   QColor initColor = Qt::white;
   if (settingValues.contains(settingsId)) {
      initColor = QColor(settingValues[settingsId]);
   }
   const QColor color =
       QColorDialog::getColor(initColor, this, "Select Color", QColorDialog::ShowAlphaChannel);
   if (color.isValid()) {
      auto* button = dynamic_cast<QPushButton*>(settingElements[settingsId]);
      settingValues[settingsId] = color.name(QColor::HexArgb);
      styleButton(button, color);
      this->saveSettings();
   }
}

void NodeSettingsWidget::styleButton(QPushButton* button, const QColor& color) {
   if (button) {
      QString fontColor("#ffffff");
      // Estimate perceived brightness to choose readable foreground text.
      if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
         fontColor = "#000000";
      }
      button->setStyleSheet(
          QString("background-color: %1; color: %2").arg(color.name(), fontColor));
      button->setText(
          color.name().append(QString("%1").arg(color.alpha(), 2, 16, QLatin1Char('0'))));
   }
}

/// @brief Compares generator settings by display order.
bool settingsComperator(const TextureGeneratorSetting& v1, const TextureGeneratorSetting& v2) {
   return v1.order < v2.order;
}

/// @brief Compares the identifying fields of two generator settings.
bool operator==(const TextureGeneratorSetting& lhs, const TextureGeneratorSetting& rhs) {
   return lhs.name == rhs.name && lhs.order == rhs.order;
}

void NodeSettingsWidget::generatorUpdated() {
   TextureGeneratorPtr generator = texNode->getGenerator();
   generatorNameLabel->setText(generator->getName());

   const QStringList sourceSlots = generator->getSourceSlots();
   if (!sourceSlots.isEmpty()) {
      sourceButtonsWidget->show();
   } else {
      sourceButtonsWidget->hide();
   }
   for (int i = 0; i < sourceSlots.size() && i < sourceSlotLabels.count(); i++) {
      sourceSlotLabels[i]->setText(sourceSlots.at(i) + ":");
   }

   // Remove all previous property rows, if any.
   while (settingsLayout->count() > 0) {
      QLayoutItem* item = settingsLayout->takeAt(0);
      delete item->widget();
      delete item;
   }
   settingLabels.clear();
   settingElements.clear();
   settingSliders.clear();
   settingValues.clear();

   TextureGeneratorSettings settings = generator->getSettings();
   QList<TextureGeneratorSetting> settingsvalues = settings.values();
   std::sort(settingsvalues.begin(), settingsvalues.end(), settingsComperator);
   QListIterator<TextureGeneratorSetting> settingsIterator(settingsvalues);

   while (settingsIterator.hasNext()) {
      TextureGeneratorSetting currSetting = settingsIterator.next();
      QString settingsId = settings.key(currSetting);
      QWidget* newWidget;
      switch (currSetting.defaultvalue.typeId()) {
         case QMetaType::QString:
            if (currSetting.multiline) {
               auto* textedit = new QPlainTextEdit;
               const int textEditHeight = textedit->fontMetrics().lineSpacing() * 4 + 18;
               textedit->setFixedHeight(textEditHeight);
               newWidget = textedit;
               QObject::connect(textedit, &QPlainTextEdit::textChanged, this,
                                &NodeSettingsWidget::saveSettings);
            } else {
               newWidget = new QLineEdit;
               QObject::connect(static_cast<QLineEdit*>(newWidget), &QLineEdit::returnPressed, this,
                                &NodeSettingsWidget::saveSettings);
            }
            break;
         case QMetaType::QStringList:
            newWidget = new QComboBox;
            static_cast<QComboBox*>(newWidget)->addItems(currSetting.defaultvalue.toStringList());
            static_cast<QComboBox*>(newWidget)->setCurrentIndex(currSetting.defaultindex);
            QObject::connect(static_cast<QComboBox*>(newWidget),
                             static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                             [=](int i) {
                                Q_UNUSED(i);
                                this->saveSettings();
                             });
            break;
         case QMetaType::Bool:
            newWidget = new QCheckBox;
            QObject::connect(static_cast<QCheckBox*>(newWidget),
                             static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::toggled),
                             [=](bool i) {
                                Q_UNUSED(i);
                                this->saveSettings();
                             });
            break;
         case QMetaType::Double:
            newWidget = new QDoubleSpinBox;
            static_cast<QDoubleSpinBox*>(newWidget)->setSingleStep(0.1);
            QObject::connect(
                static_cast<QDoubleSpinBox*>(newWidget),
                static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                [=](double i) {
                   Q_UNUSED(i);
                   this->saveSettings();
                });
            break;
         case QMetaType::QColor:
            newWidget = new QPushButton("Color");
            newWidget->setProperty("colorControl", true);
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
                             [=](int i) {
                                Q_UNUSED(i);
                                this->saveSettings();
                             });
      }
      newWidget->setProperty("inspectorEditor", true);
      QWidget* newSlider{nullptr};
      if (!currSetting.max.isNull()) {
         auto* doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(newWidget);
         auto* spinBox = dynamic_cast<QSpinBox*>(newWidget);
         if (doubleSpinBox) {
            auto* doubleSlider = new QDoubleSlider();
            newSlider = doubleSlider;
            doubleSlider->blockSignals(true);
            doubleSpinBox->blockSignals(true);
            doubleSlider->setDoubleMinimum(currSetting.min.toDouble());
            doubleSlider->setDoubleMaximum(currSetting.max.toDouble());
            doubleSpinBox->setMinimum(currSetting.min.toDouble());
            doubleSpinBox->setMaximum(currSetting.max.toDouble());
            QObject::connect(doubleSlider, &QDoubleSlider::doubleValueChanged, doubleSpinBox,
                             &QDoubleSpinBox::setValue);
            QObject::connect(
                doubleSpinBox,
                static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                [=](double value) {
                   doubleSlider->blockSignals(true);
                   doubleSlider->setDoubleValue(value);
                   doubleSlider->blockSignals(false);
                });
            doubleSpinBox->blockSignals(false);
            doubleSlider->blockSignals(false);
         } else if (spinBox) {
            auto* intSlider = new QSlider(Qt::Horizontal);
            newSlider = intSlider;
            spinBox->blockSignals(true);
            intSlider->blockSignals(true);
            intSlider->setMinimum(currSetting.min.toInt());
            intSlider->setMaximum(currSetting.max.toInt());
            spinBox->setMinimum(currSetting.min.toInt());
            spinBox->setMaximum(currSetting.max.toInt());
            QObject::connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                             intSlider, &QSlider::setValue);
            QObject::connect(intSlider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
            spinBox->blockSignals(false);
            intSlider->blockSignals(false);
         }
      }
      auto* newLabel = new QLabel(currSetting.name + ":");
      settingLabels[settingsId] = newLabel;
      settingElements[settingsId] = newWidget;
      if (newSlider != nullptr) {
         newSlider->setProperty("inspectorSlider", true);
         settingSliders[settingsId] = newSlider;
      }
      settingsLayout->addWidget(createPropertyRow(newLabel, newWidget, newSlider));
      QString groupId;
      if (!(currSetting.group.isEmpty()) &&
          ((settingsIterator.hasNext() && settingsIterator.peekNext().group != currSetting.group) ||
           !settingsIterator.hasNext())) {
         auto* groupCheckbox = new QCheckBox;
         groupId = currSetting.group + "groupcheckbox";
         QLabel* newLabel = new QLabel(QString("Align ").append(currSetting.group).append(":"));
         settingLabels[groupId] = newLabel;
         settingElements[groupId] = groupCheckbox;
         settingsLayout->addWidget(createPropertyRow(newLabel, groupCheckbox));
         QObject::connect(groupCheckbox, &QCheckBox::toggled,
                          [=](bool val) { this->setGroupAlignment(currSetting.group, val); });
      }
      if (!currSetting.enabler.isEmpty()) {
         auto* enablerWidget = dynamic_cast<QCheckBox*>(settingElements[currSetting.enabler]);
         if (enablerWidget) {
            if (settingElements[settingsId]) {
               settingElements[settingsId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled, settingElements[settingsId],
                                &QWidget::setEnabled);
            }
            if (settingSliders[settingsId]) {
               settingSliders[settingsId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled, settingSliders[settingsId],
                                &QWidget::setEnabled);
            }
            if (settingElements[groupId]) {
               settingElements[groupId]->setEnabled(enablerWidget->isChecked());
               QObject::connect(enablerWidget, &QCheckBox::toggled, settingElements[groupId],
                                &QWidget::setEnabled);
            }
         }
      }
   }
   this->settingsUpdated();
}

void NodeSettingsWidget::setGroupAlignment(const QString& group, bool aligned) {
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
               if (currSetting.defaultvalue.typeId() == QMetaType::Int) {
                  QObject::connect(static_cast<QSpinBox*>(firstwidget),
                                   static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                                   static_cast<QSpinBox*>(currwidget), &QSpinBox::setValue);
                  QObject::connect(static_cast<QSpinBox*>(firstwidget),
                                   static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                                   [=](int i) {
                                      Q_UNUSED(i);
                                      this->saveSettings();
                                   });
               } else if (currSetting.defaultvalue.typeId() == QMetaType::Double) {
                  QObject::connect(
                      static_cast<QDoubleSpinBox*>(firstwidget),
                      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                      static_cast<QDoubleSpinBox*>(currwidget), &QDoubleSpinBox::setValue);
                  QObject::connect(
                      static_cast<QDoubleSpinBox*>(firstwidget),
                      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                      [=](double i) {
                         Q_UNUSED(i);
                         this->saveSettings();
                      });
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
               if (currSetting.defaultvalue.typeId() == QMetaType::Int) {
                  QObject::connect(static_cast<QSlider*>(firstslider), &QSlider::valueChanged,
                                   static_cast<QSlider*>(currslider), &QSlider::setValue);
               } else if (currSetting.defaultvalue.typeId() == QMetaType::Double) {
                  QObject::connect(
                      static_cast<QDoubleSlider*>(firstslider), &QDoubleSlider::doubleValueChanged,
                      static_cast<QDoubleSlider*>(currslider), &QDoubleSlider::setDoubleValue);
               }
            }
            currslider->setEnabled(!aligned);
            currslider->blockSignals(aligned);
         }
      }
   }
}

void NodeSettingsWidget::slotsUpdated() {
   int numInList = sourceSlotButtons.count();
   const QStringList sourceSlots = texNode->getSourceSlots();
   const int numSlots = sourceSlots.size();
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
         const QString& slot = sourceSlots.at(i);
         currLabel->setText(slot + ":");
         int connectedNode = texNode->getSources().value(slot);
         if (connectedNode != 0) {
            currLabel->setText(currLabel->text().append(" ").append(
                widgetmanager.getTextureProject()->getNode(connectedNode)->getName()));
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

void NodeSettingsWidget::settingsUpdated() {
   nodeNameLineEdit->setText(texNode->getName());

   QMapIterator<QString, QWidget*> settingElementIterator(settingElements);
   while (settingElementIterator.hasNext()) {
      settingElementIterator.next();
      QString settingsId = settingElementIterator.key();
      if (!texNode->getGenerator()->getSettings().contains(settingsId)) {
         continue;
      }
      const TextureGeneratorSetting settingDefinition =
          texNode->getGenerator()->getSettings().value(settingsId);
      const QVariant defaultvalue = settingDefinition.defaultvalue;
      QVariant nodevalue = defaultvalue;
      const TextureNodeSettings nodeSettings = texNode->getSettings();
      if (nodeSettings.contains(settingsId)) {
         nodevalue = nodeSettings.value(settingsId);
      } else if (defaultvalue.typeId() == QMetaType::QStringList) {
         const QStringList values = defaultvalue.toStringList();
         if (settingDefinition.defaultindex >= 0 &&
             settingDefinition.defaultindex < values.size()) {
            nodevalue = values.at(settingDefinition.defaultindex);
         }
      }

      QWidget* settingsWidget = settingElementIterator.value();
      saveDisabled = true;
      int index;
      switch (defaultvalue.typeId()) {
         case QMetaType::Int: {
            QSpinBox* spinbox = dynamic_cast<QSpinBox*>(settingsWidget);
            if (spinbox) {
               spinbox->setValue(nodevalue.toInt());
            }
            break;
         }
         case QMetaType::Double: {
            QDoubleSpinBox* doublespinbox = dynamic_cast<QDoubleSpinBox*>(settingsWidget);
            if (doublespinbox) {
               doublespinbox->setValue(nodevalue.toDouble());
            }
            break;
         }
         case QMetaType::QColor: {
            QPushButton* pushbutton = dynamic_cast<QPushButton*>(settingsWidget);
            if (pushbutton) {
               settingValues[settingsId] = nodevalue.toString();
               styleButton(pushbutton, QColor(nodevalue.toString()));
            }
            break;
         }
         case QMetaType::Bool: {
            QCheckBox* checkbox = dynamic_cast<QCheckBox*>(settingsWidget);
            if (checkbox) {
               checkbox->setChecked(nodevalue.toBool());
            }
            break;
         }
         case QMetaType::QString: {
            QLineEdit* lineedit = dynamic_cast<QLineEdit*>(settingsWidget);
            if (lineedit) {
               lineedit->setText(nodevalue.toString());
            } else if (auto* textedit = dynamic_cast<QPlainTextEdit*>(settingsWidget)) {
               textedit->setPlainText(nodevalue.toString());
            }
            break;
         }
         case QMetaType::QStringList: {
            QComboBox* combobox = dynamic_cast<QComboBox*>(settingsWidget);
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
