// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/jstexgen.h"
#include <QColor>
#include <QCryptographicHash>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QMetaType>
#include <QSet>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <utility>

std::atomic<quint64> JsTexGen::nextStableId{1};
std::atomic<quint64> JsTexGen::evaluationCount{0};

namespace {

/// @brief Number of wrapper lines preceding user source in descriptorProgram().
constexpr int descriptorWrapperLineOffset = 2;

/// @brief Wraps descriptor-API source so evaluation returns its `generator` object.
/// @param source Complete JavaScript generator source.
/// @return An isolated strict-mode program that returns the descriptor.
QString descriptorProgram(const QString& source) {
   return QStringLiteral(
              "(function() {\n\"use strict\";\n%1\n"
              "return typeof generator === \"undefined\" ? undefined : generator;\n"
              "})()")
       .arg(source);
}

/// @brief Formats a JavaScript exception with its source, adjusted line, and stack trace.
/// @param error JavaScript exception value.
/// @param fallbackSource Source identity used when the exception has no file name.
/// @param lineAdjustment Offset applied to positive line numbers.
/// @return A human-readable diagnostic.
QString jsError(const QJSValue& error, const QString& fallbackSource,
                const int lineAdjustment = 0) {
   QString message = error.property(QStringLiteral("message")).toString();
   if (message.isEmpty()) {
      message = error.toString();
   }
   QString fileName = error.property(QStringLiteral("fileName")).toString();
   if (fileName.isEmpty()) {
      fileName = fallbackSource;
   }
   int line = error.property(QStringLiteral("lineNumber")).toInt();
   if (line > 0) {
      line = std::max(1, line + lineAdjustment);
   }
   const QString stack = error.property(QStringLiteral("stack")).toString();
   QString result = line > 0 ? QStringLiteral("%1:%2: %3").arg(fileName).arg(line).arg(message)
                             : QStringLiteral("%1: %2").arg(fileName, message);
   if (!stack.isEmpty() && !result.contains(stack)) {
      result += QStringLiteral("\n%1").arg(stack);
   }
   return result;
}

/// @brief Reads an optional string property from a JavaScript object.
/// @param object Object containing the property.
/// @param property Property name to read.
/// @param destination Destination updated when the property is present and valid.
/// @param error Destination for a validation failure.
/// @param context Object path used in the diagnostic.
/// @return @c true when the property is absent or contains a string.
bool readOptionalString(const QJSValue& object, const QString& property, QString& destination,
                        QString& error, const QString& context) {
   const QJSValue value = object.property(property);
   if (value.isUndefined()) {
      return true;
   }
   if (!value.isString()) {
      error = QStringLiteral("%1.%2 must be a string").arg(context, property);
      return false;
   }
   destination = value.toString();
   return true;
}

/// @brief Converts a finite, integral JavaScript number to a C++ integer.
/// @param value JavaScript value to validate.
/// @param destination Destination for the converted value.
/// @return @c true when the value is exactly representable as an `int`.
bool readInteger(const QJSValue& value, int& destination) {
   if (!value.isNumber() || !std::isfinite(value.toNumber()) ||
       std::floor(value.toNumber()) != value.toNumber() ||
       value.toNumber() < std::numeric_limits<int>::min() ||
       value.toNumber() > std::numeric_limits<int>::max()) {
      return false;
   }
   destination = value.toInt();
   return true;
}

/// @brief Parses an RGBA color object whose channel values range from 0 to 255.
/// @param value JavaScript `{r, g, b, a}` object to parse.
/// @param color Destination for the converted color.
/// @param error Destination for a validation failure.
/// @param context Object path used in the diagnostic.
/// @return @c true when all four channels are valid.
bool parseColor(const QJSValue& value, QColor& color, QString& error, const QString& context) {
   if (!value.isObject()) {
      error = QStringLiteral("%1 must be an {r, g, b, a} object").arg(context);
      return false;
   }
   int channels[4]{};
   const QStringList names{QStringLiteral("r"), QStringLiteral("g"), QStringLiteral("b"),
                           QStringLiteral("a")};
   for (int index = 0; index < names.size(); ++index) {
      if (!readInteger(value.property(names.at(index)), channels[index]) || channels[index] < 0 ||
          channels[index] > 255) {
         error =
             QStringLiteral("%1.%2 must be an integer from 0 to 255").arg(context, names.at(index));
         return false;
      }
   }
   color = QColor(channels[0], channels[1], channels[2], channels[3]);
   return true;
}

/// @brief Parses one descriptor-API setting definition.
/// @param id Stable setting identifier.
/// @param definition JavaScript setting descriptor.
/// @param setting Destination for the validated setting.
/// @param error Destination for a validation failure.
/// @return @c true when the definition has a supported type and valid constraints.
bool parseDescriptorSetting(const QString& id, const QJSValue& definition,
                            TextureGeneratorSetting& setting, QString& error) {
   const QString context = QStringLiteral("settings.%1").arg(id);
   if (!definition.isObject()) {
      error = QStringLiteral("%1 must be an object").arg(context);
      return false;
   }
   const QJSValue typeValue = definition.property(QStringLiteral("type"));
   if (!typeValue.isString()) {
      error = QStringLiteral("%1.type must be a string").arg(context);
      return false;
   }
   const QString type = typeValue.toString().toLower();
   setting.name = id;
   if (!readOptionalString(definition, QStringLiteral("name"), setting.name, error, context) ||
       !readOptionalString(definition, QStringLiteral("description"), setting.description, error,
                           context) ||
       !readOptionalString(definition, QStringLiteral("group"), setting.group, error, context) ||
       !readOptionalString(definition, QStringLiteral("enabler"), setting.enabler, error,
                           context)) {
      return false;
   }
   const QJSValue order = definition.property(QStringLiteral("order"));
   if (!order.isUndefined() && !readInteger(order, setting.order)) {
      error = QStringLiteral("%1.order must be an integer").arg(context);
      return false;
   }

   const QJSValue defaultValue = definition.property(QStringLiteral("default"));
   if (defaultValue.isUndefined()) {
      error = QStringLiteral("%1.default is required").arg(context);
      return false;
   }

   if (type == QStringLiteral("integer")) {
      int value = 0;
      if (!readInteger(defaultValue, value)) {
         error = QStringLiteral("%1.default must be an integer").arg(context);
         return false;
      }
      int minimum = std::numeric_limits<int>::min();
      int maximum = std::numeric_limits<int>::max();
      const QJSValue minValue = definition.property(QStringLiteral("min"));
      const QJSValue maxValue = definition.property(QStringLiteral("max"));
      if ((!minValue.isUndefined() && !readInteger(minValue, minimum)) ||
          (!maxValue.isUndefined() && !readInteger(maxValue, maximum)) || minimum > maximum ||
          value < minimum || value > maximum) {
         error = QStringLiteral("%1 has an invalid integer range or default").arg(context);
         return false;
      }
      setting.defaultvalue = value;
      if (!minValue.isUndefined()) {
         setting.min = minimum;
      }
      if (!maxValue.isUndefined()) {
         setting.max = maximum;
      }
      return true;
   }

   if (type == QStringLiteral("real")) {
      if (!defaultValue.isNumber() || !std::isfinite(defaultValue.toNumber())) {
         error = QStringLiteral("%1.default must be a finite number").arg(context);
         return false;
      }
      double minimum = -std::numeric_limits<double>::max();
      double maximum = std::numeric_limits<double>::max();
      const QJSValue minValue = definition.property(QStringLiteral("min"));
      const QJSValue maxValue = definition.property(QStringLiteral("max"));
      if ((!minValue.isUndefined() &&
           (!minValue.isNumber() || !std::isfinite(minimum = minValue.toNumber()))) ||
          (!maxValue.isUndefined() &&
           (!maxValue.isNumber() || !std::isfinite(maximum = maxValue.toNumber()))) ||
          minimum > maximum || defaultValue.toNumber() < minimum ||
          defaultValue.toNumber() > maximum) {
         error = QStringLiteral("%1 has an invalid real range or default").arg(context);
         return false;
      }
      setting.defaultvalue = defaultValue.toNumber();
      if (!minValue.isUndefined()) {
         setting.min = minimum;
      }
      if (!maxValue.isUndefined()) {
         setting.max = maximum;
      }
      return true;
   }

   if (type == QStringLiteral("boolean")) {
      if (!defaultValue.isBool()) {
         error = QStringLiteral("%1.default must be a boolean").arg(context);
         return false;
      }
      setting.defaultvalue = defaultValue.toBool();
      return true;
   }

   if (type == QStringLiteral("string") || type == QStringLiteral("multiline")) {
      if (!defaultValue.isString()) {
         error = QStringLiteral("%1.default must be a string").arg(context);
         return false;
      }
      setting.defaultvalue = defaultValue.toString();
      setting.multiline = type == QStringLiteral("multiline");
      return true;
   }

   if (type == QStringLiteral("color")) {
      QColor color;
      if (!parseColor(defaultValue, color, error, context + QStringLiteral(".default"))) {
         return false;
      }
      setting.defaultvalue = color;
      return true;
   }

   if (type == QStringLiteral("choice")) {
      const QJSValue valuesValue = definition.property(QStringLiteral("values"));
      if (!valuesValue.isArray() || !defaultValue.isString()) {
         error =
             QStringLiteral("%1 choice requires a string default and values array").arg(context);
         return false;
      }
      QStringList values;
      QSet<QString> uniqueValues;
      const quint32 length = valuesValue.property(QStringLiteral("length")).toUInt();
      for (quint32 index = 0; index < length; ++index) {
         const QJSValue value = valuesValue.property(index);
         if (!value.isString() || value.toString().isEmpty() ||
             uniqueValues.contains(value.toString())) {
            error = QStringLiteral("%1.values must contain unique non-empty strings").arg(context);
            return false;
         }
         values.append(value.toString());
         uniqueValues.insert(value.toString());
      }
      const int defaultIndex = values.indexOf(defaultValue.toString());
      if (values.isEmpty() || defaultIndex < 0) {
         error = QStringLiteral("%1.default must occur in values").arg(context);
         return false;
      }
      setting.defaultvalue = values;
      setting.defaultindex = defaultIndex;
      return true;
   }

   error = QStringLiteral("%1.type '%2' is unknown").arg(context, type);
   return false;
}

/// @brief Parses generator metadata from a descriptor-API definition.
/// @param descriptor JavaScript generator descriptor.
/// @param settings Destination for validated setting definitions.
/// @param name Destination for the public generator name.
/// @param description Destination for the user-facing description.
/// @param inputSlots Destination for ordered input slot names.
/// @param generatorType Destination for the add-node category.
/// @param error Destination for a validation failure.
/// @return @c true when the complete descriptor is valid.
bool parseDescriptor(const QJSValue& descriptor, TextureGeneratorSettings& settings, QString& name,
                     QString& description, QStringList& inputSlots,
                     TextureGenerator::Type& generatorType, QString& error) {
   if (!descriptor.isObject()) {
      error = QStringLiteral("generator must be an object");
      return false;
   }
   int apiVersion = 0;
   if (!readInteger(descriptor.property(QStringLiteral("apiVersion")), apiVersion) ||
       apiVersion != 1) {
      error = QStringLiteral("generator.apiVersion must be 1");
      return false;
   }
   const QJSValue nameValue = descriptor.property(QStringLiteral("name"));
   if (!nameValue.isString() || nameValue.toString().trimmed().isEmpty()) {
      error = QStringLiteral("generator.name must be a non-empty string");
      return false;
   }
   name = nameValue.toString().trimmed();
   if (!readOptionalString(descriptor, QStringLiteral("description"), description, error,
                           QStringLiteral("generator"))) {
      return false;
   }
   const QJSValue typeValue = descriptor.property(QStringLiteral("type"));
   if (!typeValue.isString()) {
      error = QStringLiteral("generator.type must be a string");
      return false;
   }
   const QString type = typeValue.toString().toLower();
   if (type == QStringLiteral("generator")) {
      generatorType = TextureGenerator::Type::Generator;
   } else if (type == QStringLiteral("filter")) {
      generatorType = TextureGenerator::Type::Filter;
   } else if (type == QStringLiteral("combiner")) {
      generatorType = TextureGenerator::Type::Combiner;
   } else {
      error = QStringLiteral("generator.type '%1' is unknown").arg(type);
      return false;
   }
   if (!descriptor.property(QStringLiteral("generate")).isCallable()) {
      error = QStringLiteral("generator.generate must be callable");
      return false;
   }

   const QJSValue inputs = descriptor.property(QStringLiteral("inputs"));
   if (!inputs.isArray()) {
      error = QStringLiteral("generator.inputs must be an array");
      return false;
   }
   QSet<QString> uniqueSlots;
   const quint32 inputCount = inputs.property(QStringLiteral("length")).toUInt();
   for (quint32 index = 0; index < inputCount; ++index) {
      const QJSValue slot = inputs.property(index);
      if (!slot.isString() || slot.toString().trimmed().isEmpty() ||
          uniqueSlots.contains(slot.toString())) {
         error = QStringLiteral("generator.inputs must contain unique non-empty strings");
         return false;
      }
      inputSlots.append(slot.toString());
      uniqueSlots.insert(slot.toString());
   }

   const QJSValue settingObject = descriptor.property(QStringLiteral("settings"));
   if (!settingObject.isUndefined() && !settingObject.isObject()) {
      error = QStringLiteral("generator.settings must be an object");
      return false;
   }
   if (!settingObject.isUndefined()) {
      QJSValueIterator iterator(settingObject);
      while (iterator.hasNext()) {
         iterator.next();
         const QString id = iterator.name();
         if (id.trimmed().isEmpty()) {
            error = QStringLiteral("setting IDs must not be empty");
            return false;
         }
         TextureGeneratorSetting setting;
         if (!parseDescriptorSetting(id, iterator.value(), setting, error)) {
            return false;
         }
         settings.insert(id, setting);
      }
   }
   return true;
}

/// @brief Calculates the byte count for a tightly packed four-channel image.
/// @param size Requested image dimensions.
/// @return Required byte count for `size.width() * size.height()` pixels.
/// @throws std::invalid_argument If either dimension is not positive.
/// @throws std::length_error If multiplication overflows or exceeds Qt array capacity.
std::size_t checkedByteCount(const QSize size) {
   if (size.width() <= 0 || size.height() <= 0) {
      throw std::invalid_argument("JavaScript image dimensions must be positive");
   }
   const std::size_t width = static_cast<std::size_t>(size.width());
   const std::size_t height = static_cast<std::size_t>(size.height());
   if (width > std::numeric_limits<std::size_t>::max() / height ||
       width * height > std::numeric_limits<std::size_t>::max() / sizeof(TexturePixel)) {
      throw std::length_error("JavaScript image dimensions overflow byte storage");
   }
   const std::size_t bytes = width * height * sizeof(TexturePixel);
   if (bytes > static_cast<std::size_t>(std::numeric_limits<qsizetype>::max())) {
      throw std::length_error("JavaScript image exceeds QByteArray capacity");
   }
   return bytes;
}

/// @brief Identifies one validated script revision in a render worker's runtime cache.
struct RuntimeKey {
   /// @brief Process-unique ID of the owning JsTexGen instance.
   quint64 stableId = 0;
   /// @brief SHA-256 source revision used to distinguish reloads.
   QByteArray revision;

