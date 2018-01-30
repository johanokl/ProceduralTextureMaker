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
 */
ViewNodeView::ViewNodeView()
{
   setMouseTracking(true);
   // The mouse wheel scroll factor
   zoomFactor = 1.0015;
   setDragMode(QGraphicsView::ScrollHandDrag);
   scale(0.33, 0.33);
}

/**
 * @brief ViewNodeView::~ViewNodeView
 */
ViewNodeView::~ViewNodeView()
{
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
      double factor = qPow(zoomFactor, angle);
      scale(factor, factor);
      setTransformationAnchor(anchor);
   }
   QGraphicsView::wheelEvent(event);
}


