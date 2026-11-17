// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/texturegenerator.h"
#include "base/executioncontroller.h"
#include <QElapsedTimer>
#include <stdexcept>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSet>
#include <cmath>

const TextureGeneratorSetting* findTextureGeneratorSetting(const TextureGeneratorSettings& settings,
                                                           const QString& id) {
   for (const TextureGeneratorSetting& setting : settings) {
      if (setting.id == id) {
         return &setting;
      }
   }
   return nullptr;
}

QString validateTextureGeneratorSettings(const TextureGeneratorSettings& settings) {
   QSet<QString> ids;
   for (const TextureGeneratorSetting& setting : settings) {
      if (setting.id.trimmed().isEmpty()) {
         return QStringLiteral("setting IDs must not be empty");
      }
      if (ids.contains(setting.id)) {
         return QStringLiteral("duplicate setting ID '%1'").arg(setting.id);
      }
      ids.insert(setting.id);
   }
   return QString();
}

QVariant migrateTextureSetting(const TextureGeneratorSetting& previous,
                               const TextureGeneratorSetting& current, const QVariant& value) {
   const int type = current.defaultvalue.typeId();
   if (previous.defaultvalue.typeId() != type || previous.multiline != current.multiline) {
      return {};
   }
   if (type == QMetaType::QStringList) {
      if (value.typeId() != QMetaType::QString) return {};
      const auto choices = current.defaultvalue.toStringList();
      return choices.contains(value.toString()) ? value
                                                : QVariant(choices.value(current.defaultindex));
   }
   if (value.typeId() != type) return {};
   if (type == QMetaType::Int) {
      int result = value.toInt();
      if (current.min.isValid()) result = qMax(result, current.min.toInt());
      if (current.max.isValid()) result = qMin(result, current.max.toInt());
      return result;
   }
   if (type == QMetaType::Double) {
      double result = value.toDouble();
      if (!std::isfinite(result)) return {};
      if (current.min.isValid()) result = qMax(result, current.min.toDouble());
      if (current.max.isValid()) result = qMin(result, current.max.toDouble());
      return result;
   }
   return value;
}

TextureNodeSettings migrateTextureSettings(const TextureGeneratorSettings& previous,
                                           const TextureNodeSettings& values,
                                           const TextureGeneratorSettings& current) {
   TextureNodeSettings result;
   for (const TextureGeneratorSetting& setting : current) {
      const auto* old = findTextureGeneratorSetting(previous, setting.id);
      QVariant value =
          old ? migrateTextureSetting(*old, setting, values.value(setting.id)) : QVariant();
      if (!value.isValid()) {
         value = setting.defaultvalue;
         if (value.typeId() == QMetaType::QStringList) {
            value = value.toStringList().value(setting.defaultindex);
         }
      }
      result.insert(setting.id, value);
   }
   return result;
}

void TextureGenerator::generateWithTiming(
    const QSize size, TexturePixel* destimage, const QMap<QString, TextureImagePtr>& sourceimages,
    const TextureNodeSettings& settings,
    const QSharedPointer<ExecutionController>& controller) const {
   QElapsedTimer timer;
   timer.start();
   try {
      if (controller && controller->isCancelled()) {
         throw std::runtime_error("Rendering cancelled");
      }
      generate(size, destimage, sourceimages, settings, controller);
   } catch (...) {
      recordGenerationTime(timer.nsecsElapsed());
      throw;
   }
   recordGenerationTime(timer.nsecsElapsed());
}

TextureGenerator::GenerationTiming TextureGenerator::getGenerationTiming() const {
   QMutexLocker lock(&generationTimesMutex);
   GenerationTiming timing;
   timing.runCount = generationTimesNanoseconds.size();
   if (timing.runCount == 0) {
      return timing;
   }

   qint64 totalNanoseconds = 0;
   for (const qint64 elapsedNanoseconds : generationTimesNanoseconds) {
      totalNanoseconds += elapsedNanoseconds;
   }
   timing.averageMilliseconds = static_cast<double>(totalNanoseconds) / timing.runCount / 1000000.0;
   return timing;
}

void TextureGenerator::recordGenerationTime(const qint64 elapsedNanoseconds) const {
   constexpr int maximumTimingSamples = 10;
   QMutexLocker lock(&generationTimesMutex);
   generationTimesNanoseconds.append(elapsedNanoseconds);
   if (generationTimesNanoseconds.size() > maximumTimingSamples) {
      generationTimesNanoseconds.removeFirst();
   }
}

QString TextureGenerator::resolveSourceSlot(const QString& serializedSlot) const {
   const QStringList sourceSlots = getSourceSlots();
   if (sourceSlots.contains(serializedSlot)) {
      return serializedSlot;
   }

   bool numericOk = false;
   const int numericSlot = serializedSlot.toInt(&numericOk);
   if (numericOk && numericSlot >= 0 && numericSlot < sourceSlots.size()) {
      return sourceSlots.at(numericSlot);
   }

   static const QRegularExpression legacyName(QStringLiteral("^Slot\\s+([1-9][0-9]*)$"),
                                              QRegularExpression::CaseInsensitiveOption);
   const QRegularExpressionMatch match = legacyName.match(serializedSlot);
   if (match.hasMatch()) {
      const int legacySlot = match.captured(1).toInt() - 1;
      if (legacySlot >= 0 && legacySlot < sourceSlots.size()) {
         return sourceSlots.at(legacySlot);
      }
   }
   return QString();
}
