
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>
class QVBoxLayout;
class QScrollArea;
class QSpinBox;
class SettingsManager;
class QLineEdit;
class QCheckBox;
class QPushButton;
class MainWindow;
class QComboBox;
class QLabel;
class QSlider;

/// @brief The SettingsPanel class
///
/// Vertical panel with all the user controlled settings for a project
/// listed. Connected to a SettingsManager instance which loads and saved
/// to an persistant storage.
class SettingsPanel : public QWidget {
   Q_OBJECT

public:
   SettingsPanel(MainWindow* parent, SettingsManager* settingsmanager);
   ~SettingsPanel() override = default;
   void showEvent(QShowEvent*) override;

public slots:
   void settingsUpdated();
   void saveSettings();
   void resetSettings();
   void cancelSettings();
   void selectDirectoryPath(QLineEdit*);
   void colorDialog(QPushButton*);

private:
   void styleColorButton(QPushButton*, const QColor&);
   void updateSceneStyleLabels();
   void applySettings();
   int headerSizeForSlider(int value) const;
   int headerSliderForSize(int size) const;
   double zoomFactorForSlider(int value) const;
   int zoomSliderForFactor(double factor) const;

   SettingsManager* settingsmanager;
   MainWindow* mainwindow;

   QVBoxLayout* contentsLayout;
   QVBoxLayout* layout;
   QScrollArea* area;
   QWidget* contents;

   QSpinBox* thumbnailWidthSpinbox;
   QSpinBox* thumbnailHeightSpinbox;
   QSpinBox* exportImageWidthSpinbox;
   QSpinBox* exportImageHeightSpinbox;
   QSlider* lineWidthSlider;
   QSlider* arrowSizeSlider;
   QSlider* headerSizeSlider;
   QSlider* zoomSpeedSlider;
   QLabel* lineWidthValueLabel;
   QLabel* arrowSizeValueLabel;
   QLabel* headerSizeValueLabel;
   QLabel* zoomSpeedValueLabel;
   QLineEdit* jsGeneratorPathEdit;
   QCheckBox* jsGeneratorEnabledCheckbox;
   QPushButton* backgroundColorButton;
   QPushButton* previewBackgroundColorButton;
   QComboBox* backgroundBrushCombobox;

   bool blockSlot;
};

#endif  // SETTINGSPANEL_H
