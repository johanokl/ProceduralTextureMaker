
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/applicationtheme.h"
#include <QApplication>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QStyleHints>

namespace {

bool shouldUseDarkTheme(const QApplication& app) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
   const Qt::ColorScheme colorScheme = QGuiApplication::styleHints()->colorScheme();
   if (colorScheme == Qt::ColorScheme::Dark) {
      return true;
   }
   if (colorScheme == Qt::ColorScheme::Light) {
      return false;
   }
#endif
   return app.palette().color(QPalette::Window).lightness() < 128;
}

QPalette darkPalette() {
   QPalette palette;
   palette.setColor(QPalette::Window, QColor("#15171c"));
   palette.setColor(QPalette::WindowText, QColor("#e8eaed"));
   palette.setColor(QPalette::Base, QColor("#1b1e24"));
   palette.setColor(QPalette::AlternateBase, QColor("#222630"));
   palette.setColor(QPalette::ToolTipBase, QColor("#2b303a"));
   palette.setColor(QPalette::ToolTipText, QColor("#f5f6f7"));
   palette.setColor(QPalette::Text, QColor("#e8eaed"));
   palette.setColor(QPalette::Button, QColor("#252a33"));
   palette.setColor(QPalette::ButtonText, QColor("#e8eaed"));
   palette.setColor(QPalette::BrightText, QColor("#ffffff"));
   palette.setColor(QPalette::Link, QColor("#65c8ff"));
   palette.setColor(QPalette::Highlight, QColor("#39a9dc"));
   palette.setColor(QPalette::HighlightedText, QColor("#071016"));
   palette.setColor(QPalette::PlaceholderText, QColor("#8e96a3"));
   palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#767d89"));
   palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#767d89"));
   palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#767d89"));
   return palette;
}

QString styleSheet(bool dark, bool nativeMacStyle) {
   const QString window = dark ? "#15171c" : "#f4f5f7";
   const QString panel = dark ? "#1b1e24" : "#ffffff";
   const QString panelRaised = dark ? "#222630" : "#eef1f4";
   const QString control = dark ? "#252a33" : "#ffffff";
   const QString controlHover = dark ? "#303744" : "#eef4f8";
   const QString controlPressed = dark ? "#1f8db8" : "#dceff8";
   const QString text = dark ? "#e8eaed" : "#20242a";
   const QString muted = dark ? "#aab1bd" : "#5c6470";
   const QString border = dark ? "#3b414c" : "#c8cdd4";
   const QString borderStrong = dark ? "#596170" : "#a9b0ba";
   const QString accent = dark ? "#4fc3f7" : "#1687b8";
   const QString accentMuted = dark ? "#2c6178" : "#d8edf6";
   const QString inspectorPanel = dark ? panel : panelRaised;
   const QString widgetFontSize = nativeMacStyle ? QStringLiteral("13pt") : QStringLiteral("10pt");
   const QString inspectorLabelFontSize =
       nativeMacStyle ? QStringLiteral("12pt") : QStringLiteral("9pt");

   QString platformRules;
   if (nativeMacStyle) {
      platformRules = QStringLiteral(R"(
QToolBar {
   padding: 7px 9px;
   spacing: 5px;
}

QToolBar QToolButton {
   border-radius: 7px;
   min-height: 30px;
   min-width: 30px;
   padding: 6px 10px;
}

QPushButton {
   border-radius: 7px;
   min-height: 29px;
   padding: 6px 14px;
}

QLineEdit,
QComboBox,
QSpinBox,
QDoubleSpinBox {
   border-radius: 7px;
   min-height: 28px;
   padding-top: 4px;
   padding-bottom: 4px;
}

QGroupBox {
   border-radius: 9px;
   margin-top: 18px;
   padding: 15px 10px 10px 10px;
}

#nodeSettingsInspector QGroupBox[inspectorSection="true"] {
   margin-top: 24px;
   padding-top: 19px;
}
)");
   }

   return QString(R"(
QMainWindow {
   background: %1;
}

QWidget {
   color: %5;
   font-size: %12;
}

QMenuBar {
   background: %1;
   border-bottom: 1px solid %7;
   padding: 3px 6px;
}

QMenuBar::item {
   background: transparent;
   border-radius: 5px;
   padding: 5px 10px;
}

QMenuBar::item:selected {
   background: %4;
}

QMenu {
   background: %2;
   border: 1px solid %7;
   padding: 6px;
}

QMenu::item {
   border-radius: 5px;
   padding: 6px 26px 6px 12px;
}

QMenu::item:selected {
   background: %4;
}

QMenu::separator {
   height: 1px;
   background: %7;
   border: 0;
   margin: 5px 8px;
}

QToolBar {
   background: %1;
   border: 0;
   border-bottom: 1px solid %7;
   spacing: 4px;
   padding: 4px 6px;
}

QToolBar::separator {
   background: %7;
   width: 1px;
   margin: 4px 6px;
}

QToolButton {
   background: transparent;
   border: 1px solid transparent;
   border-radius: 6px;
   padding: 5px 8px;
}

QToolButton:hover {
   background: %4;
   border-color: %7;
}

QToolButton:pressed {
   background: %10;
   border-color: %9;
}

QGroupBox {
   background: %2;
   border: 1px solid %7;
   border-radius: 7px;
   margin-top: 16px;
   padding: 13px 8px 8px 8px;
   font-weight: 600;
}

QGroupBox::title {
   subcontrol-origin: margin;
   subcontrol-position: top left;
   left: 9px;
   padding: 0 5px;
   color: %6;
   background: %1;
}

QLineEdit,
QComboBox,
QSpinBox,
QDoubleSpinBox,
QPlainTextEdit,
QTextEdit {
   background: %3;
   border: 1px solid %7;
   border-radius: 6px;
   padding: 4px 7px;
   min-height: 22px;
   selection-background-color: %9;
}

QLineEdit:focus,
QComboBox:focus,
QSpinBox:focus,
QDoubleSpinBox:focus,
QPlainTextEdit:focus,
QTextEdit:focus {
   border-color: %9;
}

QPushButton {
   background: %3;
   border: 1px solid %7;
   border-radius: 6px;
   padding: 5px 12px;
   min-height: 23px;
}

QPushButton:hover {
   background: %4;
   border-color: %8;
}

QPushButton:pressed,
QPushButton:checked {
   background: %10;
   border-color: %9;
}

QScrollArea,
QAbstractScrollArea {
   background: transparent;
   border: 0;
}

QSlider::groove:horizontal {
   background: %7;
   border-radius: 2px;
   height: 4px;
}

QSlider::sub-page:horizontal {
   background: %9;
   border-radius: 2px;
}

QSlider::handle:horizontal {
   background: %9;
   border: 1px solid %8;
   border-radius: 7px;
   width: 14px;
   height: 14px;
   margin: -6px 0;
}

QSplitter::handle {
   background: %1;
}

QSplitter::handle:horizontal {
   width: 6px;
}

QSplitter::handle:vertical {
   height: 6px;
}

QScrollBar:vertical,
QScrollBar:horizontal {
   background: %1;
   border: 0;
   margin: 0;
}

QScrollBar:vertical {
   width: 12px;
}

QScrollBar:horizontal {
   height: 12px;
}

QScrollBar::handle {
   background: %8;
   border-radius: 6px;
   min-height: 24px;
   min-width: 24px;
}

QScrollBar::handle:hover {
   background: %9;
}

QScrollBar::add-line,
QScrollBar::sub-line,
QScrollBar::add-page,
QScrollBar::sub-page {
   background: transparent;
   border: 0;
}

#nodeSettingsInspector,
#nodeSettingsContents,
#nodeSettingsInspector QScrollArea,
#nodeSettingsInspector QScrollArea > QWidget > QWidget {
   background: %11;
}

