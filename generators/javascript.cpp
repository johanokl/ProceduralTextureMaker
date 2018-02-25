/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QDirIterator>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "core/textureproject.h"
#include "core/settingsmanager.h"
#include "javascript.h"

#ifndef DISABLE_JAVASCRIPT
#ifdef USE_QJSENGINE
   #include <QJSEngine>
   #include <QJSValueIterator>
   #define QScriptEngine QJSEngine
   #define QScriptValue QJSValue
   #define QScriptValueIterator QJSValueIterator
   #define toInt32 toInt
   #define toUInt32 toUInt
   #define toUInt16 toUInt
   #define QScriptValueList QJSValueList
   #define SCRIPT_PARAMS args
#else
   #include <QtScript/QScriptEngine>
   #include <QtScript/QScriptValueIterator>
   #define SCRIPT_PARAMS QScriptValue(), args
#endif
#endif


/**
 * @brief JsTexGen::JsTexGen
 * @param jsContent The script's content
 * Parses the Javascript file and checks if it's a Texture Generator.
 * Creates the settings object based on the result.
 */
JsTexGen::JsTexGen(QString jsContent) {
   valid = false;
   description = "";
   scriptContent = jsContent;
   numSlots = 0;
   separateColorChannels = false;

#ifndef DISABLE_JAVASCRIPT
   QScriptEngine jsEngine;
   jsEngine.globalObject().setProperty("name", "");
   QScriptValue retVal = jsEngine.evaluate(scriptContent);
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
   QScriptValue settings = jsEngine.globalObject().property("getSettings").call();
   if (settings.isError()) {
      qDebug() << "Error: " << name << "::getSettings error " << settings.property("message").toString();
      return;
   }
   QScriptValueIterator it(settings);
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
         QScriptValue defaultvalue = it.value().property("defaultvalue");
         if (type == "color") {
            newsetting.defaultvalue =
                  QVariant(QColor(defaultvalue.property("r").toInt32(), defaultvalue.property("g").toInt32(),
                                  defaultvalue.property("b").toInt32(), defaultvalue.property("a").toInt32()));
         } else if (type == "integer" || type == "int") {
            newsetting.defaultvalue = QVariant(defaultvalue.toInt32());
            if (!it.value().property("min").isUndefined()) {
               newsetting.min = it.value().property("min").toInt32();
            }
            if (!it.value().property("max").isUndefined()) {
               newsetting.max = it.value().property("max").toInt32();
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
         newsetting.order = it.value().property("order").toInt32();
      }
      if (!settingsname.isEmpty()) {
         configurables.insert(settingsname, newsetting);
      }
   }
   if (!jsEngine.globalObject().property("numSlots").isUndefined()) {
      numSlots = jsEngine.globalObject().property("numSlots").toInt32();
   }
   if (!jsEngine.globalObject().property("separateColorChannels").isUndefined()) {
      separateColorChannels = jsEngine.globalObject().property("separateColorChannels").toBool();
   }
   jsEngine.collectGarbage();
   valid = true;
#endif
}

/**
 * @brief JsTexGen::isValid
 * @return true if the instance is a valid TextureGenerator.
 */
bool JsTexGen::isValid() {
   return valid;
}

/**
 * @brief JsTexGen::generate
 *
 * Implements the TextureGenerator's abstract generate
 * Calls the Javascript class's generate function.
 * The settings are encoded as one JSON text object.
 * The source images are sent as array of 32bit RGBA pixels.
 * The result image are returned from the JS file either as
 * an array with 32 bit RGBA pixels or as a JSON object
 * with each color in one position [r, g, b, a, r, g, b...].
 */