   /// @brief Orders cache keys by stable ID and then source revision.
   /// @param other Key to compare against.
   /// @return @c true when this key sorts before @p other.
   bool operator<(const RuntimeKey& other) const {
      return stableId < other.stableId || (stableId == other.stableId && revision < other.revision);
   }
};

/// @brief Holds evaluated descriptor-API values owned by one worker's QJSEngine.
struct RuntimeEntry {
   /// @brief Weak token used to remove entries after their JsTexGen is destroyed.
   std::weak_ptr<void> lifetime;
   /// @brief Evaluated and frozen JavaScript generator descriptor.
   QJSValue descriptor;
   /// @brief Cached callable extracted from descriptor.
   QJSValue generate;
};

/// @brief Owns one render thread's JavaScript engine, helpers, and descriptor cache.
struct WorkerRuntime {
   /// @brief Creates the engine and installs immutable native helper functions.
   WorkerRuntime() {
      const QJSValue helpersResult = engine.evaluate(
          QStringLiteral("var TexGen = Object.freeze({"
                         "offset: (x,y,stride) => y*stride+x*4,"
                         "clamp8: value => Math.max(0,Math.min(255,Math.round(value))),"
                         "copy: (destination,source) => destination.set(source),"
                         "clear: (destination,value=0) => destination.fill(value)"
                         "});"));
      if (helpersResult.isError()) {
         throw std::runtime_error(
             jsError(helpersResult, QStringLiteral("<native helpers>")).toStdString());
      }
      freeze = engine.evaluate(QStringLiteral("value => Object.freeze(value)"));
      uint8Array = engine.globalObject().property(QStringLiteral("Uint8Array"));
   }

