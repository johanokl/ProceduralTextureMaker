
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

/// @brief Displays user-controlled project and scene settings in a vertical panel.
/// The panel uses a SettingsManager to load from and save to persistent storage.
class SettingsPanel : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates a settings panel connected to a settings manager.
   /// @param parent Main window whose scene settings are updated.
   /// @param settingsmanager Settings manager edited by the panel.
   SettingsPanel(MainWindow* parent, SettingsManager* settingsmanager);

   /// @brief Destroys the settings panel.
   ~SettingsPanel() override = default;

   /// @brief Reloads settings when the panel is shown.
   /// @param event Show event.
   void showEvent(QShowEvent* event) override;

public slots:
   /// @brief Updates panel controls from the settings manager.
   void settingsUpdated();

   /// @brief Applies and persists the settings currently shown in the panel.
   void saveSettings();

   /// @brief Restores default values in the panel.
   void resetSettings();

   /// @brief Discards unsaved edits and reloads persisted settings.
   void cancelSettings();

   /// @brief Opens a directory dialog and stores the selected path in an editor.
   /// @param lineEdit Editor that receives the selected directory.
   void selectDirectoryPath(QLineEdit* lineEdit);

   /// @brief Opens a color dialog for a settings button.
   /// @param button Button whose represented color is updated.
   void colorDialog(QPushButton* button);

private:
   /// @brief Styles a color button with readable foreground text.
   /// @param button Button to style.
   /// @param color Background color represented by the button.
   void styleColorButton(QPushButton* button, const QColor& color);

   /// @brief Updates labels that display numeric scene-style values.
   void updateSceneStyleLabels();

   /// @brief Applies panel values to the settings manager and main window.
   void applySettings();

   /// @brief Converts a header-size slider value to pixels.
   /// @param value Slider value.
   /// @return Header height in pixels.
   int headerSizeForSlider(int value) const;

   /// @brief Converts a header height in pixels to a slider value.
   /// @param size Header height in pixels.
   /// @return Slider value.
   int headerSliderForSize(int size) const;

   /// @brief Converts a zoom-speed slider value to a zoom factor.
   /// @param value Slider value.
   /// @return Zoom factor.
   double zoomFactorForSlider(int value) const;

   /// @brief Converts a zoom factor to a zoom-speed slider value.
   /// @param factor Zoom factor.
   /// @return Slider value.
   int zoomSliderForFactor(double factor) const;

   /// @brief Settings manager edited by the panel.
   SettingsManager* settingsmanager;
   /// @brief Main window whose scene style is updated.
   MainWindow* mainwindow;
   /// @brief Layout arranging the scroll area's contents.
   QVBoxLayout* contentsLayout;
   /// @brief Main vertical layout for the panel.
   QVBoxLayout* layout;
   /// @brief Scroll area containing all settings controls.
   QScrollArea* area;
   /// @brief Widget containing the scroll area's controls.
   QWidget* contents;
   /// @brief Editor for the thumbnail width.
   QSpinBox* thumbnailWidthSpinbox;
   /// @brief Editor for the thumbnail height.
   QSpinBox* thumbnailHeightSpinbox;
   /// @brief Editor for the exported image width.
   QSpinBox* exportImageWidthSpinbox;
   /// @brief Editor for the exported image height.
   QSpinBox* exportImageHeightSpinbox;
   /// @brief Slider controlling regular connection-line width.
   QSlider* lineWidthSlider;
   /// @brief Slider controlling connection-arrow size.
   QSlider* arrowSizeSlider;
   /// @brief Slider controlling node-header height.
   QSlider* headerSizeSlider;
   /// @brief Slider controlling the scene-view zoom factor.
   QSlider* zoomSpeedSlider;
   /// @brief Label displaying the selected connection-line width.
   QLabel* lineWidthValueLabel;
   /// @brief Label displaying the selected connection-arrow size.
   QLabel* arrowSizeValueLabel;
   /// @brief Label displaying the selected node-header height.
   QLabel* headerSizeValueLabel;
   /// @brief Label displaying the selected zoom factor.
   QLabel* zoomSpeedValueLabel;
   /// @brief Editor for the JavaScript generator directory.
   QLineEdit* jsGeneratorPathEdit;
   /// @brief Checkbox controlling JavaScript generator loading.
   QCheckBox* jsGeneratorEnabledCheckbox;
   /// @brief Button representing the graph background color.
   QPushButton* backgroundColorButton;
   /// @brief Button representing the image-preview background color.
   QPushButton* previewBackgroundColorButton;
   /// @brief Selector for the graph background brush style.
   QComboBox* backgroundBrushCombobox;
   /// @brief Whether control signals are temporarily ignored.
   bool blockSlot;
};

#endif  // SETTINGSPANEL_H
