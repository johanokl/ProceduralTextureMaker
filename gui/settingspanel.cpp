
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/settingsmanager.h"
#include "base/backgroundbrushstyles.h"
#include "gui/mainwindow.h"
#include "gui/menuactions.h"
#include "settingspanel.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QShowEvent>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

void populateBackgroundBrushStyles(QComboBox* combobox) {
   for (const BackgroundBrushStyles::Option& option : BackgroundBrushStyles::options) {
      combobox->addItem(option.label, static_cast<int>(option.style));
   }
}

}  // namespace

SettingsPanel::SettingsPanel(MainWindow* parent, SettingsManager* settingsmanager)
    : QWidget(parent), settingsmanager(settingsmanager), mainwindow(parent) {
   setObjectName(QStringLiteral("settingsPanel"));
   auto* layout = new QVBoxLayout(this);
   auto* area = new QScrollArea;
   area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   area->setWidgetResizable(true);
   auto* contents = new QWidget;
   auto* contentsLayout = new QVBoxLayout(contents);
   layout->setContentsMargins(0, 0, 0, 0);
   area->setFrameShape(QFrame::NoFrame);
   layout->addWidget(area);
   area->setWidget(contents);
   setMinimumWidth(350);
   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

   QGroupBox* sceneWidget = new QGroupBox("Scene");
   auto* sceneLayout = new QGridLayout;
   sceneWidget->setLayout(sceneLayout);
   sceneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(sceneWidget);

   QLabel* backgroundColorLabel = new QLabel("Background color:");
   backgroundColorButton = new QPushButton("");
   QObject::connect(backgroundColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(backgroundColorButton); });

   sceneLayout->addWidget(backgroundColorLabel, 0, 0);
   sceneLayout->addWidget(backgroundColorButton, 0, 1);

   QLabel* backgroundBrushColorLabel = new QLabel("Background overlay color:");
   backgroundBrushColorButton = new QPushButton("");
   QObject::connect(backgroundBrushColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(backgroundBrushColorButton); });
   sceneLayout->addWidget(backgroundBrushColorLabel, 1, 0);
   sceneLayout->addWidget(backgroundBrushColorButton, 1, 1);

   QLabel* backgroundBrushLabel = new QLabel("Background overlay style:");
   backgroundBrushCombobox = new QComboBox;
   populateBackgroundBrushStyles(backgroundBrushCombobox);
   backgroundBrushCombobox->setCurrentIndex(0);
   sceneLayout->addWidget(backgroundBrushLabel, 2, 0);
   sceneLayout->addWidget(backgroundBrushCombobox, 2, 1);

   QLabel* zoomSpeedLabel = new QLabel("Zoom speed:");
   zoomSpeedSlider = new QSlider(Qt::Horizontal, this);
   zoomSpeedSlider->setMinimum(0);
   zoomSpeedSlider->setMaximum(4);
   zoomSpeedSlider->setSingleStep(1);
   zoomSpeedValueLabel = new QLabel(this);
   sceneLayout->addWidget(zoomSpeedLabel, 3, 0);
   sceneLayout->addWidget(zoomSpeedSlider, 3, 1);
   sceneLayout->addWidget(zoomSpeedValueLabel, 3, 2);

   QGroupBox* nodeWidget = new QGroupBox("Nodes");
   auto* nodeLayout = new QGridLayout;
   nodeWidget->setLayout(nodeLayout);
   nodeWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(nodeWidget);

   QLabel* thumbnailWidthLabel = new QLabel("Node width (px):");
   thumbnailWidthSpinbox = new QSpinBox(this);
   thumbnailWidthSpinbox->setMinimum(50);
   thumbnailWidthSpinbox->setMaximum(2000);
   nodeLayout->addWidget(thumbnailWidthLabel, 0, 0);
   nodeLayout->addWidget(thumbnailWidthSpinbox, 0, 1);

   QLabel* thumbnailHeightLabel = new QLabel("Node height (px):");
   thumbnailHeightSpinbox = new QSpinBox(this);
   thumbnailHeightSpinbox->setMinimum(50);
   thumbnailHeightSpinbox->setMaximum(2000);
   nodeLayout->addWidget(thumbnailHeightLabel, 1, 0);
   nodeLayout->addWidget(thumbnailHeightSpinbox, 1, 1);

   QLabel* headerSizeLabel = new QLabel("Header size:");
   headerSizeSlider = new QSlider(Qt::Horizontal, this);
   headerSizeSlider->setMinimum(0);
   headerSizeSlider->setMaximum(6);
   headerSizeSlider->setSingleStep(1);
   headerSizeValueLabel = new QLabel(this);
   nodeLayout->addWidget(headerSizeLabel, 2, 0);
   nodeLayout->addWidget(headerSizeSlider, 2, 1);
   nodeLayout->addWidget(headerSizeValueLabel, 2, 2);

   QLabel* nodeBackgroundColorLabel = new QLabel("Background color:");
   nodeBackgroundColorButton = new QPushButton("");
   QObject::connect(nodeBackgroundColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(nodeBackgroundColorButton); });
   nodeLayout->addWidget(nodeBackgroundColorLabel, 3, 0);
   nodeLayout->addWidget(nodeBackgroundColorButton, 3, 1);

   QLabel* nodeBackgroundBrushColorLabel = new QLabel("Background overlay color:");
   nodeBackgroundBrushColorButton = new QPushButton("");
   QObject::connect(nodeBackgroundBrushColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(nodeBackgroundBrushColorButton); });
   nodeLayout->addWidget(nodeBackgroundBrushColorLabel, 4, 0);
   nodeLayout->addWidget(nodeBackgroundBrushColorButton, 4, 1);

   QLabel* nodeBackgroundBrushLabel = new QLabel("Background overlay style:");
   nodeBackgroundBrushCombobox = new QComboBox;
   populateBackgroundBrushStyles(nodeBackgroundBrushCombobox);
   nodeLayout->addWidget(nodeBackgroundBrushLabel, 5, 0);
   nodeLayout->addWidget(nodeBackgroundBrushCombobox, 5, 1);

   QGroupBox* connectionsWidget = new QGroupBox("Connections");
   auto* connectionsLayout = new QGridLayout;
   connectionsWidget->setLayout(connectionsLayout);
   connectionsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(connectionsWidget);

   QLabel* lineWidthLabel = new QLabel("Line width:");
   lineWidthSlider = new QSlider(Qt::Horizontal, this);
   lineWidthSlider->setMinimum(2);
   lineWidthSlider->setMaximum(6);
   lineWidthSlider->setSingleStep(1);
   lineWidthValueLabel = new QLabel(this);
   connectionsLayout->addWidget(lineWidthLabel, 0, 0);
   connectionsLayout->addWidget(lineWidthSlider, 0, 1);
   connectionsLayout->addWidget(lineWidthValueLabel, 0, 2);

   QLabel* arrowSizeLabel = new QLabel("Arrow size:");
   arrowSizeSlider = new QSlider(Qt::Horizontal, this);
   arrowSizeSlider->setMinimum(4);
   arrowSizeSlider->setMaximum(8);
   arrowSizeSlider->setSingleStep(1);
   arrowSizeValueLabel = new QLabel(this);
   connectionsLayout->addWidget(arrowSizeLabel, 1, 0);
   connectionsLayout->addWidget(arrowSizeSlider, 1, 1);
   connectionsLayout->addWidget(arrowSizeValueLabel, 1, 2);

   QLabel* connectionLabelSizeLabel = new QLabel("Node name size:");
   connectionLabelSizeSlider = new QSlider(Qt::Horizontal, this);
   connectionLabelSizeSlider->setMinimum(8);
   connectionLabelSizeSlider->setMaximum(24);
   connectionLabelSizeSlider->setSingleStep(1);
   connectionLabelSizeValueLabel = new QLabel(this);
   connectionsLayout->addWidget(connectionLabelSizeLabel, 2, 0);
   connectionsLayout->addWidget(connectionLabelSizeSlider, 2, 1);
   connectionsLayout->addWidget(connectionLabelSizeValueLabel, 2, 2);

   displaySourceNamesCheckbox = new QCheckBox("Display source names", this);
   displayReceiverNamesCheckbox = new QCheckBox("Display receiver names", this);
   connectionsLayout->addWidget(displaySourceNamesCheckbox, 3, 0, 1, 3);
   connectionsLayout->addWidget(displayReceiverNamesCheckbox, 4, 0, 1, 3);

   QGroupBox* exportWidget = new QGroupBox("Exporting");
   auto* exportLayout = new QGridLayout;
   exportWidget->setLayout(exportLayout);
   exportWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(exportWidget);

   QLabel* exportImageWidthLabel = new QLabel("Image width:");
   exportImageWidthSpinbox = new QSpinBox(this);
   exportImageWidthSpinbox->setMinimum(50);
   exportImageWidthSpinbox->setMaximum(2000);
   exportLayout->addWidget(exportImageWidthLabel, 0, 0);
   exportLayout->addWidget(exportImageWidthSpinbox, 0, 1);

   QLabel* exportImageHeightLabel = new QLabel("Image height:");
   exportImageHeightSpinbox = new QSpinBox(this);
   exportImageHeightSpinbox->setMinimum(50);
   exportImageHeightSpinbox->setMaximum(2000);
   exportLayout->addWidget(exportImageHeightLabel, 1, 0);
   exportLayout->addWidget(exportImageHeightSpinbox, 1, 1);

   QGroupBox* generatorsWidget = new QGroupBox("JavaScript Generators");
   auto* generatorsLayout = new QGridLayout;
   generatorsWidget->setLayout(generatorsLayout);
   generatorsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(generatorsWidget);

   QLabel* generatorPathLabel = new QLabel("Path:");
   jsGeneratorPathEdit = new QLineEdit(this);
   jsGeneratorPathEdit->setReadOnly(true);
   QPushButton* browseGeneratorPathButton = new QPushButton("Browse");
   QObject::connect(browseGeneratorPathButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->selectDirectoryPath(jsGeneratorPathEdit); });

   QLabel* generatorEnabledLabel = new QLabel("Enabled:");
   jsGeneratorEnabledCheckbox = new QCheckBox(this);
   generatorsLayout->addWidget(generatorPathLabel, 0, 0);
   generatorsLayout->addWidget(jsGeneratorPathEdit, 0, 1);
   generatorsLayout->addWidget(browseGeneratorPathButton, 1, 1);
   generatorsLayout->addWidget(generatorEnabledLabel, 2, 0);
   generatorsLayout->addWidget(jsGeneratorEnabledCheckbox, 2, 1);

   QGroupBox* previewWidget = new QGroupBox("Preview");
   auto* previewLayout = new QGridLayout;
   previewWidget->setLayout(previewLayout);
   previewWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(previewWidget);
   QLabel* previewBackgroundColorLabel = new QLabel("3D view background:");
   previewBackgroundColorButton = new QPushButton("");
   QObject::connect(previewBackgroundColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(previewBackgroundColorButton); });
   previewLayout->addWidget(previewBackgroundColorLabel, 0, 0);
   previewLayout->addWidget(previewBackgroundColorButton, 0, 1);

   QGroupBox* saveButtonBox = new QGroupBox("");
   auto* saveButtonLayout = new QGridLayout;
   saveButtonBox->setLayout(saveButtonLayout);
   QPushButton* saveButton = new QPushButton("Save");
   QPushButton* resetButton = new QPushButton("Reset");
   QPushButton* cancelButton = new QPushButton("Cancel");
   saveButtonLayout->addWidget(saveButton, 0, 0);
   saveButtonLayout->addWidget(resetButton, 0, 1);
   saveButtonLayout->addWidget(cancelButton, 0, 2);
   QObject::connect(saveButton, &QPushButton::clicked, this, &SettingsPanel::saveSettings);
   QObject::connect(resetButton, &QPushButton::clicked, this, &SettingsPanel::resetSettings);
   QObject::connect(cancelButton, &QPushButton::clicked, this, &SettingsPanel::cancelSettings);
   contentsLayout->addWidget(saveButtonBox);

   void (QSpinBox::*spinboxChanged)(int) = &QSpinBox::valueChanged;
   void (QSlider::*sliderChanged)(int) = &QSlider::valueChanged;
   void (QComboBox::*comboboxChanged)(int) = &QComboBox::currentIndexChanged;
   QObject::connect(thumbnailWidthSpinbox, spinboxChanged, this, &SettingsPanel::applySettings);
   QObject::connect(thumbnailHeightSpinbox, spinboxChanged, this, &SettingsPanel::applySettings);
   QObject::connect(exportImageWidthSpinbox, spinboxChanged, this, &SettingsPanel::applySettings);
   QObject::connect(exportImageHeightSpinbox, spinboxChanged, this, &SettingsPanel::applySettings);
   QObject::connect(lineWidthSlider, sliderChanged, this, &SettingsPanel::applySettings);
   QObject::connect(arrowSizeSlider, sliderChanged, this, &SettingsPanel::applySettings);
   QObject::connect(connectionLabelSizeSlider, sliderChanged, this, &SettingsPanel::applySettings);
   QObject::connect(headerSizeSlider, sliderChanged, this, &SettingsPanel::applySettings);
   QObject::connect(zoomSpeedSlider, sliderChanged, this, &SettingsPanel::applySettings);
   QObject::connect(backgroundBrushCombobox, comboboxChanged, this, &SettingsPanel::applySettings);
   QObject::connect(nodeBackgroundBrushCombobox, comboboxChanged, this,
                    &SettingsPanel::applySettings);
   QObject::connect(jsGeneratorEnabledCheckbox, &QCheckBox::toggled, this,
                    &SettingsPanel::applySettings);
   QObject::connect(displaySourceNamesCheckbox, &QCheckBox::toggled, this,
                    &SettingsPanel::applySettings);
   QObject::connect(displayReceiverNamesCheckbox, &QCheckBox::toggled, this,
                    &SettingsPanel::applySettings);

   QWidget* spacerWidget = new QWidget;
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   spacerWidget->setVisible(true);
   contentsLayout->addWidget(spacerWidget);
   contentsLayout->setSizeConstraint(QLayout::SetMinimumSize);
   settingsUpdated();
}

