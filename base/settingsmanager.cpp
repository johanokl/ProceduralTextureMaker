/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "settingsmanager.h"
#include <QColor>
#include <QDir>
#include <QSettings>
#include <QSize>

/**
 * @brief SettingsManager::getPreviewSize
 * @return Size of the exported images.
 */
QSize SettingsManager::getPreviewSize() const
{
   return QSettings().value("previewsize", QSize(800, 800)).toSize();
}

/**
 * @brief SettingsManager::setPreviewSize
 * @param size Size of the exported images.
 */
void SettingsManager::setPreviewSize(const QSize& size)
{
   if (size != getPreviewSize()) {
      QSettings settings;
      settings.setValue("previewsize", size);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getThumbnailSize
 * @return Size of thumbnails in the scene widget. 300x300 pixels if not set.
 */
QSize SettingsManager::getThumbnailSize() const
{
   return QSettings().value("thumbnailsize", QSize(300, 300)).toSize();
}

/**
 * @brief SettingsManager::setThumbnailSize
 * @param val New thumbnail size to be set and saved.
 */
void SettingsManager::setThumbnailSize(const QSize& size)
{
   if (size != getThumbnailSize()) {
      QSettings settings;
      settings.setValue("thumbnailsize", size);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getDefaultZoom
 * @return Default zoom factor for the scene view. 50 if not set.
 */
int SettingsManager::getDefaultZoom() const
{
   return QSettings().value("defaultzoom", 50).toInt();
}

/**
 * @brief SettingsManager::setDefaultZoom
 * @param zoomfactor Default zoom factor in percentage.
 */
void SettingsManager::setDefaultZoom(int zoomfactor)
{
   if (zoomfactor != getDefaultZoom()) {
      QSettings settings;
      settings.setValue("defaultzoom", zoomfactor);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getJSTextureGeneratorsPath
 * @return Absolute path to the JS textures.
 */
QString SettingsManager::getJSTextureGeneratorsPath() const
{
   QString path = QSettings().value("jstexturegeneratorspath",
                                    QDir::homePath() + "/TexGen").toString();
   return QDir::toNativeSeparators(path);
}

/**
 * @brief SettingsManager::setJSTextureGeneratorsPath
 * @param path Absolute path to the JS textures.
 */
void SettingsManager::setJSTextureGeneratorsPath(const QString& path)
{
   if (path != getJSTextureGeneratorsPath()) {
      QSettings settings;
      settings.setValue("jstexturegeneratorspath", path);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getJSTextureGeneratorsEnabled
 * @return True if JS textures should be loaded.
 */
bool SettingsManager::getJSTextureGeneratorsEnabled() const
{
   return QSettings().value("jstexturegeneratorsenabled", false).toBool();
}

/**
 * @brief SettingsManager::setJSTextureGeneratorsEnabled
 * @param enabled True to load JS textures.
 */
void SettingsManager::setJSTextureGeneratorsEnabled(bool enabled)
{
   if (enabled != getJSTextureGeneratorsEnabled()) {
      QSettings settings;
      settings.setValue("jstexturegeneratorsenabled", enabled);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getBackgroundColor
 * @return Node graph view background color
 */
QColor SettingsManager::getPreviewBackgroundColor() const
{
   return {QSettings().value("previewbackgroundcolor", "#c8c8c8").toString()};
}

/**
 * @brief SettingsManager::setBackgroundColor
 * @param val New color to be saved.
 * Sets the background color for the node graph view.
 */
void SettingsManager::setPreviewBackgroundColor(const QColor& val)
{
   if (val != getPreviewBackgroundColor()) {
      QSettings settings;
      settings.setValue("previewbackgroundcolor", val.name());
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getBackgroundColor
 * @return Node graph view background color
 */
QColor SettingsManager::getBackgroundColor() const
{
   return {QSettings().value("backgroundcolor", "#c8c8c8").toString()};
}

/**
 * @brief SettingsManager::setBackgroundColor
 * @param val New color to be saved.
 * Sets the background color for the node graph view.
 */
void SettingsManager::setBackgroundColor(const QColor& val)
{
   if (val != getBackgroundColor()) {
      QSettings settings;
      settings.setValue("backgroundcolor", val.name());
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getBackgroundBrush
 * @return The saved brush style. For id mapping see Qt::BrushStyle.
 */
int SettingsManager::getBackgroundBrush() const
{
   return QSettings().value("backgroundbrush", 1).toInt();
}

/**
 * @brief SettingsManager::setBackgroundBrush
 * @param val The new brush style, for id mapping see Qt::BrushStyle.
 */
void SettingsManager::setBackgroundBrush(int val)
{
   if (val != getBackgroundBrush()) {
      QSettings settings;
      settings.setValue("backgroundbrush", val);
      settings.sync();
      emit settingsUpdated();
   }
}
