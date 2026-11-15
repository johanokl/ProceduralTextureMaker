// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef HOVERTOOLTIP_H
#define HOVERTOOLTIP_H

#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <QToolTip>

/// @brief Shows a widget's tooltip after 500 ms and follows the mouse without intercepting it.
class HoverTooltip : public QObject {
public:
   /// @brief Adds a delayed popup to a widget and tracks movement over its children.
   /// @param widget Widget whose tooltip text is displayed.
   explicit HoverTooltip(QWidget* widget) : QObject(widget), target(widget) {
      auto widgets = widget->findChildren<QWidget*>();
      widgets.append(widget);
      for (QWidget* child : widgets) {
         child->setMouseTracking(true);
         child->installEventFilter(this);
      }
      popup = new QLabel(widget, Qt::ToolTip | Qt::WindowTransparentForInput);
      popup->setObjectName("hoverTooltip");
      popup->setTextFormat(Qt::PlainText);
      popup->setFont(QToolTip::font());
      popup->setPalette(QToolTip::palette());
      popup->setBackgroundRole(QPalette::ToolTipBase);
      popup->setForegroundRole(QPalette::ToolTipText);
      popup->setAutoFillBackground(true);
      popup->setMargin(4);
      delay.setSingleShot(true);
      delay.setInterval(500);
      connect(&delay, &QTimer::timeout, this, [this] { showPopup(); });
      lifetime.setSingleShot(true);
      connect(&lifetime, &QTimer::timeout, popup, &QWidget::hide);
      connect(qApp, &QGuiApplication::applicationStateChanged, this,
              [this](Qt::ApplicationState state) {
                 if (state != Qt::ApplicationActive) {
                    hidePopup();
                 }
              });
   }

protected:
   /// @brief Tracks pointer movement while allowing normal clicks and drags.
   bool eventFilter(QObject* watched, QEvent* event) override {
      if (event->type() == QEvent::ToolTip) {
         event->accept();
         return true;
      }
      if (watched == target && event->type() == QEvent::Enter) {
         position = static_cast<QEnterEvent*>(event)->globalPosition().toPoint();
         delay.start();
      } else if (event->type() == QEvent::MouseMove &&
                 static_cast<QMouseEvent*>(event)->buttons() == Qt::NoButton) {
         position = static_cast<QMouseEvent*>(event)->globalPosition().toPoint();
         if (popup->isVisible()) {
            showPopup();
         } else {
            delay.start();
         }
      } else if ((watched == target &&
                  (event->type() == QEvent::Leave || event->type() == QEvent::Hide)) ||
                 event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove ||
                 event->type() == QEvent::Wheel) {
         hidePopup();
      }
      return false;
   }

private:
   /// @brief Displays the popup beside the pointer, keeping it within the screen.
   void showPopup() {
      if (!target->isVisible() || target->toolTip().isEmpty()) {
         hidePopup();
         return;
      }
      const bool wasVisible = popup->isVisible();
      const QRect screen = target->screen()->availableGeometry();
      popup->setMaximumWidth(screen.width());
      popup->setWordWrap(true);
      popup->setText(target->toolTip());
      popup->adjustSize();
      QPoint location = position + QPoint(2, 20);
      location.setX(qMax(screen.left(), qMin(location.x(), screen.right() - popup->width())));
      location.setY(qMax(screen.top(), qMin(location.y(), screen.bottom() - popup->height())));
      popup->move(location);
      if (!wasVisible) {
         QToolTip::hideText();
         popup->show();
         lifetime.start(10000 + 40 * qMax(0, target->toolTip().size() - 100));
      }
   }

   /// @brief Cancels pending display and hides the popup.
   void hidePopup() {
      delay.stop();
      lifetime.stop();
      popup->hide();
   }

   /// @brief Widget supplying the popup text and lifetime.
   QWidget* target;
   /// @brief Mouse-transparent popup, matching the graph tooltip's appearance.
   QLabel* popup{nullptr};
   /// @brief Delay before the popup appears.
   QTimer delay;
   /// @brief Limits how long the popup remains visible.
   QTimer lifetime;
   /// @brief Latest global pointer position.
   QPoint position;
};

#endif  // HOVERTOOLTIP_H
