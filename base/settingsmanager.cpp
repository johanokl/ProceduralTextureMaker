
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "settingsmanager.h"
#include <QColor>
#include <QDir>
#include <QSettings>
#include <QSize>

/// @brief Constructor for the SettingsManager class.
SettingsManager::SettingsManager() : jsTextureGeneratorsEnabled(false), backgroundBrush(0) {
   readSettings();
}

/// @brief Gets the size of the thumbnails in the scene widget.
/// @return Size of the exported images.
QSize SettingsManager::getPreviewSize() const { return previewSize; }

/// @brief Sets the size of the thumbnails in the scene widget.
/// @param size Size of the exported images.
void SettingsManager::setPreviewSize(const QSize& size) {
   if (size != previewSize) {
      previewSize = size;
      emit settingsUpdated();
   }
}

/// @brief Gets the size of the thumbnails in the scene widget.
/// @return Size of thumbnails in the scene widget. 300x300 pixels if not set.
QSize SettingsManager::getThumbnailSize() const { return thumbnailSize; }

/// @brief Sets the size of the thumbnails in the scene widget.
/// @param size New thumbnail size to use.
void SettingsManager::setThumbnailSize(const QSize& size) {
   if (size != thumbnailSize) {
      thumbnailSize = size;
      emit settingsUpdated();
   }
}

/// @brief Gets the absolute path to the JS texture generators.
/// @return Absolute path to the JS textures.
QString SettingsManager::getJSTextureGeneratorsPath() const { return jsTextureGeneratorsPath; }

/// @brief Sets the absolute path to the JS texture generators.
/// @param path Absolute path to the JS textures.
void SettingsManager::setJSTextureGeneratorsPath(const QString& path) {
   QString nativePath = QDir::toNativeSeparators(path);
   if (nativePath != jsTextureGeneratorsPath) {
      jsTextureGeneratorsPath = nativePath;
      emit settingsUpdated();
   }
}

/// @brief Gets whether JS texture generators are enabled.
/// @return @c true if JS textures should be loaded.
bool SettingsManager::getJSTextureGeneratorsEnabled() const { return jsTextureGeneratorsEnabled; }

/// @brief Sets whether JS texture generators are enabled.
/// @param enabled True to load JS textures.
void SettingsManager::setJSTextureGeneratorsEnabled(bool enabled) {
   if (enabled != jsTextureGeneratorsEnabled) {
      jsTextureGeneratorsEnabled = enabled;
      emit settingsUpdated();
   }
}

/// @brief Gets the background color for the preview view.
/// @return Preview view background color
QColor SettingsManager::getPreviewBackgroundColor() const { return previewBackgroundColor; }

/// @brief Sets the background color for the preview view.
/// @param val New color to use.
void SettingsManager::setPreviewBackgroundColor(const QColor& val) {
   if (val != previewBackgroundColor) {
      previewBackgroundColor = val;
      emit settingsUpdated();
   }
}

/// @brief Gets the background color for the node graph view.
/// @return Node graph view background color
QColor SettingsManager::getBackgroundColor() const { return backgroundColor; }

/// @brief Sets the background color for the node graph view.
/// @param val New color to use.
void SettingsManager::setBackgroundColor(const QColor& val) {
   if (val != backgroundColor) {
      backgroundColor = val;
      emit settingsUpdated();
   }
}

/// @brief Gets the background brush style for the node graph view.
/// @return The saved brush style. For id mapping see Qt::BrushStyle.
int SettingsManager::getBackgroundBrush() const { return backgroundBrush; }

/// @brief Sets the background brush style for the node graph view.
/// @param val The new brush style, for id mapping see Qt::BrushStyle.
void SettingsManager::setBackgroundBrush(int val) {
   if (val != backgroundBrush) {
      backgroundBrush = val;
      emit settingsUpdated();
   }
}

/// @brief Loads the settings and emits the settingsUpdated signal if any setting has changed.
void SettingsManager::loadSettings() {
   if (readSettings()) {
      emit settingsUpdated();
   }
}

/// @brief Saves the current settings to the QSettings object.
void SettingsManager::saveSettings() const {
   QSettings settings;
   settings.setValue("previewsize", previewSize);
   settings.setValue("thumbnailsize", thumbnailSize);
   settings.setValue("jstexturegeneratorspath", jsTextureGeneratorsPath);
   settings.setValue("jstexturegeneratorsenabled", jsTextureGeneratorsEnabled);
   settings.setValue("previewbackgroundcolor", previewBackgroundColor.name());
   settings.setValue("backgroundcolor", backgroundColor.name());
   settings.setValue("backgroundbrush", backgroundBrush);
   settings.sync();
}

/// @brief Reads the settings from the QSettings object.
/// @return @c true if any setting changed.
bool SettingsManager::readSettings() {
   QSettings settings;
   QSize newPreviewSize = settings.value("previewsize", QSize(800, 800)).toSize();
   QSize newThumbnailSize = settings.value("thumbnailsize", QSize(300, 300)).toSize();
   QString newJsTextureGeneratorsPath = QDir::toNativeSeparators(
       settings.value("jstexturegeneratorspath", QDir::homePath() + "/TexGen").toString());
   bool newJsTextureGeneratorsEnabled =
       settings.value("jstexturegeneratorsenabled", false).toBool();
   QColor newPreviewBackgroundColor(settings.value("previewbackgroundcolor", "#c8c8c8").toString());
   QColor newBackgroundColor(settings.value("backgroundcolor", "#c8c8c8").toString());
   int newBackgroundBrush = settings.value("backgroundbrush", 1).toInt();

   bool changed = previewSize != newPreviewSize || thumbnailSize != newThumbnailSize ||
                  jsTextureGeneratorsPath != newJsTextureGeneratorsPath ||
                  jsTextureGeneratorsEnabled != newJsTextureGeneratorsEnabled ||
                  previewBackgroundColor != newPreviewBackgroundColor ||
                  backgroundColor != newBackgroundColor || backgroundBrush != newBackgroundBrush;

   previewSize = newPreviewSize;
   thumbnailSize = newThumbnailSize;
   jsTextureGeneratorsPath = newJsTextureGeneratorsPath;
   jsTextureGeneratorsEnabled = newJsTextureGeneratorsEnabled;
   previewBackgroundColor = newPreviewBackgroundColor;
   backgroundColor = newBackgroundColor;
   backgroundBrush = newBackgroundBrush;
   return changed;
}
