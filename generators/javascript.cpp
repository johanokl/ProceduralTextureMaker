
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/settingsmanager.h"
#include "base/textureproject.h"
#include "javascript.h"
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QMetaType>
#include <QSet>
#include <QFile>
#include <QString>
#include <QThread>
#include <QWriteLocker>
#include <stdexcept>
#include <utility>

JsTexGen::JsTexGen(const QString& jsContent) : scriptContent(jsContent) {
   valid = false;
   description = "";
   inputSlots.clear();
   legacyPositionalInputs = false;
   separateColorChannels = false;

   QJSEngine jsEngine;
   jsEngine.globalObject().setProperty("name", "");
   QJSValue retVal = jsEngine.evaluate(scriptContent);
   if (retVal.isError()) {
      qDebug() << "Error!";
      return;
   }
   name = jsEngine.globalObject().property("name").toString();
   if (name.isEmpty()) {
      qDebug() << "No object with name " << name;
      return;
   }
   if (jsEngine.globalObject().property("generate").isUndefined() ||
       jsEngine.globalObject().property("getSettings").isUndefined()) {
      qDebug() << "Error: " << name << " lacks functions generate and or getSettings";
      return;
   }
   QJSValue settings = jsEngine.globalObject().property("getSettings").call();
   if (settings.isError()) {
      qDebug() << "Error: " << name << "::getSettings error "
               << settings.property("message").toString();
      return;
   }
   QJSValueIterator it(settings);
   while (it.hasNext()) {
      it.next();
      TextureGeneratorSetting newsetting;
      QString settingsname = it.name();
      newsetting.name = settingsname;
      if (!it.value().property("name").isUndefined()) {
         newsetting.name = it.value().property("name").toString();
      }
      if (!it.value().property("description").isUndefined()) {
         newsetting.description = it.value().property("description").toString();
      }
      QString type;
      if (!it.value().property("type").isUndefined()) {
         type = it.value().property("type").toString();
      }
      if (!it.value().property("defaultvalue").isUndefined()) {
         QJSValue defaultvalue = it.value().property("defaultvalue");
         if (type == "color") {
            newsetting.defaultvalue = QVariant(
                QColor(defaultvalue.property("r").toInt(), defaultvalue.property("g").toInt(),
                       defaultvalue.property("b").toInt(), defaultvalue.property("a").toInt()));
         } else if (type == "integer" || type == "int") {
            newsetting.defaultvalue = QVariant(defaultvalue.toInt());
            if (!it.value().property("min").isUndefined()) {
               newsetting.min = it.value().property("min").toInt();
            }
            if (!it.value().property("max").isUndefined()) {
               newsetting.max = it.value().property("max").toInt();
            }
         } else if (type == "double" || type == "real") {
            newsetting.defaultvalue = QVariant(defaultvalue.toNumber());
            if (!it.value().property("min").isUndefined()) {
               newsetting.min = it.value().property("min").toNumber();
            }
            if (!it.value().property("max").isUndefined()) {
               newsetting.max = it.value().property("max").toNumber();
            }
         }
      }
      if (!it.value().property("order").isUndefined()) {
         newsetting.order = it.value().property("order").toInt();
      }
      if (!settingsname.isEmpty()) {
         configurables.insert(settingsname, newsetting);
      }
   }
   const QJSValue inputSlotFunction = jsEngine.globalObject().property("getInputSlots");
   if (!inputSlotFunction.isUndefined()) {
      const QJSValue declaredSlots = inputSlotFunction.call();
      if (declaredSlots.isError() || !declaredSlots.isArray()) {
         qDebug() << "Error:" << name << "has an invalid getInputSlots result";
         return;
      }
      QSet<QString> uniqueSlots;
      const quint32 slotCount = declaredSlots.property("length").toUInt();
      for (quint32 i = 0; i < slotCount; ++i) {
         const QJSValue declaredSlot = declaredSlots.property(i);
         if (!declaredSlot.isString()) {
            qDebug() << "Error:" << name << "has a non-string input slot";
            return;
         }
         const QString slotName = declaredSlot.toString();
         if (slotName.trimmed().isEmpty() || uniqueSlots.contains(slotName)) {
            qDebug() << "Error:" << name << "has an empty or duplicate input slot";
            return;
         }
         uniqueSlots.insert(slotName);
         inputSlots.append(slotName);
      }
   } else {
      legacyPositionalInputs = true;
      int numSlots = 0;
      if (!jsEngine.globalObject().property("numSlots").isUndefined()) {
         numSlots = jsEngine.globalObject().property("numSlots").toInt();
      }
      if (numSlots < 0) {
         qDebug() << "Error:" << name << "has a negative numSlots value";
         return;
      }
      if (numSlots == 1) {
         inputSlots.append(QStringLiteral("Input"));
      } else {
         for (int i = 1; i <= numSlots; ++i) {
            inputSlots.append(QStringLiteral("Input %1").arg(i));
         }
      }
   }
   if (!jsEngine.globalObject().property("separateColorChannels").isUndefined()) {
      separateColorChannels = jsEngine.globalObject().property("separateColorChannels").toBool();
   }
   jsEngine.collectGarbage();
   valid = true;
}

