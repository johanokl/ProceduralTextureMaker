
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef VIEWNODEVIEW_H
#define VIEWNODEVIEW_H

#include <QGraphicsView>
#include <QMetaObject>
#include <QObject>
#include <QVariantAnimation>
#include <QTimer>
class QLabel;

/// @brief Displays and navigates a scene containing texture nodes and connections.
/// Supports zooming and scrolling with mouse gestures, actions, and scrollbars.
class ViewNodeView : public QGraphicsView {
public:
   /// @brief Creates an empty graph view with the configured zoom behavior.
   ViewNodeView();

   /// @brief Destroys the graph view.
   ~ViewNodeView() override = default;

   /// @brief Sets the scene displayed by the view.
   /// @param scene Scene to display, or nullptr to detach the current scene.
   void setNodeScene(QGraphicsScene* scene);

   /// @brief Sets the zoom factor applied by one action or mouse-wheel step.
   /// @param factor Zoom factor in the supported range.
   void setZoomStepFactor(double factor);

   /// @brief Fits all scene items within the viewport.
   void showAllNodes();

public slots:
   /// @brief Restores the default zoom level.
   void resetZoom();

   /// @brief Zooms in while keeping the current view center.
   void zoomIn();

   /// @brief Zooms out while keeping the current view center.
   void zoomOut();

protected:
   /// @brief Shows graph tooltips and hides them immediately when their item is left.
   /// @param event Event delivered to the graph viewport.
   /// @return True when the tooltip event was handled.
   bool viewportEvent(QEvent* event) override;

   /// @brief Updates the scrollable scene area after the viewport is resized.
   /// @param event Resize event.
   void resizeEvent(QResizeEvent* event) override;

   /// @brief Zooms the scene in or out in response to mouse-wheel scrolling.
   /// @param event Mouse-wheel event.
   void wheelEvent(QWheelEvent* event) override;

private:
   /// @brief Updates tooltip text and position, or waits for the hover delay.
   /// @param show Whether the delay has elapsed and the tooltip can be shown.
   void updateGraphTooltip(bool show);

   /// @brief Applies one frame of the active zoom animation.
   /// @param value Animation value relative to the initial zoom level.
   void applyAnimatedZoom(double value);

   /// @brief Starts an animated zoom around a viewport position or the view center.
   /// @param factor Final zoom factor.
   /// @param viewportPos Viewport position used as the optional anchor.
   /// @param useViewportAnchor Whether to keep @p viewportPos fixed.
   void startZoomAnimation(double factor, const QPoint& viewportPos, bool useViewportAnchor);

   /// @brief Updates the scrollable scene area around the current view center.
   void updateSceneRect();

   /// @brief Updates the scrollable scene area around a scene position.
   /// @param center Scene position that must remain reachable.
   void updateSceneRect(const QPointF& center);

   /// @brief Starts a zoom that keeps a viewport position fixed.
   /// @param viewportPos Viewport position to keep fixed.
   /// @param factor Zoom factor.
   void zoomAt(const QPoint& viewportPos, double factor);

   /// @brief Starts a zoom around the current view center.
   /// @param factor Zoom factor.
   void zoomCentered(double factor);

   /// @brief Mouse-transparent tooltip owned by this graph view.
   QLabel* graphTooltip;
   /// @brief Limits how long a graph tooltip remains visible.
   QTimer tooltipTimer;
   /// @brief Waits until the pointer rests before showing a tooltip.
   QTimer tooltipDelay;
   /// @brief Latest pointer position in viewport coordinates.
   QPoint tooltipPosition;

   /// @brief Connection used to observe changes in the displayed scene.
   QMetaObject::Connection sceneChangedConnection;
   /// @brief Animation that interpolates zoom operations.
   QVariantAnimation zoomAnimation;
   /// @brief Viewport anchor used by the current zoom animation.
   QPoint zoomAnimationViewportPos;
   /// @brief Scene anchor used by the current zoom animation.
   QPointF zoomAnimationSceneAnchor;
   /// @brief Per-wheel-unit zoom multiplier.
   double scrollZoomFactor;
   /// @brief Zoom multiplier applied by one action or wheel step.
   double zoomStepFactor;
   /// @brief Previous interpolation value used to calculate the next zoom step.
   double zoomAnimationLastFactor;
   /// @brief Scale used by reset and fit operations.
   double defaultZoomFactor;
   /// @brief Whether the current animation preserves a viewport anchor.
   bool zoomAnimationUsesViewportAnchor;
   /// @brief Whether the scene rectangle is currently being updated.
   bool updatingSceneRect;
};

#endif  // VIEWNODEVIEW_H
