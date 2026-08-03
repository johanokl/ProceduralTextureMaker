#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/jstexgen.h"
#include "base/jstexgenmanager.h"
#include "base/settingsmanager.h"
#include "generators/builtinregistry.h"
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <stdexcept>
#include <atomic>
#include <thread>
#include <utility>
#include <vector>

namespace {

/// @brief Returns a compact valid descriptor generator used by rendering tests.
QString solidScript() {
   return QStringLiteral(
       "const generator={apiVersion:1,name:'SolidJS',type:'generator',inputs:[],settings:{},"
       "generate(size,settings,output){void settings;for(let i=0;i<size.width*size.height;++i){"
       "const p=i*4;output.data[p]=0x11;output.data[p+1]=0x22;output.data[p+2]=0x33;"
       "output.data[p+3]=0x44;}}};");
}

/// @brief Renders a JavaScript generator and returns its first output pixel.
/// @param script JavaScript source defining the generator.
/// @return The color produced for the first pixel.
QColor renderColor(const QString& script) {
   auto generator = TextureGeneratorPtr(new JsTexGen(script));
   TextureProject project(false);
   project.addGenerator(generator);
   const TexturePixel pixel = project.newNode(1, generator)->renderImage(QSize(2, 2))->data()[0];
   return QColor(pixel.r, pixel.g, pixel.b, pixel.a);
}

/// @brief Creates a one-pixel source image.
TextureImagePtr sourceImage(const QColor& color) {
   TextureImagePtr image = TextureImage::create(QSize(1, 1));
   image->data()[0] =
       TexturePixel(static_cast<quint8>(color.red()), static_cast<quint8>(color.green()),
                    static_cast<quint8>(color.blue()), static_cast<quint8>(color.alpha()));
   return image;
}

bool writeTextFile(const QString& path, const QString& contents) {
   QFile file(path);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      return false;
   }
   return file.write(contents.toUtf8()) == contents.toUtf8().size();
}

/// @brief Runs a JavaScript generator directly with named source images.
QColor renderWithSources(const QString& script, QMap<QString, TextureImagePtr> sources) {
   JsTexGen generator(script);
   TexturePixel destination{};
   TextureNodeSettings settings;
   generator.generate(QSize(1, 1), &destination, sources, settings);
   return QColor(destination.r, destination.g, destination.b, destination.a);
}

}  // namespace

/// @brief Verifies JavaScript generator execution, diagnostics, and discovery.
class JavaScriptGeneratorsTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies RGBA rendering, script errors, and rejection of the removed globals API.
   void rendersAndReportsErrors();

   /// @brief Verifies loading valid scripts from a directory while reporting invalid ones.
   void loadsDirectory();

   /// @brief Verifies named descriptor inputs and rejects duplicate slots.
   void supportsNamedInputs();

   /// @brief Verifies the complete version-1 descriptor, settings, and in-place output contract.
   void supportsVersion1SchemaAndRendering();

   /// @brief Verifies descriptor rejection, type mapping, and source-aware diagnostics.
   void validatesVersion1Descriptors();

   /// @brief Verifies worker-local cache reuse, concurrent runtimes, and interruption.
   void cachesConcurrentRuntimesAndInterrupts();

   /// @brief Verifies explicit reload, compatible node migration, and broken-edit fallback.
   void reloadsDefinitionsAtomically();
};

void JavaScriptGeneratorsTest::rendersAndReportsErrors() {
   QCOMPARE(renderColor(solidScript()), QColor(0x11, 0x22, 0x33, 0x44));
   QVERIFY(!JsTexGen(QStringLiteral("var name=;")).isValid());

   const JsTexGen removedGlobalsApi(QStringLiteral(
       "var name='Legacy';function getSettings(){return {};}function generate(){return [];}"));
   QVERIFY(!removedGlobalsApi.isValid());
   QVERIFY(removedGlobalsApi.validationError().contains(QStringLiteral("globals API")));

   const QString throwing = QStringLiteral(
       "const generator={apiVersion:1,name:'Failure',type:'generator',inputs:[],settings:{},"
       "generate(){throw new Error('planned');}};");
   auto generator = TextureGeneratorPtr(new JsTexGen(throwing));
   TextureProject project(false);
   project.addGenerator(generator);
   const TextureNodePtr node = project.newNode(1, generator);
   QVERIFY_EXCEPTION_THROWN(static_cast<void>(node->renderImage(QSize(2, 2))), std::runtime_error);
}

