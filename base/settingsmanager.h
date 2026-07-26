
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QColor>
#include <QObject>
#include <QSize>
#include <QString>

/// @brief Manages application-wide settings.
/// @details Settings can be previewed before they are written to persistent storage with
/// `QSettings`.
class SettingsManager : public QObject {
   Q_OBJECT

public:
   /// @brief Constructs the settings manager and loads persisted settings.
   SettingsManager();
   ~SettingsManager() override = default;

   /// @brief Gets the image dimensions used for exports.
   /// @return The configured export image dimensions.
   QSize getPreviewSize() const;

   /// @brief Gets the thumbnail size used in the scene widget.
   /// @return The configured thumbnail dimensions.
   QSize getThumbnailSize() const;

   /// @brief Gets the absolute path to the JavaScript texture generator directory.
   /// @return The configured path to the JavaScript texture generator directory.
   QString getJSTextureGeneratorsPath() const;

   /// @brief Checks whether JavaScript texture generators are enabled.
   /// @return @c true if JavaScript generators should be loaded.
   bool getJSTextureGeneratorsEnabled() const;

   /// @brief Gets the background color for the preview view.
   /// @return The configured preview background color.
   QColor getPreviewBackgroundColor() const;

   /// @brief Gets the background color for the node graph view.
   /// @return The configured node graph background color.
   QColor getBackgroundColor() const;

   /// @brief Gets the background brush style for the node graph view.
   /// @return The saved integer value corresponding to `Qt::BrushStyle`.
   int getBackgroundBrush() const;

   /// @brief Reloads persisted settings and emits `settingsUpdated()` if any value changes.
   void loadSettings();

   /// @brief Saves the current settings using `QSettings`.
   void saveSettings() const;

signals:
   void settingsUpdated();

public slots:
   /// @brief Sets the image dimensions used for exports.
   /// @param size The new export image dimensions.
   void setPreviewSize(const QSize& size);

   /// @brief Sets the thumbnail dimensions used in the scene widget.
   /// @param size The new thumbnail dimensions.
   void setThumbnailSize(const QSize& size);

   /// @brief Sets the background color for the preview view.
   /// @param value The new preview background color.
   void setPreviewBackgroundColor(const QColor& value);

   /// @brief Sets the background color for the node graph view.
   /// @param value The new node graph background color.
   void setBackgroundColor(const QColor& value);

   /// @brief Sets the background brush style for the node graph view.
   /// @param val The new integer value corresponding to `Qt::BrushStyle`.
   void setBackgroundBrush(int val);

   /// @brief Sets the absolute path to the JavaScript texture generator directory.
   /// @param path The new path to the JavaScript texture generator directory.
   void setJSTextureGeneratorsPath(const QString& path);

   /// @brief Enables or disables loading JavaScript texture generators.
   /// @param enabled Whether JavaScript generators should be loaded.
   void setJSTextureGeneratorsEnabled(bool enabled);

private:
   /// @brief Reads and applies values from `QSettings`.
   /// @return @c true if at least one value changes.
   bool readSettings();

   QSize previewSize;
   QSize thumbnailSize;
   QString jsTextureGeneratorsPath;
   bool jsTextureGeneratorsEnabled;
   QColor previewBackgroundColor;
   QColor backgroundColor;
   int backgroundBrush;
};

#endif  // SETTINGSMANAGER_H