bool JsTexGen::isValid() { return valid; }

void JsTexGen::generate(QSize size, TexturePixel* destimage,
                        QMap<QString, TextureImagePtr> sourceimages,
                        TextureNodeSettings* settings) const {
   QWriteLocker lock(&mutex);
   QJSEngine jsEngine;
   QJSValue parseResult = jsEngine.evaluate(scriptContent);
   if (parseResult.isError()) {
      throw std::runtime_error(parseResult.toString().toStdString());
   }
   QList<QString> keys = settings->keys();
   QListIterator<QString> listIterator(keys);
   QJsonObject settingsJson;
   QColor color;
   QVariantMap col;
   // Create JSON object with all the setting values.
   while (listIterator.hasNext()) {
      QString settingsName = listIterator.next();
      QVariant newVal = settings->value(settingsName);
      switch (newVal.typeId()) {
         case QMetaType::Int:
            settingsJson.insert(settingsName, newVal.toInt());
            break;
         case QMetaType::Double:
            settingsJson.insert(settingsName, newVal.toDouble());
            break;
         case QMetaType::QString:
            settingsJson.insert(settingsName, newVal.toString());
            break;
         case QMetaType::QColor:
            color = newVal.value<QColor>();
            col.clear();
            col.insert("r", color.red());
            col.insert("g", color.green());
            col.insert("b", color.blue());
            col.insert("a", color.alpha());
            settingsJson.insert(settingsName, settingsJson.fromVariantMap(col));
            break;
         default:
            break;
      }
   }
   settingsJson.insert("imagewidth", size.width());
   settingsJson.insert("imageheight", size.height());
   auto imageSize = static_cast<quint32>(size.width() * size.height());

   QJsonDocument settingsJsonDoc(settingsJson);
   QString settingsJsonStr(settingsJsonDoc.toJson(QJsonDocument::Compact));
   QJSValueList args;
   args << settingsJsonStr;

   auto arraySize = imageSize;
   if (separateColorChannels) {
      arraySize *= 4;
   }

   const auto imageArray = [&](const TextureImagePtr& sourceImage) {
      QJSValue srcArray = jsEngine.newArray(arraySize);
      if (separateColorChannels) {
         auto* dataptr = reinterpret_cast<uint8_t*>(sourceImage->getData());
         for (quint32 i = 0; i < arraySize; i++) {
            srcArray.setProperty(i, dataptr[i]);
         }
      } else {
         auto* dataptr = sourceImage->getData();
         for (quint32 i = 0; i < arraySize; i++) {
            srcArray.setProperty(i, dataptr[i].toRGBA());
         }
      }
      return srcArray;
   };

   if (legacyPositionalInputs) {
      for (const QString& slot : inputSlots) {
         args << (sourceimages.contains(slot) ? imageArray(sourceimages.value(slot))
                                              : QJSValue(QJSValue::UndefinedValue));
      }
   } else {
      QJSValue inputsObject = jsEngine.newObject();
      for (const QString& slot : inputSlots) {
         if (sourceimages.contains(slot)) {
            inputsObject.setProperty(slot, imageArray(sourceimages.value(slot)));
         }
      }
      args << inputsObject;
   }

   QJSValue retArray = jsEngine.newArray(arraySize);
   for (quint32 i = 0; i < arraySize; i++) {
      retArray.setProperty(i, 0);
   }
   jsEngine.globalObject().setProperty("dest", retArray);
   QJSValue retVal;
   try {
      retVal = jsEngine.globalObject().property("generate").call(args);
      if (retVal.isError()) {
         throw std::runtime_error(retVal.toString().toStdString());
      }
   } catch (const std::exception&) {
      throw;
   } catch (...) {
      throw std::runtime_error("Unknown JavaScript generator failure");
   }
   auto* dstchar = reinterpret_cast<unsigned char*>(destimage);
   if (retVal.property("image").isString()) {
      QString imgStr = retVal.property("image").toString();
      QJsonArray imgArray = QJsonDocument::fromJson(imgStr.toUtf8()).array();
      const int requiredBytes = static_cast<int>(imageSize * sizeof(TexturePixel));
      if (imgArray.size() < requiredBytes) {
         throw std::runtime_error("JavaScript generator returned incomplete image JSON");
      }
      memset(destimage, 0, imageSize * sizeof(TexturePixel));
      for (int i = 0; i < requiredBytes; i++) {
         dstchar[i] = static_cast<unsigned char>(imgArray[i].toInt(0));
      }
   } else if (!retVal.isArray() || retVal.property("length").toUInt() < arraySize) {
      throw std::runtime_error("JavaScript generator returned an incomplete image array");
   } else if (separateColorChannels) {
      for (quint32 i = 0; i < arraySize; i++) {
         dstchar[i] = static_cast<unsigned char>(retVal.property(i).toUInt());
      }
   } else {
      for (quint32 i = 0; i < imageSize; i++) {
         quint32 color = retVal.property(i).toUInt();
         destimage[i].r = static_cast<unsigned char>((color) >> 24);
         destimage[i].g = static_cast<unsigned char>((color << 8) >> 24);
         destimage[i].b = static_cast<unsigned char>((color << 16) >> 24);
         destimage[i].a = static_cast<unsigned char>((color << 24) >> 24);
      }
   }
   jsEngine.collectGarbage();
}

