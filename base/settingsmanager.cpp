
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "settingsmanager.h"
#include <QColor>
#include <QDir>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtCore/qtmetamacros.h>

SettingsManager::SettingsManager() : jsTextureGeneratorsEnabled(false), backgroundBrush(0) {
   readSettings();
}

QSize SettingsManager::getPreviewSize() const { return previewSize; }

void SettingsManager::setPreviewSize(const QSize& size) {
   if (size != previewSize) {
      previewSize = size;
      emit settingsUpdated();
   }
}

QSize SettingsManager::getThumbnailSize() const { return thumbnailSize; }

void SettingsManager::setThumbnailSize(const QSize& size) {
   if (size != thumbnailSize) {
      thumbnailSize = size;
      emit settingsUpdated();
   }
}

QString SettingsManager::getJSTextureGeneratorsPath() const { return jsTextureGeneratorsPath; }

void SettingsManager::setJSTextureGeneratorsPath(const QString& path) {
   QString nativePath = QDir::toNativeSeparators(path);
   if (nativePath != jsTextureGeneratorsPath) {
      jsTextureGeneratorsPath = nativePath;
      emit settingsUpdated();
   }
}

bool SettingsManager::getJSTextureGeneratorsEnabled() const { return jsTextureGeneratorsEnabled; }

void SettingsManager::setJSTextureGeneratorsEnabled(bool enabled) {
   if (enabled != jsTextureGeneratorsEnabled) {
      jsTextureGeneratorsEnabled = enabled;
      emit settingsUpdated();
   }
}

QColor SettingsManager::getPreviewBackgroundColor() const { return previewBackgroundColor; }

void SettingsManager::setPreviewBackgroundColor(const QColor& val) {
   if (val != previewBackgroundColor) {
      previewBackgroundColor = val;
      emit settingsUpdated();
   }
}

QColor SettingsManager::getBackgroundColor() const { return backgroundColor; }

void SettingsManager::setBackgroundColor(const QColor& val) {
   if (val != backgroundColor) {
      backgroundColor = val;
      emit settingsUpdated();
   }
}

int SettingsManager::getBackgroundBrush() const { return backgroundBrush; }

void SettingsManager::setBackgroundBrush(int val) {
   if (val != backgroundBrush) {
      backgroundBrush = val;
      emit settingsUpdated();
   }
}

void SettingsManager::loadSettings() {
   if (readSettings()) {
      emit settingsUpdated();
   }
}

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
