#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/jstexgen.h"
#include "base/executioncontroller.h"
#include <QJSEngine>
#include "base/jstexgenmanager.h"
#include "base/settingsmanager.h"
#include "generators/builtinregistry.h"
#include <QFile>
#include <QElapsedTimer>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>
#include <stdexcept>
#include <atomic>
#include <thread>
#include <utility>
#include <vector>

namespace {

/// @brief Returns a compact valid descriptor generator used by rendering tests.
QString solidScript() {
   return QStringLiteral(
       "const generator={apiVersion:1,name:'SolidJS',type:'generator',inputs:[],settings:[],"
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

/// @brief Renders a generator with its defaults plus selected setting overrides.
/// @param generator Generator to execute.
/// @param size Requested output size.
/// @param sources Named source images supplied to the generator.
/// @param overrides Setting values replacing descriptor defaults.
/// @return The rendered texture image.
TextureImagePtr renderGenerator(const TextureGeneratorPtr& generator, const QSize size,
                                const QMap<QString, TextureImagePtr>& sources = {},
                                const TextureNodeSettings& overrides = {}) {
   TextureNodeSettings settings;
   for (const TextureGeneratorSetting& setting : generator->getSettings()) {
      QVariant value = setting.defaultvalue;
      if (value.typeId() == QMetaType::QStringList) {
         value = value.toStringList().value(setting.defaultindex);
      }
      settings.insert(setting.id, value);
   }
   for (auto iterator = overrides.cbegin(); iterator != overrides.cend(); ++iterator) {
      settings.insert(iterator.key(), iterator.value());
   }

   TextureImagePtr output = TextureImage::create(size);
   generator->generate(size, output->data(), sources, settings);
   return output;
}

}  // namespace

/// @brief Verifies JavaScript generator execution, diagnostics, and discovery.
class JavaScriptGeneratorsTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies validation time limits and cancellation on either side of registration.
   void boundsValidationAndCancelsBeforeRegistration();

   /// @brief Verifies scan shutdown before work starts and while validation is stuck.
   void cancelsScannerDuringValidationAndBeforeStart();

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

   /// @brief Checks channel adjustment modes, rounding, clamping, and alpha selection.
   void rendersAdjust();

   /// @brief Checks default RGB inversion, selected channels, and missing Image.
   void rendersInvert();

   /// @brief Checks channel routing, fixed values, and disconnected inputs.
   void rendersSetChannels();

   /// @brief Checks alpha replacement, resetting opacity, and missing Image.
   void rendersReplaceAlpha();

   /// @brief Checks complementary alpha masks, unchanged colours, and missing inputs.
   void rendersMaskAndCutout();

   /// @brief Checks Blend modes, foreground opacity, and missing layers.
   void rendersBlend();

   /// @brief Verifies corrected metadata, alpha handling, centring, and edge behaviour.
   void rendersCorrectedBundledGenerators();
};

void JavaScriptGeneratorsTest::boundsValidationAndCancelsBeforeRegistration() {
   using namespace std::chrono_literals;
   JsTexGen stuck(QStringLiteral("while (true) {}"), QStringLiteral("stuck.js"),
                  TextureGenerator::Origin::Custom, {}, 30ms);
   QVERIFY(!stuck.isValid());
   QVERIFY(stuck.validationError().contains("stuck.js"));
   QVERIFY(stuck.validationError().contains("timed out"));

   // Accessors run during descriptor parsing must also be covered by the deadline.
   JsTexGen getter(QStringLiteral("const generator={get apiVersion(){while(true){}}};"),
                   QStringLiteral("getter.js"), TextureGenerator::Origin::Custom, {}, 30ms);
   QVERIFY(!getter.isValid());
   QVERIFY(getter.validationError().contains("timed out"));

   auto cancelled = QSharedPointer<ExecutionController>::create();
   cancelled->cancel();
   JsTexGen rejected(QStringLiteral("while(true){}"), QStringLiteral("cancelled.js"),
                     TextureGenerator::Origin::Custom, cancelled);
   QVERIFY(rejected.validationError().contains("validation cancelled"));

   QJSEngine engine;
   ExecutionController owner;
   {
      ExecutionController::Registration registration(owner, engine);
      owner.cancel();
      QVERIFY(engine.isInterrupted());
   }
   QVERIFY_EXCEPTION_THROWN(ExecutionController::Registration(owner, engine), std::runtime_error);
   ExecutionController next;
   ExecutionController::Registration registration(next, engine);
   QVERIFY(!engine.isInterrupted());
}

void JavaScriptGeneratorsTest::cancelsScannerDuringValidationAndBeforeStart() {
   QTemporaryDir directory;
   QVERIFY(writeTextFile(directory.filePath("stuck.js"), "while(true){}"));
   TextureProject project(false);
   for (bool startWorker : {false, true}) {
      auto manager = std::make_unique<JsTexGenManager>(&project);
      manager->setDirectory(directory.path());
      manager->setEnabled(true);
      if (startWorker) {
         QTest::qWait(100);
      }
      QElapsedTimer timer;
      timer.start();
      manager.reset();
      QVERIFY(timer.elapsed() < 2000);
   }
}

void JavaScriptGeneratorsTest::rendersAndReportsErrors() {
   QCOMPARE(renderColor(solidScript()), QColor(0x11, 0x22, 0x33, 0x44));
   QVERIFY(!JsTexGen(QStringLiteral("var name=;")).isValid());

   const JsTexGen removedGlobalsApi(QStringLiteral(
       "var name='Legacy';function getSettings(){return {};}function generate(){return [];}"));
   QVERIFY(!removedGlobalsApi.isValid());
   QVERIFY(removedGlobalsApi.validationError().contains(QStringLiteral("globals API")));

   const QString throwing = QStringLiteral(
       "const generator={apiVersion:1,name:'Failure',type:'generator',inputs:[],settings:[],"
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
       "settings:[],generate(size,settings,output,inputs){void size;void settings;"
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
       "inputs:['Input','Input'],settings:[],generate(){}};");
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
  settings: [
    { id: "imagewidth", type: "integer", name: "Image width setting", default: 7, min: 0, max: 20 },
    { id: "amount", type: "real", default: 0.5, min: 0, max: 1 },
    { id: "enabled", type: "boolean", default: true, group: "Behavior" },
    { id: "label", type: "string", default: "hello", description: "A label" },
    { id: "notes", type: "multiline", default: "two\nlines", enabler: "enabled" },
    { id: "color", type: "color", default: { r: 9, g: 8, b: 7, a: 6 } },
    { id: "mode", type: "choice", values: ["Add", "Multiply", "Overlay"], default: "Overlay" }
  ],
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
   QCOMPARE(settings.at(0).id, QStringLiteral("imagewidth"));
   QCOMPARE(settings.at(1).id, QStringLiteral("amount"));
   QCOMPARE(settings.at(2).id, QStringLiteral("enabled"));
   QCOMPARE(settings.at(3).id, QStringLiteral("label"));
   QCOMPARE(settings.at(4).id, QStringLiteral("notes"));
   QCOMPARE(settings.at(5).id, QStringLiteral("color"));
   QCOMPARE(settings.at(6).id, QStringLiteral("mode"));
   QCOMPARE(settings.at(0).defaultvalue, QVariant(7));
   QCOMPARE(settings.at(1).defaultvalue, QVariant(0.5));
   QCOMPARE(settings.at(2).defaultvalue, QVariant(true));
   QCOMPARE(settings.at(3).defaultvalue, QVariant(QStringLiteral("hello")));
   QVERIFY(settings.at(4).multiline);
   QCOMPARE(settings.at(4).enabler, QStringLiteral("enabled"));
   QCOMPARE(settings.at(5).defaultvalue.value<QColor>(), QColor(9, 8, 7, 6));
   QCOMPARE(
       settings.at(6).defaultvalue.toStringList(),
       QStringList({QStringLiteral("Add"), QStringLiteral("Multiply"), QStringLiteral("Overlay")}));
   QCOMPARE(settings.at(6).defaultindex, 2);

   const TextureGeneratorPtr generator(rawGenerator);
   TextureProject project(false);
   project.addGenerator(generator);
   const TextureImagePtr image = project.newNode(1, generator)->renderImage(QSize(2, 2));
   const TexturePixel pixel = image->data()[0];
   QCOMPARE(QColor(pixel.r, pixel.g, pixel.b, pixel.a), QColor(9, 8, 7, 6));

   const QString immutableInputScript = QStringLiteral(R"JS(
const generator = {
  apiVersion: 1, name: "Immutable", type: "filter", inputs: ["Image"], settings: [],
  generate(size, settings, output, inputs) {
    void size; void settings;
    const original = inputs.Image.data[0];
    inputs.Image.data[0] = 0;
    output.data.set(inputs.Image.data);
    output.data[0] = original;
  }
};
)JS");
   TextureImagePtr source = sourceImage(QColor(3, 4, 5, 6));
   QCOMPARE(renderWithSources(immutableInputScript, {{QStringLiteral("Image"), source}}),
            QColor(3, 4, 5, 6));
   QCOMPARE(
       QColor(source->data()[0].r, source->data()[0].g, source->data()[0].b, source->data()[0].a),
       QColor(3, 4, 5, 6));
}

void JavaScriptGeneratorsTest::validatesVersion1Descriptors() {
   const QString typeScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Typed',type:'%1',inputs:[],settings:[],"
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
           "settings:[],generate(){}};"),
       QStringLiteral(
           "const generator={apiVersion:1,name:'NoType',inputs:[],settings:[],generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'BadType',type:'effect',inputs:[],settings:[],"
                      "generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Slots',type:'filter',inputs:['Input','Input'],"
                      "settings:[],generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Range',type:'generator',inputs:[],settings:["
                      "{id:'value',type:'integer',default:20,min:0,max:10}],generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Choice',type:'generator',inputs:[],settings:["
                      "{id:'mode',type:'choice',values:['A'],default:'B'}],generate(){}};"),
       QStringLiteral("const "
                      "generator={apiVersion:1,name:'Callable',type:'generator',inputs:[],settings:"
                      "[],generate:3};"),
       QStringLiteral("const generator={apiVersion:1,name:'ObjectSettings',type:'generator',"
                      "inputs:[],settings:{},generate(){}};"),
       QStringLiteral("const generator={apiVersion:1,name:'PrimitiveSetting',type:'generator',"
                      "inputs:[],settings:[3],generate(){}};"),
       QStringLiteral("const generator={apiVersion:1,name:'MissingId',type:'generator',inputs:[],"
                      "settings:[{type:'integer',default:1}],generate(){}};"),
       QStringLiteral("const generator={apiVersion:1,name:'EmptyId',type:'generator',inputs:[],"
                      "settings:[{id:' ',type:'integer',default:1}],generate(){}};"),
       QStringLiteral("const generator={apiVersion:1,name:'NumericId',type:'generator',inputs:[],"
                      "settings:[{id:3,type:'integer',default:1}],generate(){}};"),
       QStringLiteral(
           "const generator={apiVersion:1,name:'DuplicateId',type:'generator',inputs:[],"
           "settings:[{id:'amount',type:'integer',default:1},{id:'amount',type:'integer',"
           "default:2}],generate(){}};"),
       QStringLiteral("const generator={apiVersion:1,name:'Order',type:'generator',inputs:[],"
                      "settings:[{id:'amount',type:'integer',default:1,order:1}],generate(){}};")};
   for (const QString& script : invalidScripts) {
      JsTexGen generator(script, QStringLiteral("invalid-v1.js"));
      QVERIFY(!generator.isValid());
      QVERIFY2(generator.validationError().contains(QStringLiteral("invalid-v1.js")),
               qPrintable(generator.validationError()));
   }

   const QString throwing = QStringLiteral(
       "const generator={apiVersion:1,name:'Throwing',type:'generator',inputs:[],settings:[],"
       "generate(){throw new Error('planned v1 failure');}};");
   JsTexGen generator(throwing, QStringLiteral("throwing-v1.js"));
   TexturePixel destination{};
   QVERIFY_EXCEPTION_THROWN(
       generator.generate(QSize(1, 1), &destination, {}, TextureNodeSettings()),
       std::runtime_error);
}

