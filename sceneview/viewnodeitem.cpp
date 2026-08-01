
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/clipboardoperations.h"
#include "gui/mainwindow.h"
#include "sceneview/viewnodeitem.h"
#include "sceneview/viewnodeline.h"
#include "sceneview/viewnodescene.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

ViewNodeItem::ViewNodeItem(ViewNodeScene& scene, const TextureNodePtr& newNode)
    : id(newNode->getId()), scene(scene), texNode(newNode) {
   imageValid = false;
   setFlag(QGraphicsItem::ItemIsSelectable);

   mousePressed = false;
   isConnectable = false;
   isUnconnectable = false;
   highlightSlot = -1;
   highlighterWidth = 4;
   borderWidth = 1;
   headerSize = 24;

   setThumbnailSize(scene.getTextureProject().getThumbnailSize());
   setAcceptHoverEvents(true);
   positionUpdated();
   imageUpdated();
   positionUpdated();
   settingsUpdated();
}

void ViewNodeItem::setThumbnailSize(QSize size) {
   prepareGeometryChange();
   pixmap = QPixmap();
   update();
   thumbnailSize = size;
   imageAvailable(thumbnailSize);
}

void ViewNodeItem::setHeaderSize(int size) {
   if (size != 0 && (size < 8 || size > 48)) {
      size = 24;
   }
   if (headerSize == size) {
      return;
   }
   prepareGeometryChange();
   headerSize = size;
   update();
}

QPainterPath ViewNodeItem::shape() const {
   QPainterPath path;
   QRectF cardRect(0, -headerSize, thumbnailSize.width() + borderWidth * 2,
                   thumbnailSize.height() + borderWidth * 2 + headerSize);
   qreal outlineWidth = 1.0;
   qreal outlineOffset = 0.0;
   qreal radius = 6.0;
   if (isConnectable || isUnconnectable) {
      outlineWidth = highlighterWidth;
      outlineOffset = highlighterWidth / 2.0;
      radius = 7.0;
   } else if (isSelected()) {
      outlineWidth = 3.0;
      outlineOffset = highlighterWidth / 2.0;
      radius = 7.0;
   }
   qreal margin = outlineOffset + outlineWidth / 2.0;
   path.addRoundedRect(cardRect.adjusted(-margin, -margin, margin, margin),
                       radius + outlineWidth / 2.0, radius + outlineWidth / 2.0);
   return path;
}

QRectF ViewNodeItem::boundingRect() const {
   return QRectF(-highlighterWidth, -highlighterWidth - headerSize,
                 thumbnailSize.width() + borderWidth * 2 + highlighterWidth * 2,
                 thumbnailSize.height() + borderWidth * 2 + highlighterWidth * 2 + headerSize);
}

bool ViewNodeItem::posInImage(QPointF pos) const {
   return QRectF(0, 0, thumbnailSize.width() + borderWidth * 2,
                 thumbnailSize.height() + borderWidth * 2)
       .contains(pos);
}

void ViewNodeItem::updateConnectionLines() {
   QSetIterator<ViewNodeLine*> startIterator(startLines);
   while (startIterator.hasNext()) {
      startIterator.next()->updatePos();
   }
   QMapIterator<int, ViewNodeLine*> endIterator(endLines);
   while (endIterator.hasNext()) {
      endIterator.next().value()->updatePos();
   }
}

void ViewNodeItem::positionUpdated() {
   setPos(texNode->getPos());
   updateConnectionLines();
}

void ViewNodeItem::settingsUpdated() {
   titleString = texNode->getName().append(" (%1)").arg(texNode->getGeneratorName());
   setToolTip(titleString);
}

void ViewNodeItem::imageUpdated() {
   imageValid = false;
   update();
}

void ViewNodeItem::showConnectable(bool isConnectable) {
   if (this->isConnectable != isConnectable) {
      this->isConnectable = isConnectable;
      update();
   }
}

void ViewNodeItem::showUnconnectable(bool isUnconnectable) {
   if (this->isUnconnectable != isUnconnectable) {
      this->isUnconnectable = isUnconnectable;
      update();
   }
}

void ViewNodeItem::clearOverlays() {
   if (this->isConnectable) {
      this->isConnectable = false;
      update();
   }
   if (this->isUnconnectable) {
      this->isUnconnectable = false;
      update();
   }
}

void ViewNodeItem::addConnectionLine(ViewNodeLine* line) {
   if (line->getStartItemId() == id) {
      startLines.insert(line);
   } else {
      endLines.insert(line->getSlot(), line);
   }
}