void JavaScriptGeneratorsTest::loadsDirectory() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile file(directory.filePath(QStringLiteral("solid.js")));
   QVERIFY(file.open(QIODevice::WriteOnly));
   const QByteArray data = solidScript().toUtf8();
   QCOMPARE(file.write(data), qint64(data.size()));
   file.close();

   TextureProject project(false);
   QVERIFY(loadJavaScriptGenerators(project, directory.path()).isEmpty());
   const TextureGeneratorPtr loaded = project.getGenerator(QStringLiteral("SolidJS"));
   QVERIFY(!loaded.isNull());
   QCOMPARE(loaded->getOrigin(), TextureGenerator::Origin::Custom);
   QVERIFY(!loaded->getSourceIdentity().isEmpty());

   QFile invalid(directory.filePath(QStringLiteral("invalid.js")));
   QVERIFY(invalid.open(QIODevice::WriteOnly));
   invalid.write("const generator = { apiVersion: 1 };");
   invalid.close();
   TextureProject aggregateProject(false);
   const QString aggregateError = loadJavaScriptGenerators(aggregateProject, directory.path());
   QVERIFY(aggregateError.contains(QStringLiteral("invalid.js")));
   QVERIFY(!aggregateProject.getGenerator(QStringLiteral("SolidJS")).isNull());
   QVERIFY(
       !loadJavaScriptGenerators(project, directory.filePath(QStringLiteral("missing"))).isEmpty());
}

void JavaScriptGeneratorsTest::supportsNamedInputs() {
   const QString namedScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Named',type:'combiner',inputs:['Left','Right'],"
       "settings:{},generate(size,settings,output,inputs){void size;void settings;"
       "output.data.set(inputs.Right.data);}};");
   JsTexGen namedGenerator(namedScript);
   QVERIFY(namedGenerator.isValid());
   QCOMPARE(namedGenerator.getSourceSlots(),
            QStringList({QStringLiteral("Left"), QStringLiteral("Right")}));
   QCOMPARE(
       renderWithSources(namedScript, {{QStringLiteral("Left"), sourceImage(QColor(1, 2, 3, 4))},
                                       {QStringLiteral("Right"), sourceImage(QColor(9, 8, 7, 6))}}),
       QColor(9, 8, 7, 6));

   const QString duplicateScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Duplicate',type:'combiner',"
       "inputs:['Input','Input'],settings:{},generate(){}};");
   QVERIFY(!JsTexGen(duplicateScript).isValid());
}

