/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QSpacerItem>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFont>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QColorDialog>
#include "settingspanel.h"
#include "core/textureproject.h"
#include "sceneview/viewnodescene.h"
#include "core/settingsmanager.h"
#include "gui/menuactions.h"
#include "gui/mainwindow.h"

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
   setMinimumWidth(300);

   QGroupBox* sceneviewWidget = new QGroupBox("Scene View");
   QGridLayout* sceneviewLayout = new QGridLayout;
   sceneviewWidget->setLayout(sceneviewLayout);
   sceneviewWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(sceneviewWidget);

   QLabel* thumbnailWidthLabel = new QLabel("Thumbnail width:");
   thumbnailWidthSlider = new QSpinBox(this);
   thumbnailWidthSlider->setMinimum(50);
   thumbnailWidthSlider->setMaximum(2000);
   sceneviewLayout->addWidget(thumbnailWidthLabel, 0, 0);
   sceneviewLayout->addWidget(thumbnailWidthSlider, 0, 1);

   QLabel* thumbnailHeightLabel = new QLabel("Thumbnail height:");
   thumbnailHeightSlider = new QSpinBox(this);
   thumbnailHeightSlider->setMinimum(50);
   thumbnailHeightSlider->setMaximum(2000);
   sceneviewLayout->addWidget(thumbnailHeightLabel, 1, 0);
   sceneviewLayout->addWidget(thumbnailHeightSlider, 1, 1);

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

   QGroupBox* exportWidget = new QGroupBox("Exporting");
   QGridLayout* exportLayout = new QGridLayout;
   exportWidget->setLayout(exportLayout);
   exportWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(exportWidget);

   QLabel* exportImageWidthLabel = new QLabel("Image width:");
   exportImageWidthSlider = new QSpinBox(this);
   exportImageWidthSlider->setMinimum(50);
   exportImageWidthSlider->setMaximum(2000);
   exportLayout->addWidget(exportImageWidthLabel, 0, 0);
   exportLayout->addWidget(exportImageWidthSlider, 0, 1);

   QLabel* exportImageHeightLabel = new QLabel("Image height:");
   exportImageHeightSlider = new QSpinBox(this);
   exportImageHeightSlider->setMinimum(50);
   exportImageHeightSlider->setMaximum(2000);
   exportLayout->addWidget(exportImageHeightLabel, 1, 0);
   exportLayout->addWidget(exportImageHeightSlider, 1, 1);

   QGroupBox* generatorsWidget = new QGroupBox("Generators");
   QGridLayout* generatorsLayout = new QGridLayout;
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

   QGroupBox* saveButtonBox = new QGroupBox("");
   QGridLayout* saveButtonLayout = new QGridLayout;
   saveButtonBox->setLayout(saveButtonLayout);
   QPushButton* saveButton = new QPushButton("Save");
   QPushButton* closeButton = new QPushButton("Close");
   saveButtonLayout->addWidget(saveButton, 0, 0);
   saveButtonLayout->addWidget(closeButton, 0, 1);
   QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
   QObject::connect(closeButton, SIGNAL(clicked()), parent->getMenu(), SLOT(toggleSettingsPanel()));
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
void SettingsPanel::styleColorButton(QPushButton* button, QColor color)
{
   QString fontColor("#ffffff");
   if ((color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 170) {
      fontColor = "#000000";
   }
   button->setStyleSheet(QString("background-color: %1; color: %2").arg(color.name()).arg(fontColor));
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
 * @brief SettingsPanel::settingsUpdated
 */
void SettingsPanel::settingsUpdated()
{
   if (!blockSlot) {
      jsGeneratorPathEdit->setText(settingsmanager->getJSTextureGeneratorsPath());
      exportImageWidthSlider->setValue(settingsmanager->getPreviewSize().width());
      exportImageHeightSlider->setValue(settingsmanager->getPreviewSize().height());
      thumbnailWidthSlider->setValue(settingsmanager->getThumbnailSize().width());
      thumbnailHeightSlider->setValue(settingsmanager->getThumbnailSize().height());
      jsGeneratorEnabledCheckbox->setChecked(settingsmanager->getJSTextureGeneratorsEnabled());
      styleColorButton(backgroundColorButton, settingsmanager->getBackgroundColor());
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
   settingsmanager->setPreviewSize(QSize(exportImageWidthSlider->value(), exportImageHeightSlider->value()));
   settingsmanager->setJSTextureGeneratorsPath(jsGeneratorPathEdit->text());
   settingsmanager->setJSTextureGeneratorsEnabled(jsGeneratorEnabledCheckbox->isChecked());
   settingsmanager->setThumbnailSize(QSize(thumbnailWidthSlider->value(), thumbnailHeightSlider->value()));
   settingsmanager->setBackgroundColor(QColor(backgroundColorButton->text()));
   settingsmanager->setBackgroundBrush(backgroundBrushCombobox->currentData().toInt());
   blockSlot = false;
   settingsUpdated();
}
