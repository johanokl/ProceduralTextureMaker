/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef JSTEXGENMANAGER_H
#define JSTEXGENMANAGER_H

#include <QObject>
#include <QSet>
#include <QReadWriteLock>
#include "texturegenerator.h"

class GeneratorFileFinder;
class JsTexGen;
class TextureProject;

/**
 * @brief The JSTexGenManager class
 */
class JSTexGenManager : public QObject
{
   Q_OBJECT

public:
   explicit JSTexGenManager(TextureProject *project);
   virtual ~JSTexGenManager();

public slots:
   void setDirectory(QString path, bool forceScan=false);
   void addGenerator(JsTexGen* generator);
   void setEnabled(bool);
   void settingsUpdated(void);

signals:
   void generatorAdded(TextureGeneratorPtr);
   void generatorRemoved(TextureGeneratorPtr);
   void scanDirectory(QString);

private:
   QString directoryPath;
   GeneratorFileFinder* filefinder;
   QThread* filefinderthread;
   TextureProject* project;
   bool enabled;
   bool hasScannedDirectory;
};

/**
 * @brief The JsTexGen class
 */
class JsTexGen : public TextureGenerator
{
public:
   JsTexGen(QString jsContent);
   virtual ~JsTexGen() {}
   virtual void generate(QSize size, TexturePixel* destimage,
                         QMap<int, TextureImagePtr> sourceimages,
                         TextureNodeSettings* settings) const;

   virtual int getNumSourceSlots() const { return numSlots; }
   virtual QString getName() const { return name; }
   virtual const TextureGeneratorSettings& getSettings() const { return configurables; }
   virtual QString getDescription() const { return description; }
   virtual TextureGenerator::Type getType() const { return TextureGenerator::Type::Generator; }
   bool isValid();

private:
   TextureGeneratorSettings configurables;
   QString name;
   QString description;
   QString scriptContent;
   bool valid;
   int numSlots;
   bool separateColorChannels;
   mutable QReadWriteLock mutex;
};

/**
 * @brief The GeneratorFileFinder class
 */
class GeneratorFileFinder : public QObject
{
   Q_OBJECT
public slots:
   void abort();
   void scanDirectory(QString basepath);
signals:
   void scanFinished();
   void generatorFound(JsTexGen* filename);
private:
   bool aborted;
};

#endif // JSTEXGENMANAGER_H
