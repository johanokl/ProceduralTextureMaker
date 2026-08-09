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
      settings.insert(setting.id, setting.defaultvalue);
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

   /// @brief Verifies corrected metadata, alpha handling, centring, and edge behaviour.
   void rendersCorrectedBundledGenerators();
};

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
   const TextureGeneratorPtr blending = project.getGenerator(QStringLiteral("Blending"));
   const QMap<QString, TextureImagePtr> layers = {
       {QStringLiteral("Base"), lowerLayer},
       {QStringLiteral("Blend"), upperLayer},
   };
   const TextureImagePtr normallyBlended = renderGenerator(blending, QSize(4, 1), layers);
   QCOMPARE(normallyBlended->data()[0].toRGBA(), upperLayer->data()[0].toRGBA());
   QCOMPARE(normallyBlended->data()[1].toRGBA(), lowerLayer->data()[1].toRGBA());
   QCOMPARE(normallyBlended->data()[2].toRGBA(), TexturePixel(170, 0, 84, 191).toRGBA());
   QCOMPARE(normallyBlended->data()[3].toRGBA(), TexturePixel(0, 0, 0, 0).toRGBA());

   TextureNodeSettings reversedLayerSettings;
   reversedLayerSettings.insert(QStringLiteral("order"), QStringLiteral("Base on top of Blend"));
   const TextureImagePtr reversedLayers =
       renderGenerator(blending, QSize(4, 1), layers, reversedLayerSettings);
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