void SettingsPanel::colorDialog(QPushButton* button) {
   QColor initColor(button->text());
   QColor color = QColorDialog::getColor(initColor, this, "Select Color");
   if (color.isValid()) {
      styleColorButton(button, color);
      applySettings();
   }
}

void SettingsPanel::styleColorButton(QPushButton* button, const QColor& color) {
   QString fontColor("#ffffff");
   if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
      fontColor = "#000000";
   }
   button->setStyleSheet(QString("background-color: %1; color: %2").arg(color.name(), fontColor));
   button->setText(color.name());
}

void SettingsPanel::selectDirectoryPath(QLineEdit* lineWidget) {
   QString startDir = lineWidget->text();
   if (startDir.isEmpty()) {
      startDir = QDir::homePath();
   }
   QString dir = QFileDialog::getExistingDirectory(
       this, "Select Directory", startDir,
       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
   if (!dir.isNull()) {
      lineWidget->setText(dir);
      applySettings();
   }
}

void SettingsPanel::updateSceneStyleLabels() {
   int lineWidth = lineWidthSlider->value();
   int arrowSize = arrowSizeSlider->value() * 2;
   int connectionLabelSize = connectionLabelSizeSlider->value();
   int headerSize = headerSizeForSlider(headerSizeSlider->value());
   double zoomFactor = zoomFactorForSlider(zoomSpeedSlider->value());

   lineWidthValueLabel->setText(QString("%1/%2 px").arg(lineWidth).arg(lineWidth + 1));
   arrowSizeValueLabel->setText(QString("%1 px").arg(arrowSize));
   connectionLabelSizeValueLabel->setText(QString("%1 px").arg(connectionLabelSize));
   if (headerSize == 0) {
      headerSizeValueLabel->setText("None");
   } else {
      headerSizeValueLabel->setText(QString("%1 px").arg(headerSize));
   }
   zoomSpeedValueLabel->setText(QString::number(zoomFactor, 'f', 2) + "x");
}

void SettingsPanel::applySettings() {
   updateSceneStyleLabels();
   if (blockSlot) {
      return;
   }

   settingsmanager->setPreviewSize(
       QSize(exportImageWidthSpinbox->value(), exportImageHeightSpinbox->value()));
   settingsmanager->setThumbnailSize(
       QSize(thumbnailWidthSpinbox->value(), thumbnailHeightSpinbox->value()));
   settingsmanager->setPreviewBackgroundColor(QColor(previewBackgroundColorButton->text()));
   settingsmanager->setBackgroundColor(QColor(backgroundColorButton->text()));
   settingsmanager->setBackgroundBrushColor(QColor(backgroundBrushColorButton->text()));
   settingsmanager->setBackgroundBrush(backgroundBrushCombobox->currentData().toInt());
   settingsmanager->setNodeBackgroundColor(QColor(nodeBackgroundColorButton->text()));
   settingsmanager->setNodeBackgroundBrushColor(QColor(nodeBackgroundBrushColorButton->text()));
   settingsmanager->setNodeBackgroundBrush(nodeBackgroundBrushCombobox->currentData().toInt());
   settingsmanager->setJSTextureGeneratorsPath(jsGeneratorPathEdit->text());
   settingsmanager->setJSTextureGeneratorsEnabled(jsGeneratorEnabledCheckbox->isChecked());
   settingsmanager->setConnectionLabelSize(connectionLabelSizeSlider->value());
   settingsmanager->setDisplaySourceNames(displaySourceNamesCheckbox->isChecked());
   settingsmanager->setDisplayReceiverNames(displayReceiverNamesCheckbox->isChecked());

   int lineWidth = lineWidthSlider->value();
   int arrowSize = arrowSizeSlider->value() * 2;
   int headerSize = headerSizeForSlider(headerSizeSlider->value());
   double zoomFactor = zoomFactorForSlider(zoomSpeedSlider->value());

   mainwindow->setLineWidths(lineWidth, lineWidth + 1);
   mainwindow->setArrowSize(arrowSize);
   mainwindow->setHeaderSize(headerSize);
   mainwindow->setZoomStepFactor(zoomFactor);
}

int SettingsPanel::headerSizeForSlider(int value) const {
   switch (value) {
      case 0:
         return 0;
      case 1:
         return 16;
      case 2:
         return 20;
      case 4:
         return 28;
      case 5:
         return 32;
      case 6:
         return 40;
      case 3:
      default:
         return 24;
   }
}

int SettingsPanel::headerSliderForSize(int size) const {
   switch (size) {
      case 0:
         return 0;
      case 16:
         return 1;
      case 20:
         return 2;
      case 28:
         return 4;
      case 32:
         return 5;
      case 40:
         return 6;
      case 24:
      default:
         return 3;
   }
}

double SettingsPanel::zoomFactorForSlider(int value) const {
   switch (value) {
      case 0:
         return 1.25;
      case 2:
         return 1.65;
      case 3:
         return 1.85;
      case 4:
         return 2.05;
      case 1:
      default:
         return 1.45;
   }
}

int SettingsPanel::zoomSliderForFactor(double factor) const {
   if (factor < 1.35) {
      return 0;
   }
   if (factor < 1.55) {
      return 1;
   }
   if (factor < 1.75) {
      return 2;
   }
   if (factor < 1.95) {
      return 3;
   }
   return 4;
}

void SettingsPanel::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   if (!event->spontaneous() && !isHidden()) {
      settingsUpdated();
   }
}