void GeneratorFileFinder::abort() { aborted.store(true); }

void GeneratorFileFinder::scanDirectory(QString basepath) {
   aborted.store(false);
   if (!basepath.isEmpty() && basepath.right(1) != QDir::separator()) {
      basepath += QDir::separator();
   }
   QDir dir(basepath);
   if (basepath.isEmpty() || !dir.exists()) {
      emit scanFinished();
      return;
   }
   QDirIterator iterator(basepath, QDirIterator::Subdirectories);
   while (iterator.hasNext()) {
      if (aborted.load()) {
         // Scanning was aborted by the caller
         emit scanFinished();
         return;
      }
      iterator.next();
      if (!iterator.fileInfo().isDir() && iterator.fileInfo().suffix() == "js") {
         QString filename = iterator.fileInfo().absoluteFilePath();
         QFile scriptFile(filename);
         if (!scriptFile.open(QIODevice::ReadOnly)) {
            return;
         }
         QTextStream stream(&scriptFile);
         QString contents = stream.readAll();
         scriptFile.close();
         auto* newTexGen = new JsTexGen(contents);
         if (newTexGen->isValid()) {
            emit generatorFound(newTexGen);
         } else {
            delete newTexGen;
         }
      }
   }
   emit scanFinished();
}

JSTexGenManager::JSTexGenManager(TextureProject* project) {
   this->project = project;
   directoryPath = "";
   enabled = false;
   hasScannedDirectory = false;
   filefinder = new GeneratorFileFinder;
   filefinderthread = new QThread;
   filefinder->moveToThread(filefinderthread);
   filefinderthread->start();
   QObject::connect(this, &JSTexGenManager::scanDirectory, filefinder,
                    &GeneratorFileFinder::scanDirectory);
   QObject::connect(filefinder, &GeneratorFileFinder::generatorFound, this,
                    &JSTexGenManager::addGenerator);
   QObject::connect(filefinderthread, &QThread::finished, filefinder,
                    &GeneratorFileFinder::deleteLater);
   QObject::connect(project->getSettingsManager(), &SettingsManager::settingsUpdated, this,
                    &JSTexGenManager::settingsUpdated);
   QObject::connect(this, &JSTexGenManager::generatorAdded, project, &TextureProject::addGenerator);
   settingsUpdated();
}

JSTexGenManager::~JSTexGenManager() {
   filefinder->abort();
   filefinderthread->quit();
   filefinderthread->wait();
   delete filefinderthread;
}

void JSTexGenManager::addGenerator(JsTexGen* generator) {
   emit generatorAdded(TextureGeneratorPtr(generator));
}

void JSTexGenManager::setEnabled(bool enabled) {
   this->enabled = enabled;
   if (enabled && !hasScannedDirectory) {
      setDirectory(directoryPath, true);
   }
}

void JSTexGenManager::settingsUpdated() {
   setEnabled(project->getSettingsManager()->getJSTextureGeneratorsEnabled());
   setDirectory(project->getSettingsManager()->getJSTextureGeneratorsPath());
}

void JSTexGenManager::setDirectory(const QString& path, bool forceScan) {
   if (!forceScan && directoryPath == path) {
      return;
   }
   hasScannedDirectory = false;
   directoryPath = path;
   if (enabled) {
      emit scanDirectory(path);
      hasScannedDirectory = true;
   }
}

QString loadJavaScriptGenerators(TextureProject& project, const QString& directory) {
   const QDir sourceDirectory(directory);
   if (!sourceDirectory.exists()) {
      return QStringLiteral("JavaScript generator directory does not exist: %1").arg(directory);
   }

   QDirIterator iterator(directory, QStringList{QStringLiteral("*.js")}, QDir::Files,
                         QDirIterator::Subdirectories);
   while (iterator.hasNext()) {
      const QString path = iterator.next();
      QFile file(path);
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return QStringLiteral("Could not read JavaScript generator '%1': %2")
             .arg(path, file.errorString());
      }
      auto* generator = new JsTexGen(QString::fromUtf8(file.readAll()));
      if (!generator->isValid()) {
         delete generator;
         return QStringLiteral("Invalid JavaScript generator: %1").arg(path);
      }
      if (!project.getGenerator(generator->getName()).isNull()) {
         const QString error =
             QStringLiteral("Duplicate generator name '%1' in %2").arg(generator->getName(), path);
         delete generator;
         return error;
      }
      project.addGenerator(TextureGeneratorPtr(generator));
   }
   return {};
}
