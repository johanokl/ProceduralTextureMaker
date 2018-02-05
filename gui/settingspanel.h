/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

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

/**
 * @brief The SettingsPanel class
 *
 * Vertical panel with all the user controlled settings for a project
 * listed. Connected to a SettingsManager instance which loads and saved
 * to an persistant storage.
 */
class SettingsPanel : public QWidget
{
   Q_OBJECT

public:
   SettingsPanel(MainWindow* parent, SettingsManager* settingsmanager);
   virtual ~SettingsPanel() {}
   virtual void showEvent(QShowEvent*);

public slots:
   void settingsUpdated();
   void saveSettings();
   void selectDirectoryPath(QLineEdit*);
   void colorDialog(QPushButton*);

private:
   void styleColorButton(QPushButton*, QColor);

   SettingsManager* settingsmanager;

   QVBoxLayout* contentsLayout;
   QVBoxLayout* layout;
   QScrollArea* area;
   QWidget* contents;

   QSpinBox* thumbnailWidthSpinbox;
   QSpinBox* thumbnailHeightSpinbox;
   QSpinBox* exportImageWidthSpinbox;
   QSpinBox* exportImageHeightSpinbox;
   QSpinBox* defaultZoomSpinbox;
   QLineEdit* jsGeneratorPathEdit;
   QCheckBox* jsGeneratorEnabledCheckbox;
   QPushButton* backgroundColorButton;
   QComboBox* backgroundBrushCombobox;

   bool blockSlot;
};

#endif // SETTINGSPANEL_H