   /// @brief Removes cached descriptors whose owning JsTexGen no longer exists.
   void prune() {
      for (auto iterator = entries.begin(); iterator != entries.end();) {
         if (iterator->second.lifetime.expired()) {
            iterator = entries.erase(iterator);
         } else {
            ++iterator;
         }
      }
   }

   /// @brief JavaScript engine used exclusively by the current render thread.
   QJSEngine engine;
   /// @brief Cached `Object.freeze` wrapper used to make bridge objects immutable.
   QJSValue freeze;
   /// @brief Cached JavaScript `Uint8Array` constructor.
   QJSValue uint8Array;
   /// @brief Evaluated descriptors keyed by generator instance and content revision.
   std::map<RuntimeKey, RuntimeEntry> entries;
   /// @brief Number of generation calls since the last expired-entry scan.
   unsigned int callsSincePrune = 0;
};

/// @brief JavaScript runtime lazily allocated for each render worker thread.
thread_local std::unique_ptr<WorkerRuntime> workerRuntime;
/// @brief Protects activeEngines during cross-thread interruption requests.
std::mutex activeEnginesMutex;
/// @brief Engines currently executing user JavaScript.
std::set<QJSEngine*> activeEngines;

/// @brief Registers a QJSEngine as interruptible for the duration of one operation.
class ActiveEngine final {
public:
   /// @brief Clears stale interruption state and registers an executing engine.
   /// @param engine Engine about to execute user JavaScript.
   explicit ActiveEngine(QJSEngine& engine) : engine(engine) {
      engine.setInterrupted(false);
      std::lock_guard lock(activeEnginesMutex);
      activeEngines.insert(&engine);
   }

