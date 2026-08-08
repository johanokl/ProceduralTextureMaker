#include "testgenerators.h"
#include <stdexcept>
#include <utility>

RecordingGenerator::RecordingGenerator(QString name, const int sourceSlots, const int defaultValue)
    : generatorName(std::move(name)) {
   if (sourceSlots == 1) {
      this->sourceSlots.append(QStringLiteral("Input"));
   } else {
      for (int i = 1; i <= sourceSlots; ++i) {
         this->sourceSlots.append(QStringLiteral("Input %1").arg(i));
      }
   }
   TextureGeneratorSetting value;
   value.id = QStringLiteral("value");
   value.name = QStringLiteral("Value");
   value.defaultvalue = defaultValue;
   schema.append(value);
}

void RecordingGenerator::generate(const QSize size, TexturePixel* destination,
                                  const QMap<QString, TextureImagePtr>& sources,
                                  const TextureNodeSettings& settings) const {
   Q_UNUSED(sources);
   ++calls;
   started.release();
   if (blocking.exchange(false)) {
      gate.acquire();
   }
   if (throwing) {
      throw std::runtime_error("planned generator failure");
   }
   const int value = settings.value(QStringLiteral("value"), 17).toInt();
   const auto channel = static_cast<unsigned char>(qBound(0, value, 255));
   const qsizetype count = static_cast<qsizetype>(size.width()) * size.height();
   for (qsizetype i = 0; i < count; ++i) {
      destination[i] = TexturePixel(channel, channel, channel, 255);
   }
}

bool RecordingGenerator::waitUntilStarted(const int timeoutMilliseconds) const {
   return started.tryAcquire(1, timeoutMilliseconds);
}

void NamedInputGenerator::generate(const QSize size, TexturePixel* destination,
                                   const QMap<QString, TextureImagePtr>& sources,
                                   const TextureNodeSettings& settings) const {
   Q_UNUSED(settings);
   const TextureImagePtr source = sources.value(QStringLiteral("Zulu"));
   const qsizetype count = static_cast<qsizetype>(size.width()) * size.height();
   for (qsizetype i = 0; i < count; ++i) {
      destination[i] = source.isNull() ? TexturePixel(0, 0, 0, 0) : source->data()[i];
   }
}
