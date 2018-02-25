/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QSlider>

/**
 * @brief The QDoubleSlider class
 *
 * Class derived from QSlider to support floating point values.
 * QSlider only supports integers so this has been modified to refer to the values
 * in 0,01 increments (100 times the value).
 * All signals and slots upscales or downscales it so that it can be used
 * interchangeably as a replacement for QSlider.
 */
class QDoubleSlider : public QSlider {
   Q_OBJECT

public:
   QDoubleSlider(Qt::Orientation orientation = Qt::Orientation::Horizontal, QWidget* parent = 0)
      : QSlider(orientation, parent) {
      QObject::connect(this, &QDoubleSlider::valueChanged,
                       this, &QDoubleSlider::notifyValueChanged);
   }

signals:
   void doubleValueChanged(double value);

public slots:
   void setDoubleMinimum(double value) {
      setMinimum(value * 100);
   }
   void setDoubleMaximum(double value) {
      setMaximum(value * 100);
   }
   void notifyValueChanged(int value) {
      emit doubleValueChanged((double) value / 100.0);
   }
   void setDoubleValue(double value) {
      setValue(value * 100);
   }
};

#endif // QDOUBLESLIDER_H
