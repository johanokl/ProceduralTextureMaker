/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QSettings>
#include <QSize>
#include <QDir>
#include <QColor>
#include "settingsmanager.h"

/**
 * @brief SettingsManager::getPreviewSize
 * @return Size of the exported images.
 */
QSize SettingsManager::getPreviewSize()
{
   QSettings settings;
   return settings.value("previewsize", QSize(500, 500)).toSize();
}

/**
 * @brief SettingsManager::setPreviewSize
 * @param size Size of the exported images.
 */
void SettingsManager::setPreviewSize(QSize size)
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
 * @return Size of thumbnails in the scene widget.
 */
QSize SettingsManager::getThumbnailSize()
{
   QSettings settings;
   return settings.value("thumbnailsize", QSize(500, 500)).toSize();
}

/**
 * @brief SettingsManager::setThumbnailSize
 * @param val New thumbnail size to be set and saved.
 */
void SettingsManager::setThumbnailSize(QSize size)
{
   if (size != getThumbnailSize()) {
      QSettings settings;
      settings.setValue("thumbnailsize", size);
      settings.sync();
      emit settingsUpdated();
   }
}

/**
 * @brief SettingsManager::getJSTextureGeneratorsPath
 * @return Absolute path to the JS textures.
 */
QString SettingsManager::getJSTextureGeneratorsPath()
{
   QSettings settings;
   QString path = settings.value("jstexturegeneratorspath", QDir::homePath() + "/TexGen").toString();
   return QDir::toNativeSeparators(path);
}

/**
 * @brief SettingsManager::setJSTextureGeneratorsPath
 * @param path Absolute path to the JS textures.
 */
void SettingsManager::setJSTextureGeneratorsPath(QString path)
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
bool SettingsManager::getJSTextureGeneratorsEnabled()
{
   QSettings settings;
   return settings.value("jstexturegeneratorsenabled", false).toBool();
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
QColor SettingsManager::getBackgroundColor()
{
   QSettings settings;
   return QColor(settings.value("backgroundcolor", "#c8c8c8").toString());
}

/**
 * @brief SettingsManager::setBackgroundColor
 * @param val New color to be saved.
 * Sets the background color for the node graph view.
 */
void SettingsManager::setBackgroundColor(QColor val)
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
int SettingsManager::getBackgroundBrush()
{
   QSettings settings;
   return settings.value("backgroundbrush", 1).toInt();
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