void JsTexGen::generate(QSize size, TexturePixel* destimage,
                        QMap<int, TextureImagePtr> sourceimages,
                        TextureNodeSettings* settings) const
{
#ifndef DISABLE_JAVASCRIPT
   mutex.lockForWrite();
   QScriptEngine jsEngine;
   QScriptValue parseResult = jsEngine.evaluate(scriptContent);
   if (parseResult.isError()) {
      qDebug() << "Error!";
      return;
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
      switch (newVal.type()) {
      case QVariant::Type::Int:
         settingsJson.insert(settingsName, newVal.toInt());
         break;
      case QVariant::Type::Double:
         settingsJson.insert(settingsName, newVal.toDouble());
         break;
      case QVariant::Type::String:
         settingsJson.insert(settingsName, newVal.toString());
      case QVariant::Type::Color:
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
   int imageSize = size.width() * size.height();

   QJsonDocument settingsJsonDoc(settingsJson);
   QString settingsJsonStr(settingsJsonDoc.toJson(QJsonDocument::Compact));
   QScriptValueList args;
   args << settingsJsonStr;

   int arraySize = imageSize;
   if (separateColorChannels) {
      arraySize *= 4;
   }

   // Add all source images.
   QMapIterator<int, TextureImagePtr> sourceIterator(sourceimages);
   while (sourceIterator.hasNext()) {
      sourceIterator.next();
      QScriptValue srcArray = jsEngine.newArray(arraySize);
      if (separateColorChannels) {
         uint8_t* dataptr = (uint8_t*) sourceIterator.value()->getData();
         for (int i = 0; i < arraySize; i++) {
            srcArray.setProperty(i, dataptr[i]);
         }
      } else {
         TexturePixel* dataptr = sourceIterator.value()->getData();
         for (int i = 0; i < arraySize; i++) {
            srcArray.setProperty(i, dataptr[i].toRGBA());
         }
      }
      args << srcArray;
   }

   QScriptValue retArray = jsEngine.newArray(arraySize);
   for (int i = 0; i < arraySize; i++) {
      retArray.setProperty(i, 0);
   }
   jsEngine.globalObject().setProperty("dest", retArray);
   QScriptValue retVal;
   try {
      retArray = jsEngine.globalObject().property("generate").call(SCRIPT_PARAMS);
      if (retArray.isError()) {
         qDebug() << "Error!";
         qDebug() << retVal.toString();
         memset(destimage, 0, imageSize * sizeof(TexturePixel));
         mutex.unlock();
         return;
      }
   } catch (...) {
      qDebug() << "Exception";
      memset(destimage, 0, imageSize * sizeof(TexturePixel));
      mutex.unlock();
      return;
   }
   unsigned char* dstchar = (unsigned char*) destimage;
   if (retVal.property("image").isString()) {
      QString imgStr = retVal.property("image").toString();
      QJsonArray imgArray = QJsonDocument::fromJson(imgStr.toUtf8()).array();
      int arraySize = qMin(imageSize * (int) sizeof(TexturePixel), imgArray.size());
      memset(destimage, 0, imageSize * sizeof(TexturePixel));
      for (int i = 0; i < arraySize; i++) {
         dstchar[i] = (unsigned char) imgArray[i].toInt(0);
      }
   } else if (separateColorChannels) {
      for (int i = 0; i < arraySize; i++) {
         dstchar[i] = (unsigned char) retArray.property(i).toUInt16();
      }
   } else {
      for (int i = 0; i < imageSize; i++) {
         quint32 color = retArray.property(i).toUInt32();
         destimage[i].r = (unsigned char) ((color) >> 24);
         destimage[i].g = (unsigned char) ((color << 8) >> 24);
         destimage[i].b = (unsigned char) ((color << 16) >> 24);
         destimage[i].a = (unsigned char) ((color << 24) >> 24);
      }
   }
   jsEngine.collectGarbage();
   mutex.unlock();
#else
   Q_UNUSED(size);
   Q_UNUSED(destimage);
   Q_UNUSED(sourceimages);
   Q_UNUSED(settings);
#endif
}

/**
 * @brief GeneratorFileFinder::abort
 * Abort the scanning run from a different thread.
 */
void GeneratorFileFinder::abort()
{
   aborted = true;
}

/**
 * @brief GeneratorFileFinder::scanDirectory
 * @param basepath Directory to search recursively.
 *
 * Scans the selected directory and its sub-directories for Javascript files.
 * Files found are then checked whether they are TextureGenerator classes, and
 * if so are packaged into a JsTexGen instance and emitted as a signal.
 */
void GeneratorFileFinder::scanDirectory(QString basepath) {
   aborted = false;
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
      if (aborted) {
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
         JsTexGen* newTexGen = new JsTexGen(contents);
         if (newTexGen->isValid()) {
            emit generatorFound(newTexGen);
         } else {
            delete newTexGen;
         }
      }
   }
   emit scanFinished();
}

/**
 * @brief JSTexGenManager::JSTexGenManager
 * @param project
 * Creates a thread for the JS file finder.
 */
JSTexGenManager::JSTexGenManager(TextureProject *project)
{
   this->project = project;
   directoryPath = "";
   enabled = false;
   hasScannedDirectory = false;
   filefinder = new GeneratorFileFinder;
   filefinderthread = new QThread;
   filefinder->moveToThread(filefinderthread);
   filefinderthread->start();
   QObject::connect(this, &JSTexGenManager::scanDirectory,
                    filefinder, &GeneratorFileFinder::scanDirectory);
   QObject::connect(filefinder, &GeneratorFileFinder::generatorFound,
                    this, &JSTexGenManager::addGenerator);
   QObject::connect(filefinderthread, &QThread::finished,
                    filefinderthread, &QThread::deleteLater);
   QObject::connect(project->getSettingsManager(), &SettingsManager::settingsUpdated,
                    this, &JSTexGenManager::settingsUpdated);
   QObject::connect(this, &JSTexGenManager::generatorAdded,
                    project, &TextureProject::addGenerator);
   settingsUpdated();
}

/**
 * @brief JSTexGenManager::~JSTexGenManager
 */
JSTexGenManager::~JSTexGenManager()
{
   filefinder->abort();
   filefinderthread->quit();
}

/**
 * @brief JSTexGenManager::addGenerator
 * @param generator
 * Emits a signal that a new generator has been found.
 */
void JSTexGenManager::addGenerator(JsTexGen* generator)
{
   emit generatorAdded(TextureGeneratorPtr(generator));
}

/**
 * @brief JSTexGenManager::setEnabled
 * @param enabled
 * Sets whether it should scan the selected directory for JS files.
 */
void JSTexGenManager::setEnabled(bool enabled)
{
   this->enabled = enabled;
   if (enabled && !hasScannedDirectory) {
      setDirectory(directoryPath, true);
   }
}

/**
 * @brief JSTexGenManager::settingsUpdated
 * Updates the settings for the path and if the module's enabled.
 */
void JSTexGenManager::settingsUpdated()
{
   setEnabled(project->getSettingsManager()->getJSTextureGeneratorsEnabled());
   setDirectory(project->getSettingsManager()->getJSTextureGeneratorsPath());
}

/**
 * @brief JSTexGenManager::setDirectory
 * @param path Absolute path to scan.
 * @param forceScan True to force directory scan.
 */
void JSTexGenManager::setDirectory(QString path, bool forceScan)
{
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