   /// @brief Unregisters the engine and clears its interruption state.
   ~ActiveEngine() {
      std::lock_guard lock(activeEnginesMutex);
      activeEngines.erase(&engine);
      engine.setInterrupted(false);
   }

private:
   /// @brief Engine registered for the lifetime of this guard.
   QJSEngine& engine;
};

/// @brief Freezes a JavaScript bridge value when `Object.freeze` succeeds.
/// @param freeze Callable equivalent of `Object.freeze`.
/// @param value Value to freeze.
/// @return The frozen value, or the original value if freezing fails.
QJSValue frozen(QJSValue freeze, const QJSValue& value) {
   const QJSValue result = freeze.call(QJSValueList{value});
   return result.isError() ? value : result;
}

/// @brief Converts a generator setting QVariant into its JavaScript representation.
/// @param engine Engine that owns the returned value.
/// @param freeze Callable used to freeze compound values.
/// @param value Setting value to convert.
/// @return A JavaScript primitive, immutable color object, or immutable string array.
QJSValue settingValue(QJSEngine& engine, const QJSValue& freeze, const QVariant& value) {
   if (value.typeId() == QMetaType::QColor) {
      const QColor color = value.value<QColor>();
      QJSValue object = engine.newObject();
      object.setProperty(QStringLiteral("r"), color.red());
      object.setProperty(QStringLiteral("g"), color.green());
      object.setProperty(QStringLiteral("b"), color.blue());
      object.setProperty(QStringLiteral("a"), color.alpha());
      return frozen(freeze, object);
   }
   if (value.typeId() == QMetaType::QStringList) {
      const QStringList values = value.toStringList();
      QJSValue array = engine.newArray(static_cast<uint>(values.size()));
      for (int index = 0; index < values.size(); ++index) {
         array.setProperty(static_cast<quint32>(index), values.at(index));
      }
      return frozen(freeze, array);
   }
   return engine.toScriptValue(value);
}

/// @brief Creates an immutable JavaScript image view backed by a QByteArray buffer.
/// @param runtime Worker runtime that owns the returned JavaScript values.
/// @param bytes Tightly packed RGBA image bytes.
/// @param size Image dimensions.
/// @param buffer Destination retaining the ArrayBuffer-compatible Qt bridge value.
/// @return An image object containing data, dimensions, stride, and format.
QJSValue imageView(WorkerRuntime& runtime, const QByteArray& bytes, const QSize size,
                   QJSValue& buffer) {
   buffer = runtime.engine.toScriptValue(bytes);
   QJSValue data = runtime.uint8Array.callAsConstructor(QJSValueList{buffer});
   if (data.isError() || data.property(QStringLiteral("length")).toNumber() != bytes.size()) {
      throw std::runtime_error("Qt could not create the JavaScript Uint8Array image bridge");
   }
   QJSValue view = runtime.engine.newObject();
   view.setProperty(QStringLiteral("data"), data);
   view.setProperty(QStringLiteral("width"), size.width());
   view.setProperty(QStringLiteral("height"), size.height());
   view.setProperty(QStringLiteral("stride"), size.width() * int(sizeof(TexturePixel)));
   view.setProperty(QStringLiteral("format"), QStringLiteral("rgba8"));
   return frozen(runtime.freeze, view);
}

/// @brief Gets or evaluates a descriptor-API runtime cache entry.
/// @param runtime Current render worker's JavaScript runtime.
/// @param stableId Process-unique ID of the generator instance.
/// @param revision SHA-256 digest of the generator source.
/// @param lifetime Token used to expire the entry after its generator is destroyed.
/// @param source Complete JavaScript source to evaluate on a cache miss.
/// @param sourceIdentity Path or resource URL used in diagnostics.
/// @param evaluated Set to @c true when this call evaluates the source.
/// @return The cached or newly evaluated runtime entry.
RuntimeEntry& runtimeEntry(WorkerRuntime& runtime, const quint64 stableId,
                           const QByteArray& revision, const std::shared_ptr<void>& lifetime,
                           const QString& source, const QString& sourceIdentity, bool& evaluated) {
   const RuntimeKey key{stableId, revision};
   auto existing = runtime.entries.find(key);
   if (existing != runtime.entries.end()) {
      return existing->second;
   }
   evaluated = true;
   ActiveEngine active(runtime.engine);
   QJSValue descriptor = runtime.engine.evaluate(descriptorProgram(source), sourceIdentity, 1);
   if (runtime.engine.isInterrupted()) {
      throw std::runtime_error(
          QStringLiteral("%1: JavaScript descriptor evaluation was interrupted")
              .arg(sourceIdentity)
              .toStdString());
   }
   if (descriptor.isError()) {
      throw std::runtime_error(
          jsError(descriptor, sourceIdentity, -descriptorWrapperLineOffset).toStdString());
   }
   const QJSValue generate = descriptor.property(QStringLiteral("generate"));
   if (!descriptor.isObject() || !generate.isCallable()) {
      throw std::runtime_error(
          QStringLiteral("%1: cached descriptor has no callable generate function")
              .arg(sourceIdentity)
              .toStdString());
   }
   auto inserted = runtime.entries.emplace(
       key, RuntimeEntry{std::weak_ptr<void>(lifetime), descriptor, generate});
   return inserted.first->second;
}

}  // namespace