void JavaScriptGeneratorsTest::cachesConcurrentRuntimesAndInterrupts() {
   const QString fillScript = QStringLiteral(
       "const generator={apiVersion:1,name:'Cached',type:'generator',inputs:[],settings:[],"
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
   workers.reserve(2);
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
       "const generator={apiVersion:1,name:'Infinite',type:'generator',inputs:[],settings:[],"
       "generate(){while(true){}}};");
   JsTexGen infinite(infiniteScript, QStringLiteral("infinite.js"));
   auto controller = QSharedPointer<ExecutionController>::create();
   std::atomic_bool finished{false};
   std::atomic_bool interrupted{false};
   std::thread renderThread([&]() {
      try {
         TexturePixel pixel{};
         infinite.generate(QSize(1, 1), &pixel, {}, TextureNodeSettings(), controller);
      } catch (const std::runtime_error& error) {
         interrupted = QString::fromUtf8(error.what())
                           .contains(QStringLiteral("interrupted"), Qt::CaseInsensitive) ||
                       QString::fromUtf8(error.what()).contains(QStringLiteral("cancelled"));
      }
      finished = true;
   });
   for (int attempt = 0; attempt < 100 && !finished.load(); ++attempt) {
      QTest::qWait(10);
      controller->cancel();
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
  settings: [{ id: "amount", type: "integer", default: 2, min: 0, max: 20 }],
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
       "const generator={apiVersion:1,name:'ReloadSource',type:'generator',inputs:[],settings:[],"
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
  settings: [
    { id: "enabled", type: "boolean", default: true },
    { id: "amount", type: "integer", default: 3, min: 0, max: 20 }
  ],
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

void JavaScriptGeneratorsTest::rendersAdjust() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto generator = project.getGenerator(QStringLiteral("Adjust"));
   QVERIFY(!generator.isNull());
   QCOMPARE(generator->getSettings().at(4).defaultvalue.toStringList(),
            QStringList(
                {QStringLiteral("Multiply"), QStringLiteral("Add"), QStringLiteral("Subtract")}));
   QCOMPARE(generator->getSettings().at(5).min.toDouble(), 0.0);
   QCOMPARE(generator->getSettings().at(5).max.toDouble(), 255.0);
   const QSize size(2, 1);
   const auto image = TextureImage::create(size);
   image->data()[0] = TexturePixel(101, 128, 250, 127);
   image->data()[1] = TexturePixel(0, 1, 255, 0);
   const QMap<QString, TextureImagePtr> sources{{QStringLiteral("Image"), image}};
   const auto defaults = renderGenerator(generator, size, sources);
   const auto half = renderGenerator(generator, size, sources, {{QStringLiteral("level"), 50.0}});
   QCOMPARE(half->data()[0].toRGBA(), TexturePixel(51, 64, 125, 127).toRGBA());
   QCOMPARE(half->data()[1].toRGBA(), TexturePixel(0, 1, 128, 0).toRGBA());
   const auto brighter = renderGenerator(
       generator, size, sources,
       {{QStringLiteral("mode"), QStringLiteral("Add")}, {QStringLiteral("level"), 20.0}});
   QCOMPARE(brighter->data()[0].toRGBA(), TexturePixel(121, 148, 255, 127).toRGBA());
   QCOMPARE(brighter->data()[1].toRGBA(), TexturePixel(20, 21, 255, 0).toRGBA());
   const auto darker = renderGenerator(
       generator, size, sources,
       {{QStringLiteral("mode"), QStringLiteral("Subtract")}, {QStringLiteral("level"), 150.0}});
   QCOMPARE(darker->data()[0].toRGBA(), TexturePixel(0, 0, 100, 127).toRGBA());
   QCOMPARE(darker->data()[1].toRGBA(), TexturePixel(0, 0, 105, 0).toRGBA());
   const auto boosted =
       renderGenerator(generator, size, sources, {{QStringLiteral("level"), 255.0}});
   QCOMPARE(boosted->data()[0].toRGBA(), TexturePixel(255, 255, 255, 127).toRGBA());
   QCOMPARE(boosted->data()[1].toRGBA(), TexturePixel(0, 3, 255, 0).toRGBA());
   const auto aboveRange =
       renderGenerator(generator, size, sources, {{QStringLiteral("level"), 500.0}});
   const auto belowRange =
       renderGenerator(generator, size, sources, {{QStringLiteral("level"), -100.0}});
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      QCOMPARE(aboveRange->data()[pixel].toRGBA(), boosted->data()[pixel].toRGBA());
      QCOMPARE(belowRange->data()[pixel].toRGBA(),
               TexturePixel(0, 0, 0, image->data()[pixel].a).toRGBA());
   }
   const TextureNodeSettings alphaOnly{{QStringLiteral("channelRed"), false},
                                       {QStringLiteral("channelGreen"), false},
                                       {QStringLiteral("channelBlue"), false},
                                       {QStringLiteral("channelAlpha"), true},
                                       {QStringLiteral("level"), 50.0}};
   const auto transparent = renderGenerator(generator, size, sources, alphaOnly);
   QCOMPARE(transparent->data()[0].toRGBA(), TexturePixel(101, 128, 250, 64).toRGBA());
   QCOMPARE(transparent->data()[1].toRGBA(), image->data()[1].toRGBA());
   const auto unchecked = renderGenerator(generator, size, sources,
                                          {{QStringLiteral("channelRed"), false},
                                           {QStringLiteral("channelGreen"), false},
                                           {QStringLiteral("channelBlue"), false},
                                           {QStringLiteral("channelAlpha"), false},
                                           {QStringLiteral("level"), 255.0}});
   const auto missing = renderGenerator(generator, size, {},
                                        {{QStringLiteral("mode"), QStringLiteral("Add")},
                                         {QStringLiteral("level"), 255.0},
                                         {QStringLiteral("channelAlpha"), true}});
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      QCOMPARE(defaults->data()[pixel].toRGBA(), image->data()[pixel].toRGBA());
      QCOMPARE(unchecked->data()[pixel].toRGBA(), image->data()[pixel].toRGBA());
      QCOMPARE(missing->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
   }
}

