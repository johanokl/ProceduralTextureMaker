
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

/// @brief The SettingsManager class
///
/// Manages the global settings. The current settings can be previewed
/// before they are written to persistent storage using QSettings.
class SettingsManager : public QObject {
   Q_OBJECT

public:
   SettingsManager();
   ~SettingsManager() override = default;
   QSize getPreviewSize() const;
   QSize getThumbnailSize() const;
   QString getJSTextureGeneratorsPath() const;
   bool getJSTextureGeneratorsEnabled() const;
   QColor getPreviewBackgroundColor() const;
   QColor getBackgroundColor() const;
   int getBackgroundBrush() const;
   void loadSettings();
   void saveSettings() const;

signals:
   void settingsUpdated();

public slots:
   void setPreviewSize(const QSize&);
   void setThumbnailSize(const QSize&);
   void setPreviewBackgroundColor(const QColor&);
   void setBackgroundColor(const QColor&);
   void setBackgroundBrush(int val);
   void setJSTextureGeneratorsPath(const QString&);
   void setJSTextureGeneratorsEnabled(bool);

private:
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
