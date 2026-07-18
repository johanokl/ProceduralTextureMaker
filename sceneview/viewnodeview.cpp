
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "viewnodeview.h"
#include <QEasingCurve>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSettings>
#include <QVariantAnimation>
#include <QWheelEvent>
#include <QtMath>

/// @brief ViewNodeView::ViewNodeView
ViewNodeView::ViewNodeView() : scrollZoomFactor(1.0), zoomStepFactor(1.45) {
   setMouseTracking(true);
   setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                  QPainter::TextAntialiasing);
   setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, false);
   setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
   setZoomStepFactor(QSettings().value("zoomStepFactor", 1.45).toDouble());
   setDragMode(QGraphicsView::ScrollHandDrag);
   defaultZoomFactor = 1.0;
   zoomAnimation = new QVariantAnimation(this);
   zoomAnimation->setDuration(80);
   zoomAnimation->setEasingCurve(QEasingCurve::OutCubic);
   zoomAnimationLastFactor = 1.0;
   zoomAnimationUsesViewportAnchor = false;
   updatingSceneRect = false;
   QObject::connect(zoomAnimation, &QVariantAnimation::valueChanged, this,
                    [this](const QVariant& value) { applyAnimatedZoom(value.toDouble()); });
   scale(defaultZoomFactor, defaultZoomFactor);
}

/// @brief ViewNodeView::setZoomStepFactor
/// @param factor Zoom factor for one button press or mouse wheel step.
void ViewNodeView::setZoomStepFactor(double factor) {
   if (factor <= 1.0 || factor > 3.0) {
      factor = 1.45;
   }
   zoomStepFactor = factor;
   scrollZoomFactor = qPow(zoomStepFactor, 1.0 / 120.0);
}

/// @brief ViewNodeView::setNodeScene
/// @param scene New scene to display.
void ViewNodeView::setNodeScene(QGraphicsScene* scene) {
   if (sceneChangedConnection) {
      QObject::disconnect(sceneChangedConnection);
      sceneChangedConnection = QMetaObject::Connection();
   }
   QGraphicsView::setScene(scene);
   if (scene) {
      sceneChangedConnection =
          QObject::connect(scene, &QGraphicsScene::changed, this,
                           [this](const QList<QRectF>&) { updateSceneRect(); });
      updateSceneRect();
   }
}

/// @brief ViewNodeView::showAllNodes
/// Zooms to show all scene items.
void ViewNodeView::showAllNodes() {
   QGraphicsScene* currentScene = scene();
   if (!currentScene) {
      return;
   }

   QRectF contentRect = currentScene->itemsBoundingRect();
   if (contentRect.isNull()) {
      return;
   }

   zoomAnimation->stop();
   zoomAnimationLastFactor = 1.0;

   qreal padding = qMax(contentRect.width(), contentRect.height()) * 0.05;
   padding = qMax(padding, 80.0);
   contentRect.adjust(-padding, -padding, padding, padding);

   const ViewportAnchor anchor = transformationAnchor();
   setTransformationAnchor(QGraphicsView::NoAnchor);
   resetTransform();
   scale(defaultZoomFactor, defaultZoomFactor);
   updateSceneRect(contentRect.center());
   fitInView(contentRect, Qt::KeepAspectRatio);
   if (transform().m11() > defaultZoomFactor) {
      resetTransform();
      scale(defaultZoomFactor, defaultZoomFactor);
   }
   updateSceneRect(contentRect.center());
   centerOn(contentRect.center());
   setTransformationAnchor(anchor);
}

/// @brief ViewNodeView::resetZoom
/// Resets the zoom to 100%.
void ViewNodeView::resetZoom() {
   const ViewportAnchor anchor = transformationAnchor();
   setTransformationAnchor(QGraphicsView::AnchorViewCenter);
   resetTransform();
   scale(defaultZoomFactor, defaultZoomFactor);
   updateSceneRect();
   setTransformationAnchor(anchor);
}

/// @brief ViewNodeView::zoomIn
/// Zooms in while keeping the current view center.
void ViewNodeView::zoomIn() { zoomCentered(zoomStepFactor); }

/// @brief ViewNodeView::zoomOut
/// Zooms out while keeping the current view center.
void ViewNodeView::zoomOut() { zoomCentered(1.0 / zoomStepFactor); }

/// @brief ViewNodeView::applyAnimatedZoom
/// @param value Animation value where 1.0 is the starting zoom level.
void ViewNodeView::applyAnimatedZoom(double value) {
   double step = value / zoomAnimationLastFactor;
   zoomAnimationLastFactor = value;
   if (qFuzzyCompare(step, 1.0)) {
      return;
   }

   const ViewportAnchor anchor = transformationAnchor();
   setTransformationAnchor(QGraphicsView::NoAnchor);
   scale(step, step);

   QPointF center = zoomAnimationSceneAnchor;
   if (zoomAnimationUsesViewportAnchor) {
      QPointF viewCenter(viewport()->rect().center());
      QPointF sceneVector = mapToScene(zoomAnimationViewportPos) - mapToScene(viewCenter.toPoint());
      center = zoomAnimationSceneAnchor - sceneVector;
   }
   updateSceneRect(center);
   centerOn(center);
   setTransformationAnchor(anchor);
}