void JavaScriptGeneratorsTest::rendersInvert() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto generator = project.getGenerator(QStringLiteral("Invert"));
   QVERIFY(!generator.isNull());
   const QSize size(3, 1);
   const auto image = TextureImage::create(size);
   image->data()[0] = TexturePixel(0, 128, 255, 64);
   image->data()[1] = TexturePixel(10, 20, 30, 0);
   image->data()[2] = TexturePixel(255, 0, 1, 255);
   const QMap<QString, TextureImagePtr> sources{{QStringLiteral("Image"), image}};
   const auto defaults = renderGenerator(generator, size, sources);
   const QList<TexturePixel> expectedDefault{TexturePixel(255, 127, 0, 64),
                                             TexturePixel(245, 235, 225, 0),
                                             TexturePixel(0, 255, 254, 255)};
   const TextureNodeSettings blueAndAlpha{{QStringLiteral("invertRed"), false},
                                          {QStringLiteral("invertGreen"), false},
                                          {QStringLiteral("invertBlue"), true},
                                          {QStringLiteral("invertAlpha"), true}};
   const auto selected = renderGenerator(generator, size, sources, blueAndAlpha);
   const QList<TexturePixel> expectedSelected{
       TexturePixel(0, 128, 0, 191), TexturePixel(10, 20, 225, 255), TexturePixel(255, 0, 254, 0)};
   const auto unchanged = renderGenerator(generator, size, sources,
                                          {{QStringLiteral("invertRed"), false},
                                           {QStringLiteral("invertGreen"), false},
                                           {QStringLiteral("invertBlue"), false},
                                           {QStringLiteral("invertAlpha"), false}});
   const auto missing = renderGenerator(generator, size, {}, blueAndAlpha);
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      QCOMPARE(defaults->data()[pixel].toRGBA(), expectedDefault[pixel].toRGBA());
      QCOMPARE(selected->data()[pixel].toRGBA(), expectedSelected[pixel].toRGBA());
      QCOMPARE(unchanged->data()[pixel].toRGBA(), image->data()[pixel].toRGBA());
      QCOMPARE(missing->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
   }
}