JsTexGen::JsTexGen(QString jsContent, QString sourceIdentity, const Origin origin)
    : scriptContent(std::move(jsContent)),
      sourceIdentity(std::move(sourceIdentity)),
      revision(QCryptographicHash::hash(scriptContent.toUtf8(), QCryptographicHash::Sha256)),
      lifetimeToken(std::make_shared<int>(0)),
      stableId(nextStableId.fetch_add(1, std::memory_order_relaxed)),
      origin(origin) {
   validate();
}

JsTexGen::~JsTexGen() = default;

void JsTexGen::validate() {
   QJSEngine engine;
   QJSValue descriptor = engine.evaluate(descriptorProgram(scriptContent), sourceIdentity, 1);
   if (descriptor.isError()) {
      diagnostic = jsError(descriptor, sourceIdentity, -descriptorWrapperLineOffset);
      return;
   }
   if (descriptor.isUndefined()) {
      diagnostic =
          QStringLiteral("%1: generator descriptor is required; the old globals API is unsupported")
              .arg(sourceIdentity);
      return;
   }
   if (!parseDescriptor(descriptor, configurables, name, description, inputSlots, type,
                        diagnostic)) {
      diagnostic = QStringLiteral("%1: %2").arg(sourceIdentity, diagnostic);
      return;
   }
   valid = true;
}