void SettingsPanel::settingsUpdated() {
   if (blockSlot) {
      return;
   }

   blockSlot = true;
   QSettings settings;
   int lineWidth = settings.value("lineWidth", 3).toInt();
   if (lineWidth < 2 || lineWidth > 6) {
      lineWidth = 3;
   }
   int arrowSize = settings.value("arrowSize", 12).toInt();
   if (arrowSize < 8 || arrowSize > 16) {
      arrowSize = 12;
   }
   int headerSize = settings.value("headerSize", 24).toInt();
   if (headerSize != 0 && (headerSize < 8 || headerSize > 48)) {
      headerSize = 24;
   }
   double zoomFactor = settings.value("zoomStepFactor", 1.45).toDouble();
   if (zoomFactor <= 1.0 || zoomFactor > 3.0) {
      zoomFactor = 1.45;
   }

   jsGeneratorPathEdit->setText(settingsmanager->getJSTextureGeneratorsPath());
   exportImageWidthSpinbox->setValue(settingsmanager->getPreviewSize().width());
   exportImageHeightSpinbox->setValue(settingsmanager->getPreviewSize().height());
   thumbnailWidthSpinbox->setValue(settingsmanager->getThumbnailSize().width());
   thumbnailHeightSpinbox->setValue(settingsmanager->getThumbnailSize().height());
   lineWidthSlider->setValue(lineWidth);
   arrowSizeSlider->setValue(arrowSize / 2);
   connectionLabelSizeSlider->setValue(settingsmanager->getConnectionLabelSize());
   displaySourceNamesCheckbox->setChecked(settingsmanager->getDisplaySourceNames());
   displayReceiverNamesCheckbox->setChecked(settingsmanager->getDisplayReceiverNames());
   headerSizeSlider->setValue(headerSliderForSize(headerSize));
   zoomSpeedSlider->setValue(zoomSliderForFactor(zoomFactor));
   jsGeneratorEnabledCheckbox->setChecked(settingsmanager->getJSTextureGeneratorsEnabled());
   styleColorButton(backgroundColorButton, settingsmanager->getBackgroundColor());
   styleColorButton(backgroundBrushColorButton, settingsmanager->getBackgroundBrushColor());
   styleColorButton(nodeBackgroundColorButton, settingsmanager->getNodeBackgroundColor());
   styleColorButton(nodeBackgroundBrushColorButton, settingsmanager->getNodeBackgroundBrushColor());
   styleColorButton(previewBackgroundColorButton, settingsmanager->getPreviewBackgroundColor());
   int index = backgroundBrushCombobox->findData(settingsmanager->getBackgroundBrush());
   if (index != -1) {
      backgroundBrushCombobox->setCurrentIndex(index);
   }
   index = nodeBackgroundBrushCombobox->findData(settingsmanager->getNodeBackgroundBrush());
   if (index != -1) {
      nodeBackgroundBrushCombobox->setCurrentIndex(index);
   }
   blockSlot = false;
   applySettings();
}