void JavaScriptGeneratorsTest::rendersSetChannels() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto generator = project.getGenerator(QStringLiteral("Set channels"));
   QVERIFY(!generator.isNull());
   const QSize size(2, 1);
   const auto first = TextureImage::create(size);
   const auto second = TextureImage::create(size);
   first->data()[0] = TexturePixel(10, 20, 30, 40);
   first->data()[1] = TexturePixel(11, 21, 31, 41);
   second->data()[0] = TexturePixel(50, 60, 70, 80);
   second->data()[1] = TexturePixel(51, 61, 71, 81);
   const QMap<QString, TextureImagePtr> sources{{QStringLiteral("First"), first},
                                                {QStringLiteral("Second"), second}};
   const auto defaults = renderGenerator(generator, size, sources);
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      QCOMPARE(defaults->data()[pixel].toRGBA(), first->data()[pixel].toRGBA());
   }
   const QStringList choices = generator->getSettings().at(0).defaultvalue.toStringList();
   const QList<quint8> expectedValues{0, 255, 10, 20, 30, 40, 50, 60, 70, 80};
   QCOMPARE(choices.size(), expectedValues.size());
   for (int choice = 0; choice < choices.size(); ++choice) {
      TextureNodeSettings settings;
      for (const auto& setting : generator->getSettings()) {
         settings.insert(setting.id, choices[choice]);
      }
      const auto output = renderGenerator(generator, size, sources, settings);
      const auto missing = renderGenerator(generator, size, {}, settings);
      for (int pixel = 0; pixel < size.width(); ++pixel) {
         const auto value = static_cast<quint8>(expectedValues[choice] + (choice < 2 ? 0 : pixel));
         QCOMPARE(output->data()[pixel].toRGBA(),
                  TexturePixel(value, value, value, value).toRGBA());
         const quint8 missingValue = choice == 1 ? 255 : 0;
         QCOMPARE(missing->data()[pixel].toRGBA(),
                  TexturePixel(missingValue, missingValue, missingValue, missingValue).toRGBA());
      }
   }
   const auto mixed =
       renderGenerator(generator, size, sources,
                       {{QStringLiteral("channelRed"), QStringLiteral("Second's alpha")},
                        {QStringLiteral("channelGreen"), QStringLiteral("First's blue")},
                        {QStringLiteral("channelBlue"), QStringLiteral("Fill")},
                        {QStringLiteral("channelAlpha"), QStringLiteral("None")}});
   QCOMPARE(mixed->data()[0].toRGBA(), TexturePixel(80, 30, 255, 0).toRGBA());
   const auto firstOnly = renderGenerator(generator, size, {{QStringLiteral("First"), first}});
   const auto secondOnly = renderGenerator(generator, size, {{QStringLiteral("Second"), second}});
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      QCOMPARE(firstOnly->data()[pixel].toRGBA(), first->data()[pixel].toRGBA());
      QCOMPARE(secondOnly->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
   }
}

