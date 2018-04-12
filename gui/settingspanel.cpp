/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "core/settingsmanager.h"
#include "gui/mainwindow.h"
#include "gui/menuactions.h"
#include "settingspanel.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QShowEvent>
#include <QSpinBox>
#include <QVBoxLayout>

/**
 * @brief SettingsPanel::SettingsPanel
 * @param parent
 * @param settingsmanager
 */
SettingsPanel::SettingsPanel(MainWindow *parent, SettingsManager* settingsmanager) : QWidget(parent)
{
   this->settingsmanager = settingsmanager;
   blockSlot = false;
   layout = new QVBoxLayout(this);
   area = new QScrollArea;
   area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   area->setWidgetResizable(true);
   contents = new QWidget;
   contentsLayout = new QVBoxLayout(contents);
   layout->setContentsMargins(0, 0, 0, 0);
   area->setFrameShape(QFrame::NoFrame);
   layout->addWidget(area);
   area->setWidget(contents);
   setMinimumWidth(350);

   QGroupBox* sceneviewWidget = new QGroupBox("Scene View");
   auto* sceneviewLayout = new QGridLayout;
   sceneviewWidget->setLayout(sceneviewLayout);
   sceneviewWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(sceneviewWidget);

   QLabel* thumbnailWidthLabel = new QLabel("Thumbnail width:");
   thumbnailWidthSpinbox = new QSpinBox(this);
   thumbnailWidthSpinbox->setMinimum(50);
   thumbnailWidthSpinbox->setMaximum(2000);
   sceneviewLayout->addWidget(thumbnailWidthLabel, 0, 0);
   sceneviewLayout->addWidget(thumbnailWidthSpinbox, 0, 1);

   QLabel* thumbnailHeightLabel = new QLabel("Thumbnail height:");
   thumbnailHeightSpinbox = new QSpinBox(this);
   thumbnailHeightSpinbox->setMinimum(50);
   thumbnailHeightSpinbox->setMaximum(2000);
   sceneviewLayout->addWidget(thumbnailHeightLabel, 1, 0);
   sceneviewLayout->addWidget(thumbnailHeightSpinbox, 1, 1);

   QLabel* backgroundColorLabel = new QLabel("Background color:");
   backgroundColorButton = new QPushButton("");
   QObject::connect(backgroundColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(backgroundColorButton); });

   sceneviewLayout->addWidget(backgroundColorLabel, 2, 0);
   sceneviewLayout->addWidget(backgroundColorButton, 2, 1);

   QLabel* backgroundBrushLabel = new QLabel("Background brush:");
   backgroundBrushCombobox = new QComboBox;
   backgroundBrushCombobox->addItem("Solid", static_cast<int>(Qt::SolidPattern));
   backgroundBrushCombobox->addItem("Horizontal", static_cast<int>(Qt::HorPattern));
   backgroundBrushCombobox->addItem("Vertical", static_cast<int>(Qt::VerPattern));
   backgroundBrushCombobox->addItem("Cross", static_cast<int>(Qt::CrossPattern));
   backgroundBrushCombobox->addItem("Backward Diagonal", static_cast<int>(Qt::BDiagPattern));
   backgroundBrushCombobox->addItem("Forward Diagonal", static_cast<int>(Qt::FDiagPattern));
   backgroundBrushCombobox->addItem("Diagonal Cross", static_cast<int>(Qt::DiagCrossPattern));
   backgroundBrushCombobox->addItem("Dense 1", static_cast<int>(Qt::Dense1Pattern));
   backgroundBrushCombobox->addItem("Dense 2", static_cast<int>(Qt::Dense2Pattern));
   backgroundBrushCombobox->addItem("Dense 3", static_cast<int>(Qt::Dense3Pattern));
   backgroundBrushCombobox->addItem("Dense 4", static_cast<int>(Qt::Dense4Pattern));
   backgroundBrushCombobox->addItem("Dense 5", static_cast<int>(Qt::Dense5Pattern));
   backgroundBrushCombobox->addItem("Dense 6", static_cast<int>(Qt::Dense6Pattern));
   backgroundBrushCombobox->addItem("Dense 7", static_cast<int>(Qt::Dense7Pattern));
   backgroundBrushCombobox->setCurrentIndex(0);
   sceneviewLayout->addWidget(backgroundBrushLabel, 3, 0);
   sceneviewLayout->addWidget(backgroundBrushCombobox, 3, 1);

   QLabel* defaultZoomLabel = new QLabel("Zoom:");
   defaultZoomSpinbox = new QSpinBox(this);
   defaultZoomSpinbox->setMinimum(5);
   defaultZoomSpinbox->setMaximum(300);
   sceneviewLayout->addWidget(defaultZoomLabel, 4, 0);
   sceneviewLayout->addWidget(defaultZoomSpinbox, 4, 1);

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
   QLabel* previewBackgroundColorLabel = new QLabel("Background color:");
   previewBackgroundColorButton = new QPushButton("");
   QObject::connect(previewBackgroundColorButton,
                    static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked),
                    [=](bool) { this->colorDialog(previewBackgroundColorButton); });
   previewLayout->addWidget(previewBackgroundColorLabel, 2, 0);
   previewLayout->addWidget(previewBackgroundColorButton, 2, 1);

   QGroupBox* saveButtonBox = new QGroupBox("");
   auto* saveButtonLayout = new QGridLayout;
   saveButtonBox->setLayout(saveButtonLayout);
   QPushButton* saveButton = new QPushButton("Save");
   QPushButton* closeButton = new QPushButton("Close");
   saveButtonLayout->addWidget(saveButton, 0, 0);
   saveButtonLayout->addWidget(closeButton, 0, 1);
   QObject::connect(saveButton, &QPushButton::clicked,
                    this, &SettingsPanel::saveSettings);
   QObject::connect(closeButton, &QPushButton::clicked,
                    parent->getMenu(), &MenuActions::toggleSettingsPanel);
   contentsLayout->addWidget(saveButtonBox);

   QWidget *spacerWidget = new QWidget;
   spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   spacerWidget->setVisible(true);
   contentsLayout->addWidget(spacerWidget);
   contentsLayout->setSizeConstraint(QLayout::SetMinimumSize);
   settingsUpdated();
}