void JsTexGen::generate(const QSize size, TexturePixel* destimage,
                        const QMap<QString, TextureImagePtr>& sourceimages,
                        const TextureNodeSettings& settings) const {
   if (!valid) {
      throw std::runtime_error(diagnostic.toStdString());
   }
   if (destimage == nullptr) {
      throw std::invalid_argument("JavaScript destination image is null");
   }
   generateDescriptor(size, destimage, sourceimages, settings);
}

void JsTexGen::generateDescriptor(const QSize size, TexturePixel* destimage,
                                  const QMap<QString, TextureImagePtr>& sourceimages,
                                  const TextureNodeSettings& settings) const {
   static_assert(sizeof(TexturePixel) == 4, "The JavaScript RGBA8 ABI requires four-byte pixels");
   const std::size_t byteCount = checkedByteCount(size);
   if (!workerRuntime) {
      workerRuntime = std::make_unique<WorkerRuntime>();
   }
   WorkerRuntime& runtime = *workerRuntime;
   if (++runtime.callsSincePrune >= 32) {
      runtime.prune();
      runtime.callsSincePrune = 0;
   }
   bool evaluated = false;
   RuntimeEntry& entry = ::runtimeEntry(runtime, stableId, revision, lifetimeToken, scriptContent,
                                        sourceIdentity, evaluated);
   if (evaluated) {
      evaluationCount.fetch_add(1, std::memory_order_relaxed);
   }

   QJSValue sizeObject = runtime.engine.newObject();
   sizeObject.setProperty(QStringLiteral("width"), size.width());
   sizeObject.setProperty(QStringLiteral("height"), size.height());
   sizeObject = frozen(runtime.freeze, sizeObject);

   QJSValue settingsObject = runtime.engine.newObject();
   for (auto iterator = settings.cbegin(); iterator != settings.cend(); ++iterator) {
      settingsObject.setProperty(iterator.key(),
                                 settingValue(runtime.engine, runtime.freeze, iterator.value()));
   }
   settingsObject = frozen(runtime.freeze, settingsObject);

   QByteArray outputBytes(static_cast<qsizetype>(byteCount), '\0');
   QJSValue outputBuffer;
   const QJSValue output = imageView(runtime, outputBytes, size, outputBuffer);

   QJSValue inputs = runtime.engine.newObject();
   QList<QJSValue> inputBuffers;
   inputBuffers.reserve(sourceimages.size());
   for (const QString& slot : inputSlots) {
      const TextureImagePtr source = sourceimages.value(slot);
      if (source.isNull()) {
         continue;
      }
      if (source->getSize() != size || source->byteSize() != byteCount) {
         throw std::runtime_error(
             QStringLiteral("%1: source '%2' dimensions do not match the output")
                 .arg(sourceIdentity, slot)
                 .toStdString());
      }
      const QByteArray sourceBytes(reinterpret_cast<const char*>(source->data()),
                                   static_cast<qsizetype>(byteCount));
      QJSValue sourceBuffer;
      inputs.setProperty(slot, imageView(runtime, sourceBytes, size, sourceBuffer));
      inputBuffers.append(sourceBuffer);
   }
   inputs = frozen(runtime.freeze, inputs);

   ActiveEngine active(runtime.engine);
   const QJSValue result = entry.generate.callWithInstance(
       entry.descriptor, QJSValueList{sizeObject, settingsObject, output, inputs});
   if (runtime.engine.isInterrupted()) {
      throw std::runtime_error(QStringLiteral("%1: JavaScript generator '%2' was interrupted")
                                   .arg(sourceIdentity, name)
                                   .toStdString());
   }
   if (result.isError()) {
      throw std::runtime_error(
          QStringLiteral("%1 [%2]\n%3")
              .arg(name, sourceIdentity,
                   jsError(result, sourceIdentity, -descriptorWrapperLineOffset))
              .toStdString());
   }

   const QByteArray rendered = runtime.engine.fromScriptValue<QByteArray>(outputBuffer);
   if (rendered.size() != static_cast<qsizetype>(byteCount)) {
      throw std::runtime_error(QStringLiteral("%1: JavaScript output buffer has an invalid size")
                                   .arg(sourceIdentity)
                                   .toStdString());
   }
   std::memcpy(destimage, rendered.constData(), byteCount);
}

void JsTexGen::interruptActiveEngines() {
   std::lock_guard lock(activeEnginesMutex);
   for (QJSEngine* engine : activeEngines) {
      engine->setInterrupted(true);
   }
}

quint64 JsTexGen::runtimeEvaluationCount() noexcept {
   return evaluationCount.load(std::memory_order_relaxed);
}