void JavaScriptGeneratorsTest::supportsVersion1SchemaAndRendering() {
   const QString script = QStringLiteral(R"JS(
const generator = {
  apiVersion: 1,
  name: "SchemaV1",
  description: "All setting types",
  type: "generator",
  inputs: [],
  settings: {
    imagewidth: { type: "integer", name: "Image width setting", default: 7, min: 0, max: 20, order: 1 },
    amount: { type: "real", default: 0.5, min: 0, max: 1, order: 2 },
    enabled: { type: "boolean", default: true, group: "Behavior", order: 3 },
    label: { type: "string", default: "hello", description: "A label", order: 4 },
    notes: { type: "multiline", default: "two\nlines", enabler: "enabled", order: 5 },
    color: { type: "color", default: { r: 9, g: 8, b: 7, a: 6 }, order: 6 },
    mode: { type: "choice", values: ["Add", "Multiply", "Overlay"], default: "Overlay", order: 7 }
  },
  generate(size, settings, output, inputs) {
    if (!Object.isFrozen(size) || !Object.isFrozen(settings) || !Object.isFrozen(output) ||
        !Object.isFrozen(inputs)) throw new Error("arguments must be frozen");
    if (output.format !== "rgba8") throw new Error("unexpected pixel format");
    if (settings.imagewidth !== 7 || size.width !== 2) throw new Error("size/settings collision");
    for (let index = 0; index < output.data.length; index += 4) {
      output.data[index] = settings.color.r;
      output.data[index + 1] = settings.color.g;
      output.data[index + 2] = settings.color.b;
      output.data[index + 3] = settings.color.a;
    }
  }
};
)JS");
   auto* rawGenerator = new JsTexGen(script, QStringLiteral("schema-v1.js"));
   QVERIFY2(rawGenerator->isValid(), qPrintable(rawGenerator->validationError()));
   QCOMPARE(rawGenerator->apiVersion(), 1);
   QCOMPARE(rawGenerator->getDescription(), QStringLiteral("All setting types"));
   QCOMPARE(rawGenerator->getSourceSlots(), QStringList());
   const TextureGeneratorSettings& settings = rawGenerator->getSettings();
   QCOMPARE(settings.size(), 7);
   QCOMPARE(settings.value(QStringLiteral("imagewidth")).defaultvalue, QVariant(7));
   QCOMPARE(settings.value(QStringLiteral("amount")).defaultvalue, QVariant(0.5));
   QCOMPARE(settings.value(QStringLiteral("enabled")).defaultvalue, QVariant(true));
   QCOMPARE(settings.value(QStringLiteral("label")).defaultvalue,
            QVariant(QStringLiteral("hello")));
   QVERIFY(settings.value(QStringLiteral("notes")).multiline);
   QCOMPARE(settings.value(QStringLiteral("notes")).enabler, QStringLiteral("enabled"));
   QCOMPARE(settings.value(QStringLiteral("color")).defaultvalue.value<QColor>(),
            QColor(9, 8, 7, 6));
   QCOMPARE(
       settings.value(QStringLiteral("mode")).defaultvalue.toStringList(),
       QStringList({QStringLiteral("Add"), QStringLiteral("Multiply"), QStringLiteral("Overlay")}));
   QCOMPARE(settings.value(QStringLiteral("mode")).defaultindex, 2);

   const TextureGeneratorPtr generator(rawGenerator);
   TextureProject project(false);
   project.addGenerator(generator);
   const TextureImagePtr image = project.newNode(1, generator)->renderImage(QSize(2, 2));
   const TexturePixel pixel = image->data()[0];
   QCOMPARE(QColor(pixel.r, pixel.g, pixel.b, pixel.a), QColor(9, 8, 7, 6));

   const QString immutableInputScript = QStringLiteral(R"JS(
const generator = {
  apiVersion: 1, name: "Immutable", type: "filter", inputs: ["Input"], settings: {},
  generate(size, settings, output, inputs) {
    void size; void settings;
    const original = inputs.Input.data[0];
    inputs.Input.data[0] = 0;
    output.data.set(inputs.Input.data);
    output.data[0] = original;
  }
};
)JS");
   TextureImagePtr source = sourceImage(QColor(3, 4, 5, 6));
   QCOMPARE(renderWithSources(immutableInputScript, {{QStringLiteral("Input"), source}}),
            QColor(3, 4, 5, 6));
   QCOMPARE(
       QColor(source->data()[0].r, source->data()[0].g, source->data()[0].b, source->data()[0].a),
       QColor(3, 4, 5, 6));
}

