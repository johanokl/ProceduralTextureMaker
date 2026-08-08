#include "base/projectfileservice.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "support/testgenerators.h"
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <utility>

namespace {

class SchemaGenerator final : public TextureGenerator {
public:
   SchemaGenerator(QString name, TextureGeneratorSettings settings)
       : name(std::move(name)), settings(std::move(settings)) {}

   void generate(QSize size, TexturePixel* destination,
                 const QMap<QString, TextureImagePtr>& sources,
                 const TextureNodeSettings& nodeSettings) const override {
      Q_UNUSED(sources);
      Q_UNUSED(nodeSettings);
      std::fill_n(destination, static_cast<qsizetype>(size.width()) * size.height(),
                  TexturePixel(0, 0, 0, 255));
   }
   const TextureGeneratorSettings& getSettings() const override { return settings; }
   Type getType() const override { return Type::Generator; }
   QStringList getSourceSlots() const override { return {}; }
   QString getName() const override { return name; }
   QString getDescription() const override { return {}; }

private:
   QString name;
   TextureGeneratorSettings settings;
};

TextureGeneratorSetting integerSetting(const QString& id, const int defaultValue) {
   TextureGeneratorSetting setting;
   setting.id = id;
   setting.name = id;
   setting.defaultvalue = defaultValue;
   return setting;
}

}  // namespace

/// @brief Verifies graph ownership, rendering caches, copying, and saved state.
class TextureProjectTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies node identifiers, connections, disconnections, and removal.
   void maintainsGraphAndIds();
   /// @brief Verifies synchronous rendering caches and downstream invalidation.
   void cachesAndInvalidatesRenders();
   /// @brief Verifies clipboard-style copies and project saved-state tracking.
   void copiesAndTracksSavedState();
   /// @brief Verifies named render inputs are routed independently of alphabetical order.
   void routesNamedRenderInputs();
   /// @brief Verifies setting ID validation and ordered generator metadata serialization.
   void validatesAndSerializesSettingSchemas();
};

void TextureProjectTest::maintainsGraphAndIds() {
   TextureProject project(false);
   auto generator = TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Slots"), 2));
   project.addGenerator(generator);
   project.addGenerator(TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Slots"), 2)));
   QCOMPARE(project.getGenerator(QStringLiteral("Slots")), generator);

   const TextureNodePtr one = project.newNode(0, generator);
   const TextureNodePtr explicitNode = project.newNode(7, generator);
   const TextureNodePtr two = project.newNode(0, generator);
   QCOMPARE(project.getNodeIds(), QList<int>({1, 2, 7}));
   QCOMPARE(project.newNode(7, generator), explicitNode);

   QVERIFY(two->setSourceSlot(QStringLiteral("Input 1"), one->getId()));
   QVERIFY(two->setSourceSlot(QStringLiteral("Input 2"), one->getId()));
   QCOMPARE(one->getNumReceivers(), 1);
   QVERIFY(explicitNode->setSourceSlot(QStringLiteral("Input 1"), two->getId()));
   QVERIFY(!one->setSourceSlot(QStringLiteral("Input 1"), explicitNode->getId()));
   QVERIFY(!two->setSourceSlot(QStringLiteral("Missing"), one->getId()));
   QVERIFY(!two->setSourceSlot(QStringLiteral("Input 1"), 999));
   QVERIFY(!project.findLoops());

   project.removeNode(two->getId());
   QCOMPARE(one->getNumReceivers(), 0);
   QCOMPARE(explicitNode->getSources().value(QStringLiteral("Input 1")), 0);
   project.clear();
   QCOMPARE(project.newNode(0, generator)->getId(), 1);
}

void TextureProjectTest::cachesAndInvalidatesRenders() {
   TextureProject project(false);
   auto* sourceGenerator = new RecordingGenerator(QStringLiteral("Source"), 0, 25);
   auto* filterGenerator = new RecordingGenerator(QStringLiteral("Filter"), 1, 50);
   project.addGenerator(TextureGeneratorPtr(sourceGenerator));
   project.addGenerator(TextureGeneratorPtr(filterGenerator));
   const TextureNodePtr source = project.newNode(1, project.getGenerator(QStringLiteral("Source")));
   const TextureNodePtr output = project.newNode(2, project.getGenerator(QStringLiteral("Filter")));
   const TextureNodePtr unrelated =
       project.newNode(3, project.getGenerator(QStringLiteral("Source")));
   QVERIFY(output->setSourceSlot(QStringLiteral("Input"), source->getId()));

   const QSize size(5, 3);
   const TextureImagePtr first = output->renderImage(size);
   QCOMPARE(output->renderImage(size), first);
   QCOMPARE(sourceGenerator->callCount(), 1);
   QCOMPARE(filterGenerator->callCount(), 1);
   const TextureImagePtr unrelatedImage = unrelated->renderImage(size);

   TextureNodeSettings settings = source->getSettings();
   settings[QStringLiteral("value")] = 91;
   source->setSettings(settings);
   QVERIFY(source->cachedImage(size).isNull());
   QVERIFY(output->cachedImage(size).isNull());
   QCOMPARE(unrelated->cachedImage(size), unrelatedImage);
   QVERIFY(output->renderImage(size) != first);
}

void TextureProjectTest::copiesAndTracksSavedState() {
   TextureProject project(false);
   auto generator = TextureGeneratorPtr(new RecordingGenerator(QStringLiteral("Clone")));
   project.addGenerator(generator);
   const TextureNodePtr node = project.newNode(1, generator);
   node->setName(QStringLiteral("Copied node"));
   node->setPos(QPointF(3.5, 8.0));

   QCOMPARE(project.pasteNodes(project.serializeNode(1)), 1);
   QCOMPARE(project.getNumNodes(), 2);
   QCOMPARE(project.getNode(2)->getName(), QStringLiteral("Copied node"));
   QCOMPARE(project.pasteNodes(QStringLiteral("not xml")), 0);

   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("project.txl"));
   QVERIFY(ProjectFileService::save(path, project, false));
   QVERIFY(!project.isModified());
   node->setName(QStringLiteral("Renamed"));
   QCOMPARE(ProjectFileService::save(path, project, false).error, ProjectFileError::OutputExists);
   QVERIFY(project.isModified());
   QVERIFY(ProjectFileService::save(path, project, true));
   QVERIFY(!project.isModified());
}