void JavaScriptGeneratorsTest::rendersReplaceAlpha() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto generator = project.getGenerator(QStringLiteral("Replace alpha"));
   QVERIFY(!generator.isNull());
   QVERIFY(generator->getSettings().isEmpty());
   const QSize size(4, 1);
   const auto image = TextureImage::create(size);
   const auto alpha = TextureImage::create(size);
   const QList<quint8> imageAlpha{0, 255, 100, 20};
   const QList<quint8> replacement{200, 0, 128, 255};
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      image->data()[pixel] = TexturePixel(10 + pixel, 20 + pixel, 30 + pixel, imageAlpha[pixel]);
      alpha->data()[pixel] = TexturePixel(200, 150, 100, replacement[pixel]);
   }
   const auto output = renderGenerator(
       generator, size, {{QStringLiteral("Image"), image}, {QStringLiteral("Alpha"), alpha}});
   const auto opaque = renderGenerator(generator, size, {{QStringLiteral("Image"), image}});
   const auto noImage = renderGenerator(generator, size, {{QStringLiteral("Alpha"), alpha}});
   const auto empty = renderGenerator(generator, size);
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      TexturePixel expected = image->data()[pixel];
      expected.a = replacement[pixel];
      QCOMPARE(output->data()[pixel].toRGBA(), expected.toRGBA());
      expected.a = 255;
      QCOMPARE(opaque->data()[pixel].toRGBA(), expected.toRGBA());
      QCOMPARE(noImage->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
      QCOMPARE(empty->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
   }
}

void JavaScriptGeneratorsTest::rendersMaskAndCutout() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto cutout = project.getGenerator(QStringLiteral("Cutout"));
   QVERIFY(!cutout.isNull());
   const auto maskGenerator = project.getGenerator(QStringLiteral("Mask"));
   QVERIFY(!maskGenerator.isNull());
   const QList<quint8> imageAlpha{255, 255, 255, 128, 128, 128, 0, 1, 1, 200};
   const QList<quint8> maskAlpha{0, 128, 255, 0, 128, 255, 128, 128, 127, 50};
   const QList<quint8> expectedCutoutAlpha{255, 127, 0, 128, 64, 0, 0, 0, 1, 161};
   const QList<quint8> expectedMaskAlpha{0, 128, 255, 0, 64, 128, 0, 1, 0, 39};
   const QSize size(10, 1);
   const auto image = TextureImage::create(size);
   const auto mask = TextureImage::create(size);
   for (int pixel = 0; pixel < size.width(); ++pixel) {
      image->data()[pixel] = TexturePixel(10 + pixel, 20 + pixel, 30 + pixel, imageAlpha[pixel]);
      mask->data()[pixel] = TexturePixel(100 + pixel, 110 + pixel, 120 + pixel, maskAlpha[pixel]);
   }
   const QMap<QString, TextureImagePtr> sources{{QStringLiteral("Image"), image},
                                                {QStringLiteral("Mask"), mask}};
   for (const auto& generator : {maskGenerator, cutout}) {
      QVERIFY(generator->getSettings().isEmpty());
      const auto output = renderGenerator(generator, size, sources);
      const auto onlyImage = renderGenerator(generator, size, {{QStringLiteral("Image"), image}});
      const auto onlyMask = renderGenerator(generator, size, {{QStringLiteral("Mask"), mask}});
      const auto empty = renderGenerator(generator, size);
      const auto& expectedAlpha =
          generator == maskGenerator ? expectedMaskAlpha : expectedCutoutAlpha;
      for (int pixel = 0; pixel < size.width(); ++pixel) {
         TexturePixel expected = image->data()[pixel];
         expected.a = expectedAlpha[pixel];
         QCOMPARE(output->data()[pixel].toRGBA(), expected.toRGBA());
         QCOMPARE(onlyImage->data()[pixel].toRGBA(), image->data()[pixel].toRGBA());
         QCOMPARE(onlyMask->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
         QCOMPARE(empty->data()[pixel].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());
      }
   }
}

