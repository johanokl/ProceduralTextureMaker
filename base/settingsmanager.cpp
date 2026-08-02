
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

namespace {

bool isValidImageSize(const QSize value) {
   constexpr qint64 maximumPixels = 64LL * 1024 * 1024;
   const qint64 pixels = static_cast<qint64>(value.width()) * value.height();
   return value.width() > 0 && value.height() > 0 && pixels <= maximumPixels;
}

QSize validSizeOrDefault(const QSize value, const QSize fallback) {
   return isValidImageSize(value) ? value : fallback;
}

QColor validColorOrDefault(const QColor& value, const QColor& fallback) {
   return value.isValid() ? value : fallback;
}

int validBrushOrDefault(const int value, const int fallback) {
   return value >= static_cast<int>(Qt::NoBrush) && value <= static_cast<int>(Qt::TexturePattern)
              ? value
              : fallback;
}

}  // namespace

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
      jsTextureGeneratorsEnabled(false),
      backgroundBrush(0),
      connectionLabelSize(12),
      displaySourceNames(false),
      displayReceiverNames(false) {
   readSettings();
}

QSize SettingsManager::getPreviewSize() const { return previewSize; }

void SettingsManager::setPreviewSize(const QSize& size) {
   if (!isValidImageSize(size)) {
      return;
   }
   if (size != previewSize) {
      previewSize = size;
      emit settingsUpdated();
   }
}

QSize SettingsManager::getThumbnailSize() const { return thumbnailSize; }

void SettingsManager::setThumbnailSize(const QSize& size) {
   if (!isValidImageSize(size)) {
      return;
   }
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
   if (!val.isValid()) {
      return;
   }
   if (val != previewBackgroundColor) {
      previewBackgroundColor = val;
      emit settingsUpdated();
   }
}

QColor SettingsManager::getBackgroundColor() const { return backgroundColor; }

void SettingsManager::setBackgroundColor(const QColor& val) {
   if (!val.isValid()) {
      return;
   }
   if (val != backgroundColor) {
      backgroundColor = val;
      emit settingsUpdated();
   }
}

int SettingsManager::getBackgroundBrush() const { return backgroundBrush; }

void SettingsManager::setBackgroundBrush(int val) {
   if (val < static_cast<int>(Qt::NoBrush) || val > static_cast<int>(Qt::TexturePattern)) {
      return;
   }
   if (val != backgroundBrush) {
      backgroundBrush = val;
      emit settingsUpdated();
   }
}

int SettingsManager::getConnectionLabelSize() const { return connectionLabelSize; }

void SettingsManager::setConnectionLabelSize(int size) {
   if (size < 8 || size > 24) {
      return;
   }
   if (size != connectionLabelSize) {
      connectionLabelSize = size;
      emit settingsUpdated();
   }
}

bool SettingsManager::getDisplaySourceNames() const { return displaySourceNames; }

void SettingsManager::setDisplaySourceNames(bool enabled) {
   if (enabled != displaySourceNames) {
      displaySourceNames = enabled;
      emit settingsUpdated();
   }
}

bool SettingsManager::getDisplayReceiverNames() const { return displayReceiverNames; }

void SettingsManager::setDisplayReceiverNames(bool enabled) {
   if (enabled != displayReceiverNames) {
      displayReceiverNames = enabled;
      emit settingsUpdated();
   }
}

void SettingsManager::loadSettings() {
   if (readSettings()) {
      emit settingsUpdated();
   }
}

bool SettingsManager::saveSettings() const {
   QSettings settings;
   settings.setValue("previewsize", previewSize);
   settings.setValue("thumbnailsize", thumbnailSize);
   settings.setValue("jstexturegeneratorspath", jsTextureGeneratorsPath);
   settings.setValue("jstexturegeneratorsenabled", jsTextureGeneratorsEnabled);
   settings.setValue("previewbackgroundcolor", previewBackgroundColor.name());
   settings.setValue("backgroundcolor", backgroundColor.name());
   settings.setValue("backgroundbrush", backgroundBrush);
   settings.setValue("connectionlabelsize", connectionLabelSize);
   settings.setValue("displaysourcenames", displaySourceNames);
   settings.setValue("displayreceivernames", displayReceiverNames);
   settings.sync();
   return settings.status() == QSettings::NoError;
}

bool SettingsManager::readSettings() {
   QSettings settings;
   const QSize defaultPreviewSize(800, 800);
   const QSize defaultThumbnailSize(300, 300);
   QSize newPreviewSize = validSizeOrDefault(
       settings.value("previewsize", defaultPreviewSize).toSize(), defaultPreviewSize);
   QSize newThumbnailSize = validSizeOrDefault(
       settings.value("thumbnailsize", defaultThumbnailSize).toSize(), defaultThumbnailSize);
   QString newJsTextureGeneratorsPath = QDir::toNativeSeparators(
       settings.value("jstexturegeneratorspath", QDir::homePath() + "/TexGen").toString());
   bool newJsTextureGeneratorsEnabled =
       settings.value("jstexturegeneratorsenabled", false).toBool();
   const QColor defaultBackgroundColor(QStringLiteral("#c8c8c8"));
   QColor newPreviewBackgroundColor =
       validColorOrDefault(QColor(settings.value("previewbackgroundcolor", "#c8c8c8").toString()),
                           defaultBackgroundColor);
   QColor newBackgroundColor = validColorOrDefault(
       QColor(settings.value("backgroundcolor", "#c8c8c8").toString()), defaultBackgroundColor);
   int newBackgroundBrush = validBrushOrDefault(settings.value("backgroundbrush", 1).toInt(), 1);
   int newConnectionLabelSize = settings.value("connectionlabelsize", 12).toInt();
   if (newConnectionLabelSize < 8 || newConnectionLabelSize > 24) {
      newConnectionLabelSize = 12;
   }
   bool newDisplaySourceNames = settings.value("displaysourcenames", true).toBool();
   bool newDisplayReceiverNames = settings.value("displayreceivernames", false).toBool();

   bool changed = previewSize != newPreviewSize || thumbnailSize != newThumbnailSize ||
                  jsTextureGeneratorsPath != newJsTextureGeneratorsPath ||
                  jsTextureGeneratorsEnabled != newJsTextureGeneratorsEnabled ||
                  previewBackgroundColor != newPreviewBackgroundColor ||
                  backgroundColor != newBackgroundColor || backgroundBrush != newBackgroundBrush ||
                  connectionLabelSize != newConnectionLabelSize ||
                  displaySourceNames != newDisplaySourceNames ||
                  displayReceiverNames != newDisplayReceiverNames;

   previewSize = newPreviewSize;
   thumbnailSize = newThumbnailSize;
   jsTextureGeneratorsPath = newJsTextureGeneratorsPath;
   jsTextureGeneratorsEnabled = newJsTextureGeneratorsEnabled;
   previewBackgroundColor = newPreviewBackgroundColor;
   backgroundColor = newBackgroundColor;
   backgroundBrush = newBackgroundBrush;
   connectionLabelSize = newConnectionLabelSize;
   displaySourceNames = newDisplaySourceNames;
   displayReceiverNames = newDisplayReceiverNames;
   return changed;
}
