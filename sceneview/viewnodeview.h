/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef VIEWNODEVIEW_H
#define VIEWNODEVIEW_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>

class ViewNodeScene;

/**
 * @brief The ViewNodeView class
 *
 * Displays a ViewNodeScene instance with ViewNodeItem/ViewNodeLine objects.
 * Supports zooming and scrolling, both with mouse dragging and scrollbars.
 */
class ViewNodeView : public QGraphicsView
{
public:
   ViewNodeView();
   virtual ~ViewNodeView();

protected:
   void wheelEvent(QWheelEvent* event) override;

private:
   double zoomFactor;
};

#endif // VIEWNODEVIEW_H
