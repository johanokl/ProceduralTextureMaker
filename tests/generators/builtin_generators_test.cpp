#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/builtinregistry.h"
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
   QCOMPARE(generators.size(), 34);

   for (auto it = generators.cbegin(); it != generators.cend(); ++it) {
      const TextureGeneratorPtr& generator = it.value();
      const TextureNodePtr source =
          project.newNode(1, project.getGenerator(QStringLiteral("Fill")));
      const TextureNodePtr output = project.newNode(100, generator);
      for (int slot = 0; slot < generator->getNumSourceSlots(); ++slot) {
         QVERIFY2(output->setSourceSlot(slot, source->getId()), qPrintable(it.key()));
      }
      const TextureImagePtr image = output->renderImage(QSize(17, 13));
      QVERIFY2(!image.isNull(), qPrintable(it.key()));
      QCOMPARE(image->getSize(), QSize(17, 13));
      project.clear();
   }
}

QTEST_MAIN(BuiltinGeneratorsTest)
#include "builtin_generators_test.moc"