void JavaScriptGeneratorsTest::rendersBlend() {
   TextureProject project(false);
   registerBuiltInGenerators(project);
   const auto blend = project.getGenerator(QStringLiteral("Blend"));
   QVERIFY(!blend.isNull());
   QCOMPARE(blend->getSettings().size(), 2);
   QCOMPARE(blend->getSettings().at(0).id, QStringLiteral("mode"));
   QCOMPARE(blend->getSettings().at(1).id, QStringLiteral("alpha"));
   QCOMPARE(blend->getSettings().at(0).defaultvalue.toStringList(),
            QStringList({QStringLiteral("Normal"), QStringLiteral("Add"),
                         QStringLiteral("Multiply"), QStringLiteral("Screen"),
                         QStringLiteral("Overlay"), QStringLiteral("Difference")}));
   const QSize size(1, 1);
   const auto background = sourceImage(QColor(10, 20, 30, 80));
   const auto foreground = sourceImage(QColor(200, 100, 50, 128));
   const auto transparentBackground = sourceImage(QColor(10, 20, 30, 0));
   const QMap<QString, TextureImagePtr> foregroundOnly{{QStringLiteral("Foreground"), foreground}};
   const QMap<QString, TextureImagePtr> backgroundOnly{{QStringLiteral("Background"), background}};
   const QMap<QString, TextureImagePtr> both{{QStringLiteral("Background"), background},
                                             {QStringLiteral("Foreground"), foreground}};
   const QMap<QString, TextureImagePtr> transparentBelow{
       {QStringLiteral("Background"), transparentBackground},
       {QStringLiteral("Foreground"), foreground}};
   for (const QString& mode : blend->getSettings().at(0).defaultvalue.toStringList()) {
      const TextureNodeSettings halfOpacity{{QStringLiteral("mode"), mode},
                                            {QStringLiteral("alpha"), 50.0}};
      const TextureNodeSettings invisible{{QStringLiteral("mode"), mode},
                                          {QStringLiteral("alpha"), 0.0}};
      QCOMPARE(renderGenerator(blend, size, foregroundOnly, halfOpacity)->data()[0].toRGBA(),
               TexturePixel(200, 100, 50, 64).toRGBA());
      QCOMPARE(renderGenerator(blend, size, transparentBelow, halfOpacity)->data()[0].toRGBA(),
               TexturePixel(200, 100, 50, 64).toRGBA());
      QCOMPARE(renderGenerator(blend, size, backgroundOnly, halfOpacity)->data()[0].toRGBA(),
               background->data()[0].toRGBA());
      QCOMPARE(renderGenerator(blend, size, both, invisible)->data()[0].toRGBA(),
               background->data()[0].toRGBA());
      QCOMPARE(renderGenerator(blend, size, foregroundOnly, invisible)->data()[0].toRGBA(),
               TexturePixel(0, 0, 0, 0).toRGBA());
      QCOMPARE(renderGenerator(blend, size, {}, halfOpacity)->data()[0].toRGBA(),
               TexturePixel(0, 0, 0, 0).toRGBA());
   }
   const QMap<QString, TextureImagePtr> opaqueLayers{
       {QStringLiteral("Background"), sourceImage(QColor(64, 128, 192))},
       {QStringLiteral("Foreground"), sourceImage(QColor(128, 128, 64))}};
   const TextureNodeSettings add{{QStringLiteral("mode"), QStringLiteral("Add")}};
   QCOMPARE(renderGenerator(blend, size, opaqueLayers, add)->data()[0].toRGBA(),
            TexturePixel(192, 255, 255, 255).toRGBA());
   QCOMPARE(renderGenerator(
                blend, size,
                {{QStringLiteral("Background"), opaqueLayers.value(QStringLiteral("Background"))},
                 {QStringLiteral("Foreground"), sourceImage(QColor(200, 200, 200, 0))}},
                add)
                ->data()[0]
                .toRGBA(),
            opaqueLayers.value(QStringLiteral("Background"))->data()[0].toRGBA());
   QCOMPARE(renderGenerator(blend, size,
                            {{QStringLiteral("Background"), transparentBackground},
                             {QStringLiteral("Foreground"), sourceImage(QColor(200, 200, 200, 0))}},
                            add)
                ->data()[0]
                .toRGBA(),
            TexturePixel(0, 0, 0, 0).toRGBA());
   QCOMPARE(renderGenerator(blend, size, opaqueLayers,
                            {{QStringLiteral("mode"), QStringLiteral("Multiply")}})
                ->data()[0]
                .toRGBA(),
            TexturePixel(32, 64, 48, 255).toRGBA());
   QCOMPARE(renderGenerator(blend, size, opaqueLayers,
                            {{QStringLiteral("mode"), QStringLiteral("Screen")}})
                ->data()[0]
                .toRGBA(),
            TexturePixel(159, 191, 207, 255).toRGBA());
   QCOMPARE(renderGenerator(blend, size, opaqueLayers,
                            {{QStringLiteral("mode"), QStringLiteral("Overlay")}})
                ->data()[0]
                .toRGBA(),
            TexturePixel(64, 128, 160, 255).toRGBA());
   QCOMPARE(renderGenerator(blend, size, opaqueLayers,
                            {{QStringLiteral("mode"), QStringLiteral("Difference")}})
                ->data()[0]
                .toRGBA(),
            TexturePixel(64, 0, 128, 255).toRGBA());
}