void SettingsPanel::resetSettings() {
   blockSlot = true;
   exportImageWidthSpinbox->setValue(800);
   exportImageHeightSpinbox->setValue(800);
   thumbnailWidthSpinbox->setValue(300);
   thumbnailHeightSpinbox->setValue(300);
   lineWidthSlider->setValue(3);
   arrowSizeSlider->setValue(6);
   connectionLabelSizeSlider->setValue(12);
   displaySourceNamesCheckbox->setChecked(false);
   displayReceiverNamesCheckbox->setChecked(false);
   headerSizeSlider->setValue(3);
   zoomSpeedSlider->setValue(1);
   jsGeneratorPathEdit->setText(QDir::toNativeSeparators(QDir::homePath() + "/TexGen"));
   jsGeneratorEnabledCheckbox->setChecked(false);
   styleColorButton(backgroundColorButton, QColor("#c8c8c8"));
   styleColorButton(backgroundBrushColorButton, QColor("#000000"));
   styleColorButton(nodeBackgroundColorButton, QColor("#ffffff"));
   styleColorButton(nodeBackgroundBrushColorButton, QColor("#dedede"));
   styleColorButton(previewBackgroundColorButton, QColor("#c8c8c8"));
   int index = backgroundBrushCombobox->findData(static_cast<int>(Qt::NoBrush));
   if (index != -1) {
      backgroundBrushCombobox->setCurrentIndex(index);
   }
   index = nodeBackgroundBrushCombobox->findData(static_cast<int>(Qt::CrossPattern));
   if (index != -1) {
      nodeBackgroundBrushCombobox->setCurrentIndex(index);
   }
   blockSlot = false;
   applySettings();
}

