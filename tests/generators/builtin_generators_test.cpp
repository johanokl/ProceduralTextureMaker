#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/builtinregistry.h"
#include <QSet>
#include <QTest>

/// @brief Exercises every registered built-in generator with a small render.
class BuiltinGeneratorsTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies that every built-in generator can render without failing.
   void rendersEveryGenerator();
};

void BuiltinGeneratorsTest::rendersEveryGenerator() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto generators = project.getGenerators();
   QCOMPARE(generators.size(), 35);

   for (auto it = generators.cbegin(); it != generators.cend(); ++it) {
      const TextureGeneratorPtr& generator = it.value();
      QVERIFY2(!generator->getDescription().trimmed().isEmpty(), qPrintable(it.key()));
      const QStringList sourceSlots = generator->getSourceSlots();
      QSet<QString> uniqueSlots;
      for (const QString& slot : sourceSlots) {
         QVERIFY2(!slot.isEmpty(), qPrintable(it.key()));
         QVERIFY2(!uniqueSlots.contains(slot), qPrintable(it.key()));
         uniqueSlots.insert(slot);
      }
      QSet<QString> uniqueSettingIds;
      for (const TextureGeneratorSetting& setting : generator->getSettings()) {
         QVERIFY2(!setting.id.trimmed().isEmpty(), qPrintable(it.key()));
         QVERIFY2(!uniqueSettingIds.contains(setting.id), qPrintable(it.key()));
         QVERIFY2(!setting.name.trimmed().isEmpty(),
                  qPrintable(QStringLiteral("%1: %2 has no name").arg(it.key(), setting.id)));
         QVERIFY2(
             !setting.description.trimmed().isEmpty(),
             qPrintable(QStringLiteral("%1: %2 has no description").arg(it.key(), setting.id)));
         uniqueSettingIds.insert(setting.id);
      }
      QVERIFY2(validateTextureGeneratorSettings(generator->getSettings()).isEmpty(),
               qPrintable(it.key()));
      if (sourceSlots.size() == 1) {
         QCOMPARE(sourceSlots.first(), QStringLiteral("Input"));
      }
      const TextureNodePtr source =
          project.newNode(1, project.getGenerator(QStringLiteral("Fill")));
      const TextureNodePtr output = project.newNode(100, generator);
      for (const QString& slot : generator->getSourceSlots()) {
         QVERIFY2(output->setSourceSlot(slot, source->getId()), qPrintable(it.key()));
      }
      const TextureImagePtr image = output->renderImage(QSize(17, 13));
      QVERIFY2(!image.isNull(), qPrintable(it.key()));
      QCOMPARE(image->getSize(), QSize(17, 13));
      project.clear();
   }

   QCOMPARE(project.getGenerator(QStringLiteral("Fill"))->getSourceSlots(), QStringList());
   QCOMPARE(project.getGenerator(QStringLiteral("Blending"))->getSourceSlots(),
            QStringList({QStringLiteral("Base"), QStringLiteral("Blend")}));
   QCOMPARE(project.getGenerator(QStringLiteral("Cutout"))->getSourceSlots(),
            QStringList({QStringLiteral("Image"), QStringLiteral("Mask")}));
   QCOMPARE(project.getGenerator(QStringLiteral("Displacement"))->getSourceSlots(),
            QStringList({QStringLiteral("Source image"), QStringLiteral("Map")}));
   QCOMPARE(project.getGenerator(QStringLiteral("Set channels"))->getSourceSlots(),
            QStringList({QStringLiteral("First"), QStringLiteral("Second")}));
   QCOMPARE(project.getGenerator(QStringLiteral("Merge"))->getSourceSlots().size(), 10);
   const TextureGeneratorSettings& transformSettings =
       project.getGenerator(QStringLiteral("Transform"))->getSettings();
   QCOMPARE(transformSettings.at(0).id, QStringLiteral("xscale"));
   QCOMPARE(transformSettings.at(1).id, QStringLiteral("yscale"));
   QCOMPARE(transformSettings.at(2).id, QStringLiteral("rotation"));
   const TextureGeneratorPtr mask = project.getGenerator(QStringLiteral("Mask"));
   QVERIFY(!mask.isNull());
   QCOMPARE(mask->getType(), TextureGenerator::Type::Combiner);
   QCOMPARE(mask->getOrigin(), TextureGenerator::Origin::BuiltIn);
   QCOMPARE(mask->getSourceIdentity(), QStringLiteral(":/generators/mask.js"));
}

QTEST_MAIN(BuiltinGeneratorsTest)
#include "builtin_generators_test.moc"