void JavaScriptGeneratorsTest::validatesVersion1Descriptors() {
   const QString typeScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Typed',type:'%1',inputs:[],settings:{},"
       "generate(size,settings,output){output.data.fill(0);}};");
   const QList<QPair<QString, TextureGenerator::Type>> types{
       {QStringLiteral("generator"), TextureGenerator::Type::Generator},
       {QStringLiteral("filter"), TextureGenerator::Type::Filter},
       {QStringLiteral("combiner"), TextureGenerator::Type::Combiner}};
   for (const auto& type : types) {
      JsTexGen generator(typeScript.arg(type.first), QStringLiteral("typed.js"));
      QVERIFY2(generator.isValid(), qPrintable(generator.validationError()));
      QCOMPARE(generator.getType(), type.second);
   }

   const QStringList invalidScripts{
       QStringLiteral(
           "const generator={apiVersion:2,name:'UnreleasedVersion',type:'generator',inputs:[],"
           "settings:{},generate(){}};"),
       QStringLiteral(
           "const generator={apiVersion:1,name:'NoType',inputs:[],settings:{},generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'BadType',type:'effect',inputs:[],settings:{},"
                      "generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Slots',type:'filter',inputs:['Input','Input'],"
                      "settings:{},generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Range',type:'generator',inputs:[],settings:{"
                      "value:{type:'integer',default:20,min:0,max:10}},generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Choice',type:'generator',inputs:[],settings:{"
                      "mode:{type:'choice',values:['A'],default:'B'}},generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Callable',type:'generator',inputs:[],settings:"
                      "{},generate:3};")};
   for (const QString& script : invalidScripts) {
      JsTexGen generator(script, QStringLiteral("invalid-v1.js"));
      QVERIFY(!generator.isValid());
      QVERIFY2(generator.validationError().contains(QStringLiteral("invalid-v1.js")),
               qPrintable(generator.validationError()));
   }

   const QString throwing = QStringLiteral(
       "const generator={apiVersion:1,name:'Throwing',type:'generator',inputs:[],settings:{},"
       "generate(){throw new Error('planned v1 failure');}};");
   JsTexGen generator(throwing, QStringLiteral("throwing-v1.js"));
   TexturePixel destination{};
   QVERIFY_EXCEPTION_THROWN(
       generator.generate(QSize(1, 1), &destination, {}, TextureNodeSettings()),
       std::runtime_error);
}

void JavaScriptGeneratorsTest::cachesConcurrentRuntimesAndInterrupts() {
   const QString fillScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Cached',type:'generator',inputs:[],settings:{},"
       "generate(size,settings,output){void size;void settings;output.data.fill(17);}};");
   JsTexGen generator(fillScript, QStringLiteral("cached.js"));
   TextureNodeSettings settings;
   TexturePixel destination{};
   const quint64 before = JsTexGen::runtimeEvaluationCount();
   generator.generate(QSize(1, 1), &destination, {}, settings);
   generator.generate(QSize(1, 1), &destination, {}, settings);
   QCOMPARE(JsTexGen::runtimeEvaluationCount(), before + 1);

   const quint64 concurrentBefore = JsTexGen::runtimeEvaluationCount();
   std::vector<std::thread> workers;
   std::atomic_int completed{0};
   for (int index = 0; index < 2; ++index) {
      workers.emplace_back([&generator, &completed]() {
         TexturePixel pixel{};
         generator.generate(QSize(1, 1), &pixel, {}, TextureNodeSettings());
         if (pixel.b == 17 && pixel.g == 17 && pixel.r == 17 && pixel.a == 17) {
            ++completed;
         }
      });
   }
   for (std::thread& worker : workers) {
      worker.join();
   }
   QCOMPARE(completed.load(), 2);
   QCOMPARE(JsTexGen::runtimeEvaluationCount(), concurrentBefore + 2);

   const QString infiniteScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Infinite',type:'generator',inputs:[],settings:{},"
       "generate(){while(true){}}};");
   JsTexGen infinite(infiniteScript, QStringLiteral("infinite.js"));
   std::atomic_bool finished{false};
   std::atomic_bool interrupted{false};
   std::thread renderThread([&]() {
      try {
         TexturePixel pixel{};
         infinite.generate(QSize(1, 1), &pixel, {}, TextureNodeSettings());
      } catch (const std::runtime_error& error) {
         interrupted = QString::fromUtf8(error.what())
                           .contains(QStringLiteral("interrupted"), Qt::CaseInsensitive);
      }
      finished = true;
   });
   for (int attempt = 0; attempt < 100 && !finished.load(); ++attempt) {
      QTest::qWait(10);
      JsTexGen::interruptActiveEngines();
   }
   renderThread.join();
   QVERIFY(finished.load());
   QVERIFY(interrupted.load());
}

