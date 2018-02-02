/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QWidget>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>
#include "viewnodeview.h"
#include "viewnodescene.h"

/**
 * @brief ViewNodeView::ViewNodeView
 * @param defaultzoom Zoom factor in percentage, 100 is 1:1.
 */
ViewNodeView::ViewNodeView(int defaultzoom)
{
   setMouseTracking(true);
   // The mouse wheel scroll factor
   scrollZoomFactor = 1.0015;
   setDragMode(QGraphicsView::ScrollHandDrag);
   defaultZoomFactor = (double) defaultzoom / 100;
   scale(defaultZoomFactor, defaultZoomFactor);
}

/**
 * @brief ViewNodeView::~ViewNodeView
 */
ViewNodeView::~ViewNodeView()
{
}

/**
 * @brief ViewNodeView::setDefaultZoom
 * @param zoom Zoom factor in percentage, 100 is 1:1.
 */
void ViewNodeView::setDefaultZoom(int zoom)
{
   if (zoom != defaultZoomFactor) {
      defaultZoomFactor = (double) zoom / 100;
      resetZoom();
   }
}

/**
 * @brief ViewNodeView::resetZoom
 * Resets to the default zoom, 100%.
 */
void ViewNodeView::resetZoom()
{
   const ViewportAnchor anchor = transformationAnchor();
   setTransformationAnchor(QGraphicsView::AnchorViewCenter);
   resetTransform();
   scale(defaultZoomFactor, defaultZoomFactor);
   setTransformationAnchor(anchor);
}

/**
 * @brief ViewNodeView::wheelEvent
 * @param event
 *
 * Zooms in and out from the screen when the user scrolls the
 * mouse wheel while pressing the Shift or Alt keys.
 */
void ViewNodeView::wheelEvent(QWheelEvent* event) {
   QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
   if ((QApplication::keyboardModifiers() == Qt::ShiftModifier
       || QApplication::keyboardModifiers() == Qt::AltModifier)
       && wheel_event->orientation() == Qt::Vertical) {
      const ViewportAnchor anchor = transformationAnchor();
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      double angle = wheel_event->angleDelta().y();
      double factor = qPow(scrollZoomFactor, angle);
      scale(factor, factor);
      setTransformationAnchor(anchor);
   }
   QGraphicsView::wheelEvent(event);
}