/// @brief ViewNodeView::startZoomAnimation
/// @param factor Final zoom factor.
/// @param viewportPos Viewport point to keep fixed.
/// @param useViewportAnchor If true, keep viewportPos fixed instead of the center.
void ViewNodeView::startZoomAnimation(double factor, const QPoint& viewportPos,
                                      bool useViewportAnchor) {
   zoomAnimation->stop();
   zoomAnimationLastFactor = 1.0;
   zoomAnimationViewportPos = viewportPos;
   zoomAnimationUsesViewportAnchor = useViewportAnchor;
   if (useViewportAnchor) {
      zoomAnimationSceneAnchor = mapToScene(viewportPos);
   } else {
      zoomAnimationSceneAnchor = mapToScene(viewport()->rect().center());
   }

   zoomAnimation->setStartValue(1.0);
   zoomAnimation->setEndValue(factor);
   zoomAnimation->start();
}

/// @brief ViewNodeView::zoomAt
/// @param viewportPos Viewport point to keep fixed.
/// @param factor Zoom factor.
void ViewNodeView::zoomAt(const QPoint& viewportPos, double factor) {
   startZoomAnimation(factor, viewportPos, true);
}

/// @brief ViewNodeView::zoomCentered
/// @param factor Zoom factor.
void ViewNodeView::zoomCentered(double factor) {
   startZoomAnimation(factor, viewport()->rect().center(), false);
}

/// @brief ViewNodeView::updateSceneRect
/// Updates the scrollable scene area around the current view center.
void ViewNodeView::updateSceneRect() { updateSceneRect(mapToScene(viewport()->rect().center())); }

/// @brief ViewNodeView::updateSceneRect
/// @param center Scene position that should remain reachable.
void ViewNodeView::updateSceneRect(const QPointF& center) {
   QGraphicsScene* currentScene = scene();
   if (!currentScene || updatingSceneRect) {
      return;
   }

   QRectF contentRect = currentScene->itemsBoundingRect();
   if (contentRect.isNull()) {
      contentRect = QRectF(center.x() - 100, center.y() - 100, 200, 200);
   }

   QRectF visibleRect = mapToScene(viewport()->rect()).boundingRect();
   if (visibleRect.isNull()) {
      visibleRect = QRectF(center.x() - 100, center.y() - 100, 200, 200);
   }

   QRectF centerRect(center.x() - visibleRect.width(), center.y() - visibleRect.height(),
                     visibleRect.width() * 2, visibleRect.height() * 2);
   QRectF sceneRect = contentRect.united(centerRect);
   qreal horizontalPadding = qMax(visibleRect.width(), 300.0);
   qreal verticalPadding = qMax(visibleRect.height(), 300.0);
   sceneRect.adjust(-horizontalPadding, -verticalPadding, horizontalPadding, verticalPadding);

   QRectF oldSceneRect = currentScene->sceneRect();
   if (qAbs(oldSceneRect.left() - sceneRect.left()) < 1.0 &&
       qAbs(oldSceneRect.top() - sceneRect.top()) < 1.0 &&
       qAbs(oldSceneRect.width() - sceneRect.width()) < 1.0 &&
       qAbs(oldSceneRect.height() - sceneRect.height()) < 1.0) {
      return;
   }

   updatingSceneRect = true;
   currentScene->setSceneRect(sceneRect);
   updatingSceneRect = false;
}

/// @brief ViewNodeView::resizeEvent
/// @param event Qt resize event.
void ViewNodeView::resizeEvent(QResizeEvent* event) {
   QGraphicsView::resizeEvent(event);
   updateSceneRect();
}

/// @brief ViewNodeView::wheelEvent
/// @param event
///
/// Zooms in and out from the screen when the user scrolls the
/// mouse wheel while pressing the Shift or Alt keys.
void ViewNodeView::wheelEvent(QWheelEvent* event) {
   if (event->modifiers() & (Qt::ShiftModifier | Qt::AltModifier)) {
      double angle = event->angleDelta().y();
      if (angle == 0) {
         event->accept();
         return;
      }
      double factor = qPow(scrollZoomFactor, angle);
      if (angle > 0) {
         zoomAt(event->position().toPoint(), factor);
      } else {
         zoomCentered(factor);
      }
      event->accept();
      return;
   }
   QGraphicsView::wheelEvent(event);
}
