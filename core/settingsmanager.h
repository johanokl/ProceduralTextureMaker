/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSize>
#include <QColor>

/**
 * @brief The SettingsManager class
 *
 * Manages the global settings. Settings are written to
 * and loaded from persistent storage using the QSettings class.
 */
class SettingsManager : public QObject
{
   Q_OBJECT

public:
   SettingsManager() {}
   virtual ~SettingsManager() {}
   QSize getPreviewSize();
   QSize getThumbnailSize();
   QString getJSTextureGeneratorsPath();
   bool getJSTextureGeneratorsEnabled();
   QColor getPreviewBackgroundColor();
   QColor getBackgroundColor();
   int getBackgroundBrush();
   int getDefaultZoom();

signals:
   void settingsUpdated();

public slots:
   void setDefaultZoom(int);
   void setPreviewSize(QSize);
   void setThumbnailSize(QSize);
   void setPreviewBackgroundColor(QColor);
   void setBackgroundColor(QColor);
   void setBackgroundBrush(int val);
   void setJSTextureGeneratorsPath(QString);
   void setJSTextureGeneratorsEnabled(bool);
};

#endif // SETTINGSMANAGER_H