void JavaScriptGeneratorsTest::rendersCorrectedBundledGenerators() {
   TextureProject project(false);
   registerBuiltInGenerators(project);

   const TextureGeneratorPtr checkboard = project.getGenerator(QStringLiteral("Checkboard"));
   QVERIFY(!checkboard.isNull());

   const TextureGeneratorPtr fire = project.getGenerator(QStringLiteral("Fire"));
   QVERIFY(!fire.isNull());
   for (const TextureGeneratorSetting& setting : fire->getSettings()) {
      QVERIFY2(setting.group.isEmpty(), qPrintable(setting.id));
   }

   TextureImagePtr lowerLayer = TextureImage::create(QSize(4, 1));
   lowerLayer->data()[0] = TexturePixel(10, 20, 30, 255);
   lowerLayer->data()[1] = TexturePixel(20, 40, 60, 255);
   lowerLayer->data()[2] = TexturePixel(0, 0, 255, 128);
   lowerLayer->data()[3] = TexturePixel(10, 20, 30, 0);
   TextureImagePtr upperLayer = TextureImage::create(QSize(4, 1));
   upperLayer->data()[0] = TexturePixel(100, 110, 120, 255);
   upperLayer->data()[1] = TexturePixel(200, 210, 220, 0);
   upperLayer->data()[2] = TexturePixel(255, 0, 0, 128);
   upperLayer->data()[3] = TexturePixel(200, 210, 220, 0);
   const TextureGeneratorPtr blending = project.getGenerator(QStringLiteral("Blend"));
   const QMap<QString, TextureImagePtr> layers = {
       {QStringLiteral("Background"), lowerLayer},
       {QStringLiteral("Foreground"), upperLayer},
   };
   const TextureImagePtr normallyBlended = renderGenerator(blending, QSize(4, 1), layers);
   QCOMPARE(normallyBlended->data()[0].toRGBA(), upperLayer->data()[0].toRGBA());
   QCOMPARE(normallyBlended->data()[1].toRGBA(), lowerLayer->data()[1].toRGBA());
   QCOMPARE(normallyBlended->data()[2].toRGBA(), TexturePixel(170, 0, 84, 191).toRGBA());
   QCOMPARE(normallyBlended->data()[3].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());

   const TextureImagePtr reversedLayers = renderGenerator(
       blending, QSize(4, 1),
       {{QStringLiteral("Background"), upperLayer}, {QStringLiteral("Foreground"), lowerLayer}});
   QCOMPARE(reversedLayers->data()[0].toRGBA(), lowerLayer->data()[0].toRGBA());

   const TextureGeneratorPtr lines = project.getGenerator(QStringLiteral("Lines"));
   const auto phaseIterator =
       std::find_if(lines->getSettings().cbegin(), lines->getSettings().cend(),
                    [](const TextureGeneratorSetting& setting) {
                       return setting.id == QStringLiteral("offset");
                    });
   QVERIFY(phaseIterator != lines->getSettings().cend());
   QCOMPARE(phaseIterator->name, QStringLiteral("Pattern phase (%)"));
   QCOMPARE(phaseIterator->min.toInt(), 0);
   QCOMPARE(phaseIterator->max.toInt(), 100);

   const TextureGeneratorPtr perlin = project.getGenerator(QStringLiteral("Perlin noise"));
   QVERIFY(!perlin.isNull());
   const TextureImagePtr firstNoise = renderGenerator(perlin, QSize(19, 13));
   const TextureImagePtr repeatedNoise = renderGenerator(perlin, QSize(19, 13));
   for (std::size_t pixel = 0; pixel < firstNoise->pixelCount(); ++pixel) {
      QCOMPARE(firstNoise->data()[pixel].toRGBA(), repeatedNoise->data()[pixel].toRGBA());
   }

   TextureNodeSettings differentSeedSettings;
   differentSeedSettings.insert(QStringLiteral("randomizer"), 501);
   const TextureImagePtr differentNoise =
       renderGenerator(perlin, QSize(19, 13), {}, differentSeedSettings);
   bool seedChangedOutput = false;
   for (std::size_t pixel = 0; pixel < firstNoise->pixelCount(); ++pixel) {
      if (firstNoise->data()[pixel].toRGBA() != differentNoise->data()[pixel].toRGBA()) {
         seedChangedOutput = true;
         break;
      }
   }
   QVERIFY(seedChangedOutput);

   TextureNodeSettings seamlessSettings;
   seamlessSettings.insert(QStringLiteral("seamless"), true);
   const QSize seamlessSize(19, 13);
   const TextureImagePtr seamlessNoise =
       renderGenerator(perlin, seamlessSize, {}, seamlessSettings);
   for (int y = 0; y < seamlessSize.height(); ++y) {
      QCOMPARE(seamlessNoise->data()[y * seamlessSize.width()].toRGBA(),
               seamlessNoise->data()[y * seamlessSize.width() + seamlessSize.width() - 1].toRGBA());
   }
   for (int x = 0; x < seamlessSize.width(); ++x) {
      QCOMPARE(
          seamlessNoise->data()[x].toRGBA(),
          seamlessNoise->data()[(seamlessSize.height() - 1) * seamlessSize.width() + x].toRGBA());
   }

   TextureNodeSettings circleSettings;
   circleSettings.insert(QStringLiteral("innerradius"), 50.0);
   circleSettings.insert(QStringLiteral("outerradius"), 50.0);
   const TextureImagePtr emptyCircle = renderGenerator(
       project.getGenerator(QStringLiteral("Circle")), QSize(17, 13), {}, circleSettings);
   for (std::size_t pixel = 0; pixel < emptyCircle->pixelCount(); ++pixel) {
      QCOMPARE(emptyCircle->data()[pixel].a, static_cast<quint8>(0));
   }

   TextureNodeSettings brickSettings;
   brickSettings.insert(QStringLiteral("color"), QColor(100, 80, 60, 128));
   brickSettings.insert(QStringLiteral("linewidth"), 20.0);
   brickSettings.insert(QStringLiteral("brickwidth"), 40.0);
   brickSettings.insert(QStringLiteral("brickheight"), 40.0);
   const TextureImagePtr bricks = renderGenerator(project.getGenerator(QStringLiteral("Bricks")),
                                                  QSize(10, 10), {}, brickSettings);
   int mortarPixels = 0;
   for (std::size_t pixel = 0; pixel < bricks->pixelCount(); ++pixel) {
      if (bricks->data()[pixel].a > 0) {
         QCOMPARE(bricks->data()[pixel].a, static_cast<quint8>(128));
         ++mortarPixels;
      }
   }
   QVERIFY(mortarPixels > 0);

   TextureImagePtr twoPixels = TextureImage::create(QSize(2, 1));
   twoPixels->data()[0] = TexturePixel(255, 0, 0, 255);
   twoPixels->data()[1] = TexturePixel(0, 0, 255, 0);
   TextureNodeSettings pixelateSettings;
   pixelateSettings.insert(QStringLiteral("width"), 100.0);
   pixelateSettings.insert(QStringLiteral("height"), 100.0);
   const TextureGeneratorPtr pixelate = project.getGenerator(QStringLiteral("Pixelate"));
   const TextureImagePtr averaged = renderGenerator(
       pixelate, QSize(2, 1), {{QStringLiteral("Image"), twoPixels}}, pixelateSettings);
   QCOMPARE(averaged->data()[0].r, static_cast<quint8>(255));
   QCOMPARE(averaged->data()[0].g, static_cast<quint8>(0));
   QCOMPARE(averaged->data()[0].b, static_cast<quint8>(0));
   QCOMPARE(averaged->data()[0].a, static_cast<quint8>(127));

   pixelateSettings.insert(QStringLiteral("width"), 0.0);
   const TextureImagePtr unpixelated = renderGenerator(
       pixelate, QSize(2, 1), {{QStringLiteral("Image"), twoPixels}}, pixelateSettings);
   QCOMPARE(unpixelated->data()[0].toRGBA(), twoPixels->data()[0].toRGBA());
   QCOMPARE(unpixelated->data()[1].toRGBA(), twoPixels->data()[1].toRGBA());

   TextureImagePtr centrePixel = TextureImage::create(QSize(5, 5));
   for (std::size_t pixel = 0; pixel < centrePixel->pixelCount(); ++pixel) {
      centrePixel->data()[pixel] = TexturePixel(0, 0, 0, 0);
   }
   centrePixel->data()[2 * 5 + 2] = TexturePixel(255, 255, 255, 255);
   TextureNodeSettings glowSettings;
   glowSettings.insert(QStringLiteral("color"), QColor(255, 255, 0, 0));
   glowSettings.insert(QStringLiteral("includesource"), false);
   const TextureImagePtr transparentGlow =
       renderGenerator(project.getGenerator(QStringLiteral("Glow")), QSize(5, 5),
                       {{QStringLiteral("Image"), centrePixel}}, glowSettings);
   for (std::size_t pixel = 0; pixel < transparentGlow->pixelCount(); ++pixel) {
      QCOMPARE(transparentGlow->data()[pixel].a, static_cast<quint8>(0));
   }

   TextureNodeSettings squareSettings;
   squareSettings.insert(QStringLiteral("cutoutwidth"), 100.0);
   squareSettings.insert(QStringLiteral("cutoutheight"), 100.0);
   const TextureImagePtr emptySquare = renderGenerator(
       project.getGenerator(QStringLiteral("Square")), QSize(17, 13), {}, squareSettings);
   for (std::size_t pixel = 0; pixel < emptySquare->pixelCount(); ++pixel) {
      QCOMPARE(emptySquare->data()[pixel].a, static_cast<quint8>(0));
   }

   TextureImagePtr transformSource = TextureImage::create(QSize(17, 13));
   for (std::size_t pixel = 0; pixel < transformSource->pixelCount(); ++pixel) {
      transformSource->data()[pixel] = TexturePixel(0, 0, 0, 0);
   }
   transformSource->data()[6 * 17 + 8] = TexturePixel(10, 20, 30, 255);
   TextureNodeSettings transformSettings;
   transformSettings.insert(QStringLiteral("rotation"), 90.0);
   const TextureImagePtr transformed =
       renderGenerator(project.getGenerator(QStringLiteral("Transform")), QSize(17, 13),
                       {{QStringLiteral("Image"), transformSource}}, transformSettings);
   QCOMPARE(transformed->data()[6 * 17 + 8].a, static_cast<quint8>(255));

   TextureImagePtr shadowSource = TextureImage::create(QSize(5, 5));
   for (std::size_t pixel = 0; pixel < shadowSource->pixelCount(); ++pixel) {
      shadowSource->data()[pixel] = TexturePixel(0, 0, 0, 0);
   }
   shadowSource->data()[2 * 5] = TexturePixel(255, 255, 255, 255);
   TextureNodeSettings shadowSettings;
   shadowSettings.insert(QStringLiteral("offsetleft"), 100.0);
   shadowSettings.insert(QStringLiteral("offsettop"), 0.0);
   shadowSettings.insert(QStringLiteral("level"), 20.0);
   const TextureImagePtr shadow =
       renderGenerator(project.getGenerator(QStringLiteral("Shadow")), QSize(5, 5),
                       {{QStringLiteral("Foreground"), shadowSource}}, shadowSettings);
   QVERIFY(shadow->data()[2 * 5 + 4].a > 0);

   TextureImagePtr background = TextureImage::create(QSize(1, 1));
   background->data()[0] = TexturePixel(20, 30, 40, 255);
   TextureNodeSettings perlinSettings;
   perlinSettings.insert(QStringLiteral("color"), QColor(255, 0, 0, 0));
   const TextureImagePtr transparentNoise = renderGenerator(
       perlin, QSize(1, 1), {{QStringLiteral("Background"), background}}, perlinSettings);
   QCOMPARE(transparentNoise->data()[0].toRGBA(), background->data()[0].toRGBA());

   TextureNodeSettings plasmaSettings;
   plasmaSettings.insert(QStringLiteral("color"), QColor(255, 0, 0, 0));
   const TextureImagePtr plasma =
       renderGenerator(project.getGenerator(QStringLiteral("Sine plasma")), QSize(1, 1),
                       {{QStringLiteral("Background"), background}}, plasmaSettings);
   QCOMPARE(plasma->data()[0].toRGBA(), background->data()[0].toRGBA());
}

QTEST_GUILESS_MAIN(JavaScriptGeneratorsTest)
#include "javascript_generators_test.moc"
