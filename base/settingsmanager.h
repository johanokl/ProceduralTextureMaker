
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
   explicit SettingsManager(QObject* parent = nullptr);

   /// @brief Destroys the settings manager.
   ~SettingsManager() override = default;

   /// @brief Gets the image dimensions used for previews and exports.
   /// @return The configured preview and export image dimensions.
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

   /// @brief Gets the color used by the node graph background brush.
   /// @return The configured node graph background brush color.
   QColor getBackgroundBrushColor() const;

   /// @brief Gets the base color shown behind transparent node textures.
   QColor getNodeBackgroundColor() const;

   /// @brief Gets the overlay color shown behind transparent node textures.
   QColor getNodeBackgroundBrushColor() const;

   /// @brief Gets the overlay brush style shown behind transparent node textures.
   int getNodeBackgroundBrush() const;

   /// @brief Gets the background brush style for the node graph view.
   /// @return The saved integer value corresponding to `Qt::BrushStyle`.
   int getBackgroundBrush() const;

   /// @brief Gets the font size used for connection endpoint labels.
   /// @return Endpoint label font size in pixels.
   int getConnectionLabelSize() const;

   /// @brief Checks whether source node names are shown at receiving edges.
   bool getDisplaySourceNames() const;

   /// @brief Checks whether receiver node names are shown at source edges.
   bool getDisplayReceiverNames() const;

   /// @brief Reloads persisted settings and emits `settingsUpdated()` if any value changes.
   void loadSettings();

   /// @brief Saves the current settings using `QSettings`.
   [[nodiscard]] bool saveSettings() const;

signals:
   /// @brief Emitted after one or more application settings change.
   void settingsUpdated();

public slots:
   /// @brief Sets the image dimensions used for previews and exports.
   /// @param size The new preview and export image dimensions.
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

   /// @brief Sets the color used by the node graph background brush.
   /// @param value The new node graph background brush color.
   void setBackgroundBrushColor(const QColor& value);

   /// @brief Sets the base color shown behind transparent node textures.
   void setNodeBackgroundColor(const QColor& value);

   /// @brief Sets the overlay color shown behind transparent node textures.
   void setNodeBackgroundBrushColor(const QColor& value);

   /// @brief Sets the overlay brush style shown behind transparent node textures.
   void setNodeBackgroundBrush(int value);

   /// @brief Sets the background brush style for the node graph view.
   /// @param val The new integer value corresponding to `Qt::BrushStyle`.
   void setBackgroundBrush(int val);

   /// @brief Sets the absolute path to the JavaScript texture generator directory.
   /// @param path The new path to the JavaScript texture generator directory.
   void setJSTextureGeneratorsPath(const QString& path);

   /// @brief Enables or disables loading JavaScript texture generators.
   /// @param enabled Whether JavaScript generators should be loaded.
   void setJSTextureGeneratorsEnabled(bool enabled);

   /// @brief Sets the font size used for connection endpoint labels.
   void setConnectionLabelSize(int size);

   /// @brief Sets whether source node names are shown at receiving edges.
   void setDisplaySourceNames(bool enabled);

   /// @brief Sets whether receiver node names are shown at source edges.
   void setDisplayReceiverNames(bool enabled);

private:
   /// @brief Reads and applies values from `QSettings`.
   /// @return @c true if at least one value changes.
   bool readSettings();

   /// @brief Width and height of preview and export images.
   QSize previewSize;
   /// @brief Width and height of node thumbnail images.
   QSize thumbnailSize;
   /// @brief Directory containing JavaScript texture generators.
   QString jsTextureGeneratorsPath;
   /// @brief Whether JavaScript texture generators are enabled.
   bool jsTextureGeneratorsEnabled;
   /// @brief Background color used by the preview view.
   QColor previewBackgroundColor;
   /// @brief Background color used by the node graph view.
   QColor backgroundColor;
   /// @brief Color used by the node graph background brush.
   QColor backgroundBrushColor;
   /// @brief Base color shown behind transparent node textures.
   QColor nodeBackgroundColor;
   /// @brief Overlay color shown behind transparent node textures.
   QColor nodeBackgroundBrushColor;
   /// @brief Integer value of the transparent node texture overlay brush style.
   int nodeBackgroundBrush;
   /// @brief Integer value of the node graph's Qt brush style.
   int backgroundBrush;
   /// @brief Font size used for connection endpoint labels.
   int connectionLabelSize;
   /// @brief Whether receiving-edge labels include source node names.
   bool displaySourceNames;
   /// @brief Whether source-edge labels include receiver node names.
   bool displayReceiverNames;
};

#endif  // SETTINGSMANAGER_H
