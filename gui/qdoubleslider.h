
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QSlider>

/// @brief Class derived from QSlider to support floating point values.
/// QSlider only supports integers, so values are stored in increments of 0.01
/// by scaling them by 100. Signals and slots convert between the stored integer
/// and exposed floating-point values.
class QDoubleSlider : public QSlider {
   Q_OBJECT

public:
   /// @brief Creates a slider that exposes values in increments of one hundredth.
   /// @param orientation Slider orientation.
   /// @param parent Parent widget.
   explicit QDoubleSlider(Qt::Orientation orientation = Qt::Orientation::Horizontal,
                          QWidget* parent = nullptr)
       : QSlider(orientation, parent) {
      QObject::connect(this, &QDoubleSlider::valueChanged, this,
                       &QDoubleSlider::notifyValueChanged);
   }

signals:
   /// @brief Emitted when the scaled floating-point value changes.
   /// @param value Current floating-point value.
   void doubleValueChanged(double value);

public slots:
   /// @brief Sets the minimum floating-point value.
   /// @param value Minimum value.
   void setDoubleMinimum(double value) { setMinimum(value * 100); }

   /// @brief Sets the maximum floating-point value.
   /// @param value Maximum value.
   void setDoubleMaximum(double value) { setMaximum(value * 100); }

   /// @brief Converts an integer slider change into a floating-point signal.
   /// @param value Current scaled integer value.
   void notifyValueChanged(int value) { emit doubleValueChanged((double)value / 100.0); }

   /// @brief Sets the current floating-point value.
   /// @param value New value.
   void setDoubleValue(double value) { setValue(value * 100); }
};

#endif  // QDOUBLESLIDER_H
