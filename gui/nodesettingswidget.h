
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef NODESETTINGSWIDGET_H
#define NODESETTINGSWIDGET_H

#include "base/texturenode.h"
#include <QList>
#include <QMap>
#include <QWidget>
class ItemInfoPanel;
class QLabel;
class QPushButton;
class QSpacerItem;
class QGroupBox;
class QLineEdit;
class QFormLayout;
class QSlider;
class QGridLayout;
class QDoubleSlider;
class QScrollArea;
class QVBoxLayout;

/// @brief Vertical panel for changing the values sent to the node's generator.
/// Has widgets for displaying and setting integer, double, color and
/// string/enum values.
class NodeSettingsWidget : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates property editors for a texture node.
   /// @param widgetmanager Information panel that owns this widget.
   /// @param id Identifier of the node to edit.
   NodeSettingsWidget(ItemInfoPanel* widgetmanager, int id);

   /// @brief Destroys the node settings widget.
   ~NodeSettingsWidget() override = default;

public slots:
   /// @brief Updates editor values after the node's settings change.
   void settingsUpdated();

   /// @brief Updates source labels and controls after node connections change.
   void slotsUpdated();

   /// @brief Recreates property editors after the node's generator changes.
   void generatorUpdated();

   /// @brief Writes editor values to the texture node.
   /// @return True when saving was enabled and the values were applied.
   bool saveSettings();

   /// @brief Opens a color dialog and saves the selected setting value.
   /// @param settingsId Identifier of the color setting to edit.
   void colorDialog(const QString& settingsId);

   /// @brief Rotates connected sources among the node's input slots.
   void swapSlots();

private:
   /// @brief Creates and styles a form layout for a settings group.
   /// @return Newly allocated form layout.
   QFormLayout* createGroupLayout();

   /// @brief Creates a property editor row for the inspector.
   /// @param label Property label.
   /// @param editor Property value editor.
   /// @param slider Optional slider displayed below the label and numeric value.
   /// @return Newly allocated property row widget.
   QWidget* createPropertyRow(QLabel* label, QWidget* editor, QWidget* slider = nullptr);

   /// @brief Styles a color button with readable foreground text.
   /// @param button Button to style.
   /// @param color Background color represented by the button.
   void styleButton(QPushButton* button, const QColor& color);

   /// @brief Sets the alignment of a generated settings group.
   /// @param group Group identifier.
   /// @param aligned Whether the group's editors should use aligned rows.
   void setGroupAlignment(const QString& group, bool aligned);

   /// @brief Information panel that owns and coordinates this widget.
   ItemInfoPanel* widgetmanager;
   /// @brief Identifier of the node being edited.
   int id;
   /// @brief Texture node whose properties are edited.
   TextureNodePtr texNode;
   /// @brief Whether editor changes are temporarily prevented from being saved.
   bool saveDisabled;
   /// @brief Labels indexed by generator setting identifier.
   QMap<QString, QLabel*> settingLabels;
   /// @brief Property editors indexed by generator setting identifier.
   QMap<QString, QWidget*> settingElements;
   /// @brief Optional sliders indexed by generator setting identifier.
   QMap<QString, QWidget*> settingSliders;
   /// @brief String representations of values requiring auxiliary storage.
   QMap<QString, QString> settingValues;
   /// @brief Scroll area containing node information and property editors.
   QScrollArea* scrollarea;
   /// @brief Widget containing the scroll area's controls.
   QWidget* contents;
   /// @brief Layout arranging the scroll area's sections.
   QVBoxLayout* contentsLayout;
   /// @brief Layout arranging generated property editors.
   QVBoxLayout* settingsLayout;
   /// @brief Group box containing generated property editors.
   QGroupBox* settingsWidget;
   /// @brief Label displaying the active generator name.
   QLabel* generatorNameLabel;
   /// @brief Group box containing basic node information.
   QGroupBox* nodeInfoWidget;
   /// @brief Form layout arranging basic node information.
   QFormLayout* nodeInfoLayout;
   /// @brief Editor for the node's display name.
   QLineEdit* nodeNameLineEdit;
   /// @brief Group box containing source-slot controls.
   QGroupBox* sourceButtonsWidget;
   /// @brief Grid layout arranging source-slot controls.
   QGridLayout* sourceButtonsLayout;
   /// @brief Labels describing each source slot.
   QList<QLabel*> sourceSlotLabels;
   /// @brief Button that rotates connected sources among slots.
   QPushButton* swapSlotButton;
   /// @brief Buttons used to clear individual source slots.
   QList<QPushButton*> sourceSlotButtons;
   /// @brief Main vertical layout for the widget.
   QVBoxLayout* layout;
};

#endif  // NODESETTINGSWIDGET_H