/**
 * @brief SettingsPanel::colorDialog
 * @param button
 */
void SettingsPanel::colorDialog(QPushButton* button)
{
   QColor initColor(button->text());
   QColor color = QColorDialog::getColor(initColor, this, "Select Color");
   if (color.isValid()) {
      styleColorButton(button, color);
   }
}

/**
 * @brief SettingsPanel::styleColorButton
 * @param button
 * @param color
 */
void SettingsPanel::styleColorButton(QPushButton* button, const QColor& color)
{
   QString fontColor("#ffffff");
   if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
      fontColor = "#000000";
   }
   button->setStyleSheet(QString("background-color: %1; color: %2").arg(color.name(), fontColor));
   button->setText(color.name());
}

/**
 * @brief SettingsPanel::selectDirectoryPath
 * @param lineWidget
 */
void SettingsPanel::selectDirectoryPath(QLineEdit* lineWidget)
{
   QString startDir = lineWidget->text();
   if (startDir.isEmpty()) {
      startDir = QDir::homePath();
   }
   QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", startDir,
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
   if (!dir.isNull()) {
      lineWidget->setText(dir);
   }
}

/**
 * @brief SettingsPanel::showEvent
 * @param event
 * Reload the settings when the panel is opened.
 */
void SettingsPanel::showEvent(QShowEvent* event)
{
   QWidget::showEvent(event);
   if (!event->spontaneous() && !isHidden()) {
      settingsUpdated();
   }
}

/**
 * @brief SettingsPanel::settingsUpdated
 */
void SettingsPanel::settingsUpdated()
{
   if (!blockSlot) {
      jsGeneratorPathEdit->setText(settingsmanager->getJSTextureGeneratorsPath());
      exportImageWidthSpinbox->setValue(settingsmanager->getPreviewSize().width());
      exportImageHeightSpinbox->setValue(settingsmanager->getPreviewSize().height());
      thumbnailWidthSpinbox->setValue(settingsmanager->getThumbnailSize().width());
      thumbnailHeightSpinbox->setValue(settingsmanager->getThumbnailSize().height());
      defaultZoomSpinbox->setValue(settingsmanager->getDefaultZoom());
      jsGeneratorEnabledCheckbox->setChecked(settingsmanager->getJSTextureGeneratorsEnabled());
      styleColorButton(backgroundColorButton, settingsmanager->getBackgroundColor());
      styleColorButton(previewBackgroundColorButton, settingsmanager->getPreviewBackgroundColor());
      int index = backgroundBrushCombobox->findData(settingsmanager->getBackgroundBrush());
      if (index != -1) {
         backgroundBrushCombobox->setCurrentIndex(index);
      }
   }
}

/**
 * @brief SettingsPanel::saveSettings
 */
void SettingsPanel::saveSettings()
{
   blockSlot = true;
   int exportImageWidth = exportImageWidthSpinbox->value();
   int exportImageHeight = exportImageHeightSpinbox->value();
   int thumbnailWidth = thumbnailWidthSpinbox->value();
   int thumbnailHeight = thumbnailHeightSpinbox->value();
   exportImageWidth = (exportImageWidth % 2) ? (exportImageWidth + 1) : exportImageWidth;
   exportImageHeight = (exportImageHeight % 2) ? (exportImageHeight + 1) : exportImageHeight;
   thumbnailWidth = (thumbnailWidth % 2) ? (thumbnailWidth + 1) : thumbnailWidth;
   thumbnailHeight = (thumbnailHeight % 2) ? (thumbnailHeight + 1) : thumbnailHeight;
   settingsmanager->setPreviewSize(QSize(exportImageWidth, exportImageHeight));
   settingsmanager->setThumbnailSize(QSize(thumbnailWidth, thumbnailHeight));
   settingsmanager->setJSTextureGeneratorsPath(jsGeneratorPathEdit->text());
   settingsmanager->setJSTextureGeneratorsEnabled(jsGeneratorEnabledCheckbox->isChecked());
   settingsmanager->setDefaultZoom(defaultZoomSpinbox->value());
   settingsmanager->setPreviewBackgroundColor(QColor(previewBackgroundColorButton->text()));
   settingsmanager->setBackgroundColor(QColor(backgroundColorButton->text()));
   settingsmanager->setBackgroundBrush(backgroundBrushCombobox->currentData().toInt());
   blockSlot = false;
   settingsUpdated();
}