#nodeSettingsInspector QGroupBox[inspectorSection="true"] {
   background: transparent;
   border: 0;
   border-top: 1px solid %7;
   border-radius: 0;
   margin-top: 22px;
   padding: 17px 2px 4px 2px;
   font-weight: 600;
}

#nodeSettingsInspector QGroupBox[inspectorSection="true"]::title {
   subcontrol-origin: margin;
   subcontrol-position: top left;
   left: 2px;
   padding: 0 8px 0 0;
   color: %5;
   background: %11;
}

QLabel#previewImage {
   background: #000000;
   border: 1px solid #000000;
}

#nodeSettingsInspector QWidget[inspectorProperty="true"] {
   background: transparent;
   border: 0;
}

#nodeSettingsInspector QLabel[inspectorLabel="true"] {
   color: %5;
   font-size: %13;
   font-weight: 500;
}

#nodeSettingsInspector QLabel#generatorBadge {
   background: %10;
   border: 1px solid %9;
   border-radius: 6px;
   color: %5;
   font-weight: 600;
   padding: 5px 8px;
}

#nodeSettingsInspector QLineEdit,
#nodeSettingsInspector QComboBox,
#nodeSettingsInspector QSpinBox,
#nodeSettingsInspector QDoubleSpinBox,
#nodeSettingsInspector QPlainTextEdit,
#nodeSettingsInspector QPushButton {
   min-height: 28px;
}

#nodeSettingsInspector QComboBox,
#nodeSettingsInspector QLineEdit,
#nodeSettingsInspector QPushButton[colorControl="true"] {
   padding-left: 10px;
   padding-right: 10px;
}

#nodeSettingsInspector QCheckBox {
   spacing: 9px;
   min-height: 28px;
}

#nodeSettingsInspector QSlider[inspectorSlider="true"] {
   min-height: 18px;
}

#nodeSettingsInspector QSlider[inspectorSlider="true"]::groove:horizontal {
   height: 5px;
}

#nodeSettingsInspector QSlider[inspectorSlider="true"]::handle:horizontal {
   width: 16px;
   height: 16px;
   border-radius: 8px;
   margin: -6px 0;
}

%14
)")
       .arg(window, panel, control, controlHover, text, muted, border, borderStrong, accent,
            accentMuted, inspectorPanel, widgetFontSize, inspectorLabelFontSize, platformRules);
}

}  // namespace
void ApplicationTheme::apply(QApplication& app) {
#ifdef Q_OS_MAC
   const bool nativeMacStyle = true;
   QFont applicationFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
   applicationFont.setPointSizeF(13.0);
   app.setFont(applicationFont);
#else
   const bool nativeMacStyle = false;
   app.setStyle(QStyleFactory::create("Fusion"));
#endif
   const bool dark = shouldUseDarkTheme(app);
   if (dark && !nativeMacStyle) {
      app.setPalette(darkPalette());
   }
   app.setStyleSheet(styleSheet(dark, nativeMacStyle));
}