void JavaScriptGeneratorsTest::reloadsDefinitionsAtomically() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString path = directory.filePath(QStringLiteral("reload.js"));
   const QString firstRevision = QStringLiteral(R"JS(
const generator = {
  apiVersion: 1, name: "Reloaded", type: "filter", inputs: ["Source"],
  settings: { amount: { type: "integer", default: 2, min: 0, max: 20 } },
  generate(size, settings, output, inputs) {
    void size; output.data.fill(settings.amount);
    if (inputs.Source) output.data[3] = inputs.Source.data[3];
  }
};
)JS");
   QVERIFY(writeTextFile(path, firstRevision));

   TextureProject project(false);
   SettingsManager settingsManager;
   project.setSettingsManager(&settingsManager);
   settingsManager.setJSTextureGeneratorsPath(directory.path());
   settingsManager.setJSTextureGeneratorsEnabled(true);
   JsTexGenManager manager(&project);
   QSignalSpy reloadSpy(&manager, &JsTexGenManager::reloadFinished);
   QVERIFY(reloadSpy.wait(5000));

   TextureGeneratorPtr firstGenerator = project.getGenerator(QStringLiteral("Reloaded"));
   QVERIFY(!firstGenerator.isNull());
   QCOMPARE(firstGenerator->getOrigin(), TextureGenerator::Origin::Custom);
   const TextureGeneratorPtr sourceGenerator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'ReloadSource',type:'generator',inputs:[],settings:{},"
       "generate(size,settings,output){void size;void settings;output.data.fill(255);}};")));
   project.addGenerator(sourceGenerator);
   const TextureNodePtr sourceNode = project.newNode(1, sourceGenerator);
   const TextureNodePtr reloadedNode = project.newNode(2, firstGenerator);
   TextureNodeSettings nodeSettings = reloadedNode->getSettings();
   nodeSettings.insert(QStringLiteral("amount"), 9);
   reloadedNode->setSettings(nodeSettings);
   QVERIFY(reloadedNode->setSourceSlot(QStringLiteral("Source"), sourceNode->getId()));

   const QString secondRevision = QStringLiteral(R"JS(
const generator = {
  apiVersion: 1, name: "Reloaded", type: "combiner", inputs: ["Source", "Mask"],
  settings: {
    amount: { type: "integer", default: 3, min: 0, max: 20 },
    enabled: { type: "boolean", default: true }
  },
  generate(size, settings, output, inputs) {
    void size; void inputs; output.data.fill(settings.enabled ? settings.amount : 0);
  }
};
)JS");
   QVERIFY(writeTextFile(path, secondRevision));
   reloadSpy.clear();
   manager.reload();
   QVERIFY(reloadSpy.wait(5000));
   const QStringList reloadDiagnostics = reloadSpy.last().at(0).toStringList();
   QVERIFY2(reloadDiagnostics.isEmpty(), qPrintable(reloadDiagnostics.join(QLatin1Char('\n'))));

   const TextureGeneratorPtr secondGenerator = project.getGenerator(QStringLiteral("Reloaded"));
   QVERIFY(!secondGenerator.isNull());
   QVERIFY(secondGenerator != firstGenerator);
   QCOMPARE(secondGenerator->getType(), TextureGenerator::Type::Combiner);
   QCOMPARE(reloadedNode->getGenerator(), secondGenerator);
   QCOMPARE(reloadedNode->getSettings().value(QStringLiteral("amount")).toInt(), 9);
   QCOMPARE(reloadedNode->getSettings().value(QStringLiteral("enabled")).toBool(), true);
   QCOMPARE(reloadedNode->getSources().value(QStringLiteral("Source")), sourceNode->getId());
   QCOMPARE(reloadedNode->getSources().value(QStringLiteral("Mask")), 0);

   QVERIFY(writeTextFile(path, QStringLiteral("const generator = {")));
   reloadSpy.clear();
   manager.reload();
   QVERIFY(reloadSpy.wait(5000));
   QCOMPARE(project.getGenerator(QStringLiteral("Reloaded")), secondGenerator);
}

QTEST_GUILESS_MAIN(JavaScriptGeneratorsTest)
#include "javascript_generators_test.moc"