void SettingsPanel::cancelSettings() {
   settingsmanager->loadSettings();
   settingsUpdated();
   mainwindow->getMenu()->toggleSettingsPanel();
}

void SettingsPanel::saveSettings() {
   blockSlot = true;
   int exportImageWidth = exportImageWidthSpinbox->value();
   int exportImageHeight = exportImageHeightSpinbox->value();
   int thumbnailWidth = thumbnailWidthSpinbox->value();
   int thumbnailHeight = thumbnailHeightSpinbox->value();
   int lineWidth = lineWidthSlider->value();
   int arrowSize = arrowSizeSlider->value() * 2;
   int headerSize = headerSizeForSlider(headerSizeSlider->value());
   double zoomFactor = zoomFactorForSlider(zoomSpeedSlider->value());
   exportImageWidth = (exportImageWidth % 2) ? (exportImageWidth + 1) : exportImageWidth;
   exportImageHeight = (exportImageHeight % 2) ? (exportImageHeight + 1) : exportImageHeight;
   thumbnailWidth = (thumbnailWidth % 2) ? (thumbnailWidth + 1) : thumbnailWidth;
   thumbnailHeight = (thumbnailHeight % 2) ? (thumbnailHeight + 1) : thumbnailHeight;
   settingsmanager->setPreviewSize(QSize(exportImageWidth, exportImageHeight));
   settingsmanager->setThumbnailSize(QSize(thumbnailWidth, thumbnailHeight));
   settingsmanager->setJSTextureGeneratorsPath(jsGeneratorPathEdit->text());
   settingsmanager->setJSTextureGeneratorsEnabled(jsGeneratorEnabledCheckbox->isChecked());
   settingsmanager->setConnectionLabelSize(connectionLabelSizeSlider->value());
   settingsmanager->setDisplaySourceNames(displaySourceNamesCheckbox->isChecked());
   settingsmanager->setDisplayReceiverNames(displayReceiverNamesCheckbox->isChecked());
   settingsmanager->setPreviewBackgroundColor(QColor(previewBackgroundColorButton->text()));
   settingsmanager->setBackgroundColor(QColor(backgroundColorButton->text()));
   settingsmanager->setBackgroundBrushColor(QColor(backgroundBrushColorButton->text()));
   settingsmanager->setBackgroundBrush(backgroundBrushCombobox->currentData().toInt());
   settingsmanager->setNodeBackgroundColor(QColor(nodeBackgroundColorButton->text()));
   settingsmanager->setNodeBackgroundBrushColor(QColor(nodeBackgroundBrushColorButton->text()));
   settingsmanager->setNodeBackgroundBrush(nodeBackgroundBrushCombobox->currentData().toInt());
   if (!settingsmanager->saveSettings()) {
      QMessageBox::warning(this, QStringLiteral("Settings error"),
                           QStringLiteral("Could not save the application settings."));
   }
   QSettings settings;
   settings.setValue("lineWidth", lineWidth);
   settings.setValue("arrowSize", arrowSize);
   settings.setValue("headerSize", headerSize);
   settings.setValue("zoomStepFactor", zoomFactor);
   mainwindow->setLineWidths(lineWidth, lineWidth + 1);
   mainwindow->setArrowSize(arrowSize);
   mainwindow->setHeaderSize(headerSize);
   mainwindow->setZoomStepFactor(zoomFactor);
   blockSlot = false;
   settingsUpdated();
}
