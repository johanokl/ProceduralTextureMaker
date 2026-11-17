// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/jstexgeneditor.h"
#include <QColor>
#include "base/jstexgenmanager.h"
#include "base/settingsmanager.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextCursor>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextDocumentLayout>
#include <QPlainTextEdit>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextDocument>
#include <QVBoxLayout>
#include <utility>

namespace {

/// @brief Adds a current-line highlight and a small line-number gutter to the left.
class SourceEdit : public QPlainTextEdit {
public:
   /// @brief Uses Qt's editing and scrolling, with only the gutter painted here.
   explicit SourceEdit(QWidget* parent) : QPlainTextEdit(parent), numbers(new QWidget(this)) {
      numbers->setObjectName("jsLineNumbers");
      numbers->installEventFilter(this);
      connect(this, &QPlainTextEdit::updateRequest, numbers, [this] { numbers->update(); });
      connect(this, &QPlainTextEdit::blockCountChanged, this, [this] { updateGutter(); });
      connect(this, &QPlainTextEdit::cursorPositionChanged, this,
              &SourceEdit::highlightCurrentLine);
      highlightCurrentLine();
   }

protected:
   /// @brief Refreshes the gutter and highlight when the font or editor availability changes.
   void changeEvent(QEvent* event) override {
      QPlainTextEdit::changeEvent(event);
      if (event->type() == QEvent::FontChange) updateGutter();
      if (event->type() == QEvent::EnabledChange) highlightCurrentLine();
   }

   /// @brief Keeps the gutter beside the viewport when the editor changes size.
   void resizeEvent(QResizeEvent* event) override {
      QPlainTextEdit::resizeEvent(event);
      updateGutter();
   }

   /// @brief Draws only visible line numbers, using the editor's actual block positions.
   bool eventFilter(QObject* watched, QEvent* event) override {
      if (watched != numbers || event->type() != QEvent::Paint)
         return QPlainTextEdit::eventFilter(watched, event);
      QPainter painter(numbers);
      painter.fillRect(numbers->rect(), palette().alternateBase());
      if (!isEnabled()) return true;
      painter.setFont(font());
      for (auto block = firstVisibleBlock(); block.isValid(); block = block.next()) {
         const int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
         if (top >= numbers->height()) break;
         const bool current = block.blockNumber() == textCursor().blockNumber();
         painter.setPen(palette().color(current ? QPalette::Highlight : QPalette::PlaceholderText));
         painter.drawText(0, top, numbers->width() - 6, fontMetrics().height(), Qt::AlignRight,
                          QString::number(block.blockNumber() + 1));
      }
      return true;
   }

private:
   /// @brief Reserves enough space for the largest line number.
   void updateGutter() {
      const int width = 12 + fontMetrics().horizontalAdvance(QString::number(blockCount()));
      if (viewportMargins().left() != width) setViewportMargins(width, 0, 0, 0);
      const QRect area = viewport()->geometry();
      numbers->setGeometry(area.left() - width, area.top(), width, area.height());
      numbers->update();
   }

   /// @brief Marks the cursor's full line without changing the document or undo history.
   void highlightCurrentLine() {
      if (!isEnabled()) {
         setExtraSelections({});
         numbers->update();
         return;
      }
      QTextEdit::ExtraSelection line;
      QColor background = palette().color(QPalette::Highlight);
      background.setAlpha(30);
      line.format.setBackground(background);
      line.format.setProperty(QTextFormat::FullWidthSelection, true);
      line.cursor = textCursor();
      line.cursor.clearSelection();
      setExtraSelections({line});
      numbers->update();
   }

