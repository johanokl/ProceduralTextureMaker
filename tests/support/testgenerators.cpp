#include "testgenerators.h"
#include <stdexcept>
#include <utility>

RecordingGenerator::RecordingGenerator(QString name, const int sourceSlots, const int defaultValue)
    : generatorName(std::move(name)), sourceSlotCount(sourceSlots) {
   TextureGeneratorSetting value;
   value.name = QStringLiteral("Value");
   value.defaultvalue = defaultValue;
   schema.insert(QStringLiteral("value"), value);
}

void RecordingGenerator::generate(const QSize size, TexturePixel* destination,
                                  QMap<int, TextureImagePtr> sources,
                                  TextureNodeSettings* settings) const {
   Q_UNUSED(sources);
   ++calls;
   started.release();
   if (blocking.exchange(false)) {
      gate.acquire();
   }
   if (throwing) {
      throw std::runtime_error("planned generator failure");
   }
   const int value = settings ? settings->value(QStringLiteral("value"), 17).toInt() : 17;
   const auto channel = static_cast<unsigned char>(qBound(0, value, 255));
   const qsizetype count = static_cast<qsizetype>(size.width()) * size.height();
   for (qsizetype i = 0; i < count; ++i) {
      destination[i] = TexturePixel(channel, channel, channel, 255);
   }
}

bool RecordingGenerator::waitUntilStarted(const int timeoutMilliseconds) const {
   return started.tryAcquire(1, timeoutMilliseconds);
}