void ViewNodeItem::removeConnectionLine(ViewNodeLine* line) {
   if (line->getStartItemId() == id) {
      startLines.remove(line);
   } else {
      endLines.remove(line->getSlot());
   }
}

void ViewNodeItem::imageAvailable(QSize size) {
   if (size == thumbnailSize) {
      const TextureImagePtr image = texNode->cachedImage(thumbnailSize);
      if (image.isNull()) {
         return;
      }
      QImage tempimage =
          QImage(thumbnailSize.width(), thumbnailSize.height(), QImage::Format_ARGB32);
      memcpy(tempimage.bits(), image->data(), image->byteSize());
      pixmap = QPixmap::fromImage(tempimage);
      imageValid = true;
      update();
   }
}

void ViewNodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
   event->accept();
   QMenu menu;
   QMap<int, QAction*> actions;
   for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
      if (texNode->getSources().value(i) != 0) {
         actions[i] = menu.addAction(QString("Remove source for slot %1").arg(i + 1));
      }
   }
   menu.addSeparator();
   QAction* copyNodeAction = menu.addAction(QString("Copy node"));
   QAction* cutNodeAction = menu.addAction(QString("Cut"));
   menu.addSeparator();
   QAction* exportImageAction = menu.addAction(QString("Save selected image"));
   menu.addSeparator();
   QAction* removeNodeAction = menu.addAction(QString("Remove node %1").arg(texNode->getName()));

   QAction* selectedAction = menu.exec(event->screenPos());
   if (removeNodeAction == selectedAction) {
      scene.getTextureProject().removeNode(id);
   } else if (copyNodeAction == selectedAction) {
      copyNodeToClipboard(scene.getTextureProject(), id);
   } else if (cutNodeAction == selectedAction) {
      cutNodeToClipboard(scene.getTextureProject(), id);
   } else if (exportImageAction == selectedAction) {
      scene.getMainWindow().saveImage(id);
   } else {
      for (int i = 0; i < texNode->getNumSourceSlots(); i++) {
         if (actions[i] == selectedAction) {
            texNode->setSourceSlot(i, 0);
         }
      }
   }
   event->accept();
}

void ViewNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (mouseEvent->button() == Qt::LeftButton) {
      if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
         scene.startLineDrawing(id);
      } else {
         if (!isSelected()) {
            scene.clearSelection();
            setSelected(true);
            scene.setSelectedNode(id);
         }
         mousePressed = true;
         mousePressedPos = mouseEvent->scenePos();
         mousePressedItemPos = texNode->getPos();
      }
   }
   mouseEvent->accept();
}

QVariant ViewNodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
   if (change == ItemSelectedChange && value.toInt() == 0) {
      scene.setSelectedNode(-1);
   } else if (change == ItemSelectedHasChanged) {
      updateConnectionLines();
   }
   return QGraphicsItem::itemChange(change, value);
}

void ViewNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
   if (mousePressed && QApplication::keyboardModifiers() != Qt::ControlModifier &&
       event->button() == Qt::LeftButton) {
      mousePressed = false;
   }
}

void ViewNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
   if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
      return;
   }
   if (mousePressed) {
      texNode->setPos(mousePressedItemPos + (mouseEvent->scenePos() - mousePressedPos));
   }
   QGraphicsItem::mouseMoveEvent(mouseEvent);
}

void ViewNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
   setCursor(Qt::ArrowCursor);
   QSetIterator<ViewNodeLine*> startLineIterator(startLines);
   while (startLineIterator.hasNext()) {
      startLineIterator.next()->setHighlighted(true);
   }
   QMapIterator<int, ViewNodeLine*> endLineIterator(endLines);
   while (endLineIterator.hasNext()) {
      endLineIterator.next().value()->setHighlighted(true);
   }
   QGraphicsItem::hoverEnterEvent(event);
}

void ViewNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
   QSetIterator<ViewNodeLine*> startLineIterator(startLines);
   while (startLineIterator.hasNext()) {
      startLineIterator.next()->setHighlighted(false);
   }
   QMapIterator<int, ViewNodeLine*> endLineIterator(endLines);
   while (endLineIterator.hasNext()) {
      endLineIterator.next().value()->setHighlighted(false);
   }
   QGraphicsItem::hoverLeaveEvent(event);
}

void ViewNodeItem::hoverMoveEvent(QGraphicsSceneHoverEvent*) {
   // Left empty
}

void ViewNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
   const QRectF cardRect(0, -headerSize, thumbnailSize.width() + borderWidth * 2,
                         thumbnailSize.height() + borderWidth * 2 + headerSize);
   const qreal imageTop = 0;
   const QRectF imageRect(cardRect.left(), imageTop, cardRect.width(),
                          cardRect.bottom() - imageTop);

   painter->setRenderHints(
       QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);

   painter->setPen(Qt::NoPen);
   painter->setBrush(QColor(0, 0, 0, 32));
   painter->drawRoundedRect(cardRect.translated(2, 3), 6, 6);

   QPen borderPen(isSelected() ? QColor("#4fc3f7") : QColor("#4d5563"));
   borderPen.setWidth(isSelected() ? 2 : 1);
   painter->setPen(Qt::NoPen);
   painter->setBrush(QColor("#eef0f3"));
   painter->drawRoundedRect(cardRect, 6, 6);

   if (headerSize > 0) {
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor("#252a33"));
      const QRectF titleRect(cardRect.left(), cardRect.top(), cardRect.width(), headerSize);
      qreal titleRadius = 6;
      if (titleRadius > headerSize / 2.0) {
         titleRadius = headerSize / 2.0;
      }
      QPainterPath titlePath;
      titlePath.moveTo(titleRect.left(), titleRect.bottom());
      titlePath.lineTo(titleRect.left(), titleRect.top() + titleRadius);
      titlePath.quadTo(titleRect.left(), titleRect.top(), titleRect.left() + titleRadius,
                       titleRect.top());
      titlePath.lineTo(titleRect.right() - titleRadius, titleRect.top());
      titlePath.quadTo(titleRect.right(), titleRect.top(), titleRect.right(),
                       titleRect.top() + titleRadius);
      titlePath.lineTo(titleRect.right(), titleRect.bottom());
      titlePath.closeSubpath();
      painter->drawPath(titlePath);

      QFont font;
      int titleFontSize = headerSize / 2 + 1;
#ifdef Q_OS_MAC
      // Graph node titles are painted independently of QApplication's font.
      titleFontSize = qRound(titleFontSize * 1.2);
      if (titleFontSize < 11) {
         titleFontSize = 11;
      } else if (titleFontSize > 22) {
         titleFontSize = 22;
      }
#else
      if (titleFontSize < 9) {
         titleFontSize = 9;
      } else if (titleFontSize > 20) {
         titleFontSize = 20;
      }
#endif
      font.setPixelSize(titleFontSize);
      font.setWeight(QFont::Medium);
      font.setStyleHint(QFont::StyleHint::Helvetica);
      font.setFamily(font.defaultFamily());
      painter->setFont(font);
      painter->setPen(QColor("#edf2f7"));
      painter->drawText(titleRect.adjusted(8, 0, -14, 0), Qt::AlignVCenter | Qt::AlignLeft,
                        titleString);
   }
   painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
   painter->setPen(Qt::NoPen);
   painter->save();
   QPainterPath imageClipPath;
   imageClipPath.addRoundedRect(cardRect, 6, 6);
   QPainterPath imageRectPath;
   imageRectPath.addRect(imageRect);
   imageClipPath = imageClipPath.intersected(imageRectPath);
   painter->setClipPath(imageClipPath);
   painter->drawPixmap(imageRect, pixmap,
                       QRectF(0, 0, thumbnailSize.width(), thumbnailSize.height()));

   if (!imageValid) {
      QBrush updateBrush(QColor(230, 126, 34, 180));
      updateBrush.setStyle(Qt::DiagCrossPattern);
      painter->setBrush(updateBrush);
      painter->drawRect(imageRect);
   }
   painter->restore();

   painter->setBrush(Qt::NoBrush);
   painter->setPen(borderPen);
   painter->drawRoundedRect(cardRect, 6, 6);

   if (isConnectable || isUnconnectable || isSelected()) {
      painter->setBrush(Qt::NoBrush);
      QPen selectedPen;
      if (isConnectable || isUnconnectable) {
         selectedPen.setColor(isConnectable ? QColor("#37d67a") : QColor("#ff5a5f"));
         selectedPen.setWidth(highlighterWidth);
      } else {
         selectedPen.setColor(QColor("#4fc3f7"));
         selectedPen.setWidth(3);
      }
      selectedPen.setJoinStyle(Qt::RoundJoin);
      painter->setPen(selectedPen);
      if (isUnconnectable) {
         painter->drawLine(0, 0, thumbnailSize.width() + borderWidth * 2,
                           thumbnailSize.height() + borderWidth * 2);
         painter->drawLine(thumbnailSize.width() + borderWidth * 2, 0, 0,
                           thumbnailSize.height() + borderWidth * 2);
      }
      painter->drawRoundedRect(cardRect.adjusted(-highlighterWidth / 2, -highlighterWidth / 2,
                                                 highlighterWidth / 2, highlighterWidth / 2),
                               7, 7);
   }
}
