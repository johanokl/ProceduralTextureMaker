// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef JSTEXGENEDITOR_H
#define JSTEXGENEDITOR_H

#include "base/texturegenerator.h"
#include <QMap>
#include <QPoint>
#include <QTextCursor>
#include <QWidget>
class JsTexGenManager;
class SettingsManager;
class QPlainTextEdit;
class QTextDocument;
class QPushButton;
class QLabel;

/// @brief Edits JavaScript generators, retaining separate drafts and undo histories in memory.
class JsTexGenEditor : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates the editor using the project's existing script loader.
   /// @param manager Loader used to validate and apply saves.
   /// @param settings Application settings controlling the editor font.
   /// @param parent Parent widget.
   explicit JsTexGenEditor(JsTexGenManager& manager, SettingsManager& settings,
                           QWidget* parent = nullptr);

   /// @brief Displays a generator's saved source or its existing draft.
   /// @param generator Selected palette definition, or null to clear the editor.
   void setGenerator(const TextureGeneratorPtr& generator);

   /// @brief Confirms discarding unsaved drafts when the window closes.
   /// @return True when the window may close.
   bool canClose();

   /// @brief Changes a string metadata property in source, preserving literal definitions.
   /// @param source JavaScript to update.
   /// @param property Metadata key, such as name or filename.
   /// @param value New string value.
   /// @return Updated source.
   static QString withProperty(QString source, const QString& property, const QString& value);

public slots:
   /// @brief Applies an information-panel field change to the current in-memory draft.
   void setMetadata(const QString& property, const QString& value);

   /// @brief Validates, writes, and applies the selected custom generator.
   void save();

   /// @brief Reads the external file into the current draft without applying it.
   void reload();

signals:
   /// @brief Keeps information-panel fields in sync with the selected draft.
   void draftMetadataChanged(QString name, QString filename);

   /// @brief Selects the replacement definition after a successful save.
   void generatorSaved(TextureGeneratorPtr generator);

private:
   /// @brief Keeps one draft's text, cursor, and scroll position while switching generators.
   struct Draft {
      /// @brief Text and undo history owned by this editor.
      QTextDocument* document{nullptr};
      /// @brief Last cursor and selection in this document.
      QTextCursor cursor;
      /// @brief Horizontal and vertical scrollbar values.
      QPoint scroll;
   };

   /// @brief Reads a literal string from the draft, falling back while source is incomplete.
   QString draftProperty(const QString& property, const QString& fallback) const;

   /// @brief Updates button availability and the selected generator label.
   void updateControls();

   /// @brief Creates an editable document with JavaScript highlighting.
   QTextDocument* createDocument(const QString& source);

   /// @brief Loader used for validation, writing, and registry updates.
   JsTexGenManager& manager;
   /// @brief Currently selected definition.
   TextureGeneratorPtr generator;
   /// @brief Drafts keyed by source identity, including their undo histories and view positions.
   QMap<QString, Draft> drafts;
   /// @brief Empty document used when no JavaScript generator is selected.
   QTextDocument* emptyDocument{nullptr};
   /// @brief Source editor.
   QPlainTextEdit* code{nullptr};
   /// @brief Undoes the current draft's last edit.
   QPushButton* undoButton{nullptr};
   /// @brief Redoes the current draft's last undone edit.
   QPushButton* redoButton{nullptr};
   /// @brief Validates and applies the current draft.
   QPushButton* saveButton{nullptr};
   /// @brief Reads the external source file.
   QPushButton* reloadButton{nullptr};
   /// @brief Selected generator name and modified marker.
   QLabel* heading{nullptr};
   /// @brief Save status or validation diagnostic.
   QLabel* status{nullptr};
};

#endif  // JSTEXGENEDITOR_H