void TextureProjectTest::routesNamedRenderInputs() {
   TextureProject project(false);
   const TextureGeneratorPtr firstGenerator(new RecordingGenerator(QStringLiteral("First"), 0, 23));
   const TextureGeneratorPtr secondGenerator(
       new RecordingGenerator(QStringLiteral("Second"), 0, 91));
   const TextureGeneratorPtr positionalGenerator(
       new RecordingGenerator(QStringLiteral("Positional"), 2));
   const TextureGeneratorPtr namedGenerator(new NamedInputGenerator);
   const TextureNodePtr first = project.newNode(1, firstGenerator);
   const TextureNodePtr second = project.newNode(2, secondGenerator);
   const TextureNodePtr output = project.newNode(3, positionalGenerator);
   QVERIFY(output->setSourceSlot(QStringLiteral("Input 1"), first->getId()));
   QVERIFY(output->setSourceSlot(QStringLiteral("Input 2"), second->getId()));
   QVERIFY(output->setGenerator(namedGenerator));
   QCOMPARE(output->getSources().value(QStringLiteral("Zulu")), first->getId());
   QCOMPARE(output->getSources().value(QStringLiteral("Alpha")), second->getId());
   QCOMPARE(output->renderImage(QSize(2, 2))->data()[0].r, static_cast<unsigned char>(23));
}

void TextureProjectTest::validatesAndSerializesSettingSchemas() {
   TextureProject project(false);
   const TextureGeneratorSettings orderedSettings{integerSetting(QStringLiteral("zeta"), 1),
                                                  integerSetting(QStringLiteral("alpha"), 2)};
   const TextureGeneratorPtr valid(
       new SchemaGenerator(QStringLiteral("Ordered schema"), orderedSettings));
   project.addGenerator(valid);
   QCOMPARE(project.getGenerator(QStringLiteral("Ordered schema")), valid);
   const TextureNodePtr node = project.newNode(1, valid);

   const TextureGeneratorSettings duplicateSettings{integerSetting(QStringLiteral("duplicate"), 1),
                                                    integerSetting(QStringLiteral("duplicate"), 2)};
   const TextureGeneratorPtr invalidAdd(
       new SchemaGenerator(QStringLiteral("Invalid schema"), duplicateSettings));
   project.addGenerator(invalidAdd);
   QVERIFY(project.getGenerator(QStringLiteral("Invalid schema")).isNull());

   const TextureGeneratorPtr emptyId(new SchemaGenerator(QStringLiteral("Empty setting ID"),
                                                         {integerSetting(QStringLiteral(" "), 1)}));
   project.addGenerator(emptyId);
   QVERIFY(project.getGenerator(QStringLiteral("Empty setting ID")).isNull());

   const TextureGeneratorPtr invalidReplacement(
       new SchemaGenerator(QStringLiteral("Ordered schema"), duplicateSettings));
   QVERIFY(!project.replaceGenerator(valid, invalidReplacement));
   QCOMPARE(project.getGenerator(QStringLiteral("Ordered schema")), valid);
   QCOMPARE(node->getGenerator(), valid);

   const QDomNodeList definitions = project.saveAsXML(true).elementsByTagName("generatorsetting");
   QCOMPARE(definitions.size(), 2);
   QCOMPARE(definitions.at(0).toElement().attribute(QStringLiteral("id")), QStringLiteral("zeta"));
   QCOMPARE(definitions.at(1).toElement().attribute(QStringLiteral("id")), QStringLiteral("alpha"));
}

QTEST_APPLESS_MAIN(TextureProjectTest)
#include "textureproject_test.moc"
