// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/texturegenerator.h"
#include <QRegularExpression>
#include <QSet>

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