   /// @brief Gutter painted beside the text, independent of horizontal scrolling.
   QWidget* numbers;
};

/// @brief Finds the effective literal name or filename used by a normal descriptor.
QRegularExpressionMatch metadataMatch(const QString& source, const QString& property) {
   // Skip comments, strings, templates, and regular-expression literals before counting braces.
   static const QRegularExpression tokens(
       R"(//[^\n]*|/\*[\s\S]*?\*/|`(?:\\.|[^`\\])*`|/(?:\\.|[^/\n\\])+/[a-z]*|)"
       R"(("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')|[A-Za-z_$][\w$]*|[{}:,.=;\[\]])");
   QList<QRegularExpressionMatch> parts;
   auto matches = tokens.globalMatch(source);
   while (matches.hasNext()) {
      auto token = matches.next();
      if (!token.captured().startsWith('/') && !token.captured().startsWith('`'))
         parts.append(token);
   }
   QRegularExpressionMatch result;
   int depth = 0;
   int descriptorDepth = -1;
   for (int i = 0; i < parts.size(); ++i) {
      const auto text = parts[i].captured();
      const auto at = [&](int offset) {
         return i + offset < parts.size() ? parts[i + offset].captured() : QString();
      };
      if (depth == 0 && text == "generator" && at(1) == "=" && at(2) == "{") descriptorDepth = 1;
      if (descriptorDepth == depth &&
          (text == property || text == '"' + property + '"' || text == '\'' + property + '\'') &&
          at(1) == ":" && (at(3) == "," || at(3) == "}") && i + 2 < parts.size() &&
          !parts[i + 2].captured(1).isEmpty())
         result = parts[i + 2];
      if (depth == 0 && text == "generator" && at(1) == "." && at(2) == property && at(3) == "=" &&
          (at(5) == ";" || at(5).isEmpty()) && i + 4 < parts.size() &&
          !parts[i + 4].captured(1).isEmpty())
         result = parts[i + 4];
      if (text == "{") ++depth;
      if (text == "}" && --depth == 0) descriptorDepth = -1;
   }
   return result;
}

/// @brief Highlights common JavaScript tokens and multiline comments.
class JavaScriptHighlighter : public QSyntaxHighlighter {
public:
   /// @brief Attaches highlighting to one retained draft.
   explicit JavaScriptHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {}

protected:
   /// @brief Colours tokens and carries block-comment state to the next line.
   void highlightBlock(const QString& text) override {
      static const QRegularExpression tokens(
          R"((?<comment>//[^\n]*))"
          R"(|(?<string>"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|`[^`]*`))"
          R"(|(?<api>\b(?:apiVersion|name|description|type|inputs|settings|id|default|values|min|max|group|enabler|order|generate|filename)\b(?=\s*[:(])))"
          R"(|(?<bridge>\b(?:generator|TexGen|size|settings|output|inputs)\b))"
          R"(|(?<keyword>\b(?:const|let|var|function|return|if|else|for|while|do|break|continue|new|class|throw|try|catch|finally|true|false|null|undefined|this|typeof|instanceof|switch|case|default|of|in)\b))"
          R"(|(?<number>\b\d+(?:\.\d+)?\b))");
      auto matches = tokens.globalMatch(text);
      while (matches.hasNext()) {
         const auto match = matches.next();
         const QColor color = !match.captured("comment").isEmpty()  ? QColor("#748875")
                              : !match.captured("string").isEmpty() ? QColor("#ba7940")
                              : !match.captured("api").isEmpty()    ? QColor("#4086b0")
                              : !match.captured("bridge").isEmpty() ? QColor("#2e9a91")
                              : !match.captured("number").isEmpty() ? QColor("#408fa0")
                                                                    : QColor("#9b66c4");
         setFormat(match.capturedStart(), match.capturedLength(), color);
      }
      setCurrentBlockState(0);
      int start = previousBlockState() == 1 ? 0 : text.indexOf("/*");
      while (start >= 0) {
         const int end =
             text.indexOf("*/", start + (previousBlockState() == 1 && start == 0 ? 0 : 2));
         setFormat(start, end < 0 ? text.size() - start : end + 2 - start, QColor("#748875"));
         if (end < 0) {
            setCurrentBlockState(1);
            break;
         }
         start = text.indexOf("/*", end + 2);
      }
   }
};
}  // namespace

JsTexGenEditor::JsTexGenEditor(JsTexGenManager& manager, SettingsManager& settings, QWidget* parent)
    : QWidget(parent), manager(manager) {
   setObjectName("jsTexGenEditor");
   setAttribute(Qt::WA_StyledBackground);
   auto* layout = new QVBoxLayout(this);
   auto* controls = new QHBoxLayout;
   heading = new QLabel(this);
   heading->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
   controls->addWidget(heading, 1);
   undoButton = new QPushButton(tr("Undo"), this);
   redoButton = new QPushButton(tr("Redo"), this);
   reloadButton = new QPushButton(tr("Reload"), this);
   saveButton = new QPushButton(tr("Save"), this);
   saveButton->setObjectName("saveGeneratorButton");
   reloadButton->setToolTip(tr("Read the external file into the editor. Save to apply changes."));
   for (auto* button : {undoButton, redoButton, reloadButton, saveButton})
      controls->addWidget(button);
   layout->addLayout(controls);
   code = new SourceEdit(this);
   code->setObjectName("generatorSourceEditor");
   code->setLineWrapMode(QPlainTextEdit::NoWrap);
   code->setTabStopDistance(code->fontMetrics().horizontalAdvance(' ') * 4);
   layout->addWidget(code, 1);
   status = new QLabel(this);
   status->setTextFormat(Qt::PlainText);
   status->setWordWrap(true);
   status->setMaximumHeight(status->fontMetrics().lineSpacing() * 3);
   layout->addWidget(status);
   emptyDocument = createDocument({});
   code->setDocument(emptyDocument);
   const auto updateFont = [this, &settings] {
      QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
      font.setPointSize(settings.getCodeEditorFontSize());
      if (font == code->font()) return;
      // Keep the source font size independent of the theme's general widget font.
      code->setStyleSheet(QString("QPlainTextEdit { font-size: %1pt; }").arg(font.pointSize()));
      code->setFont(font);
      code->setTabStopDistance(code->fontMetrics().horizontalAdvance(' ') * 4);
      emptyDocument->setDefaultFont(font);
      for (const auto& draft : std::as_const(drafts)) draft.document->setDefaultFont(font);
   };
   connect(&settings, &SettingsManager::settingsUpdated, this, updateFont);
   updateFont();
   connect(undoButton, &QPushButton::clicked, code, &QPlainTextEdit::undo);
   connect(redoButton, &QPushButton::clicked, code, &QPlainTextEdit::redo);
   connect(saveButton, &QPushButton::clicked, this, &JsTexGenEditor::save);
   connect(reloadButton, &QPushButton::clicked, this, &JsTexGenEditor::reload);
   connect(code, &QPlainTextEdit::textChanged, this, &JsTexGenEditor::updateControls);
   connect(code, &QPlainTextEdit::undoAvailable, this, [this] { updateControls(); });
   connect(code, &QPlainTextEdit::redoAvailable, this, [this] { updateControls(); });
   updateControls();
}

QTextDocument* JsTexGenEditor::createDocument(const QString& source) {
   auto* document = new QTextDocument(this);
   document->setDocumentLayout(new QPlainTextDocumentLayout(document));
   document->setDefaultFont(code->font());
   document->setPlainText(source);
   document->setModified(false);
   new JavaScriptHighlighter(document);
   connect(document, &QTextDocument::modificationChanged, this, [this] { updateControls(); });
   return document;
}

void JsTexGenEditor::setGenerator(const TextureGeneratorPtr& selected) {
   if (selected == generator) {
      updateControls();
      return;
   }
   if (generator && drafts.contains(generator->getSourceIdentity())) {
      auto& draft = drafts[generator->getSourceIdentity()];
      draft.cursor = code->textCursor();
      draft.scroll = {code->horizontalScrollBar()->value(), code->verticalScrollBar()->value()};
   }
   generator = selected;
   auto* js = dynamic_cast<JsTexGen*>(generator.data());
   if (js) {
      const QString key = js->getSourceIdentity();
      auto& draft = drafts[key];
      if (!draft.document) {
         draft.document = createDocument(js->source());
         draft.cursor = QTextCursor(draft.document);
      }
      code->setDocument(draft.document);
      code->setTextCursor(draft.cursor);
      code->horizontalScrollBar()->setValue(draft.scroll.x());
      code->verticalScrollBar()->setValue(draft.scroll.y());
   } else {
      code->setDocument(emptyDocument);
   }
   status->clear();
   updateControls();
}

void JsTexGenEditor::updateControls() {
   const auto* js = dynamic_cast<JsTexGen*>(generator.data());
   const bool javascript = js != nullptr;
   const bool editable = javascript && generator->getOrigin() == TextureGenerator::Origin::Custom;
   code->setEnabled(javascript);
   code->setReadOnly(!editable);
   heading->setText(javascript ? generator->getName() + (code->document()->isModified() ? " *" : "")
                               : tr("Select a JavaScript generator in Nodes"));
   heading->setToolTip(javascript ? generator->getSourceIdentity() : QString());
   undoButton->setEnabled(editable && code->document()->isUndoAvailable());
   redoButton->setEnabled(editable && code->document()->isRedoAvailable());
   saveButton->setEnabled(editable && code->toPlainText() != js->source());
   reloadButton->setEnabled(editable);
   if (javascript) {
      emit draftMetadataChanged(
          draftProperty("name", generator->getName()),
          draftProperty("filename", QFileInfo(generator->getSourceIdentity()).fileName()));
   }
}

void JsTexGenEditor::save() {
   if (code->isReadOnly() || !generator) return;
   QString error;
   const auto replacement =
       manager.saveScript(generator, code->toPlainText(), generator->getSourceIdentity(), error);
   if (!replacement) {
      status->setText(error);
      status->setToolTip(error);
      return;
   }
   const auto draft = drafts.take(generator->getSourceIdentity());
   drafts.insert(replacement->getSourceIdentity(), draft);
   generator = replacement;
   code->document()->setModified(false);
   status->setText(tr("Saved"));
   status->setToolTip({});
   updateControls();
   emit generatorSaved(replacement);
}

void JsTexGenEditor::reload() {
   if (code->isReadOnly() || !generator) return;
   if (code->document()->isModified() &&
       QMessageBox::question(this, tr("Reload generator"),
                             tr("Discard this generator's unsaved edits and reload the file?")) !=
           QMessageBox::Yes)
      return;
   QFile file(generator->getSourceIdentity());
   if (!file.open(QIODevice::ReadOnly)) {
      status->setText(file.errorString());
      return;
   }
   code->setPlainText(QString::fromUtf8(file.readAll()));
   code->document()->setModified(false);
   status->clear();
   updateControls();
}

bool JsTexGenEditor::canClose() {
   for (const auto& draft : drafts) {
      if (draft.document->isModified())
         return QMessageBox::question(
                    this, tr("Unsaved generator edits"),
                    tr("Close this window and discard its unsaved generator edits?")) ==
                QMessageBox::Yes;
   }
   return true;
}

QString JsTexGenEditor::withProperty(QString source, const QString& property,
                                     const QString& value) {
   const QByteArray json = QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact);
   const QString quoted = QString::fromUtf8(json.mid(1, json.size() - 2));
   const auto match = metadataMatch(source, property);
   if (match.hasMatch()) {
      source.replace(match.capturedStart(1), match.capturedLength(1), quoted);
   } else {
      source += "\ngenerator." + property + " = " + quoted + ";\n";
   }
   return source;
}

void JsTexGenEditor::setMetadata(const QString& property, const QString& value) {
   if (code->isReadOnly() || !generator) return;
   const QString updated = withProperty(code->toPlainText(), property, value);
   if (updated == code->toPlainText()) return;
   QTextCursor cursor(code->document());
   cursor.beginEditBlock();
   cursor.select(QTextCursor::Document);
   cursor.insertText(updated);
   cursor.endEditBlock();
}

QString JsTexGenEditor::draftProperty(const QString& property, const QString& fallback) const {
   const auto match = metadataMatch(code->toPlainText(), property);
   if (!match.hasMatch()) return fallback;
   QString literal = match.captured(1);
   if (literal.startsWith('\'')) {
      literal = literal.mid(1, literal.size() - 2);
      literal.replace("\\'", "'");
      literal.replace('"', "\\\"");
      literal = '"' + literal + '"';
   }
   const auto parsed = QJsonDocument::fromJson(("[" + literal + "]").toUtf8());
   return parsed.isArray() && !parsed.array().isEmpty() ? parsed.array().at(0).toString()
                                                        : fallback;
}
