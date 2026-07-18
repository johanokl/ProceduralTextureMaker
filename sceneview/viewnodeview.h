
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODEVIEW_H
#define VIEWNODEVIEW_H

#include <QGraphicsView>
#include <QMetaObject>
#include <QObject>
class QVariantAnimation;

/// @brief The ViewNodeView class
///
/// Displays a ViewNodeScene instance with ViewNodeItem/ViewNodeLine objects.
/// Supports zooming and scrolling, both with mouse dragging and scrollbars.
class ViewNodeView : public QGraphicsView {
public:
   ViewNodeView();
   ~ViewNodeView() override = default;
   void setNodeScene(QGraphicsScene* scene);
   void setZoomStepFactor(double factor);
   void showAllNodes();

public slots:
   void resetZoom();
   void zoomIn();
   void zoomOut();

protected:
   void resizeEvent(QResizeEvent* event) override;
   void wheelEvent(QWheelEvent* event) override;

private:
   void applyAnimatedZoom(double value);
   void startZoomAnimation(double factor, const QPoint& viewportPos, bool useViewportAnchor);
   void updateSceneRect();
   void updateSceneRect(const QPointF& center);
   void zoomAt(const QPoint& viewportPos, double factor);
   void zoomCentered(double factor);

   QMetaObject::Connection sceneChangedConnection;
   QVariantAnimation* zoomAnimation;
   QPoint zoomAnimationViewportPos;
   QPointF zoomAnimationSceneAnchor;
   double scrollZoomFactor;
   double zoomStepFactor;
   double zoomAnimationLastFactor;
   double defaultZoomFactor;
   bool zoomAnimationUsesViewportAnchor;
   bool updatingSceneRect;
};

#endif  // VIEWNODEVIEW_H
