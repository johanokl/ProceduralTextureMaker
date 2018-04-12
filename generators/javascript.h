/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef JSTEXGENMANAGER_H
#define JSTEXGENMANAGER_H

#include "texturegenerator.h"
#include <QObject>
#include <QReadWriteLock>

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
   explicit JSTexGenManager(TextureProject* project);
   ~JSTexGenManager() override;

public slots:
   void setDirectory(const QString& path, bool forceScan=false);
   void addGenerator(JsTexGen* generator);
   void setEnabled(bool);
   void settingsUpdated();

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
   explicit JsTexGen(const QString& jsContent);
    ~JsTexGen() override = default;
   void generate(QSize size, TexturePixel* destimage,
                 QMap<int, TextureImagePtr> sourceimages,
                 TextureNodeSettings* settings) const override;

   int getNumSourceSlots() const override { return numSlots; }
   QString getName() const override { return name; }
   const TextureGeneratorSettings& getSettings() const override { return configurables; }
   QString getDescription() const override { return description; }
   TextureGenerator::Type getType() const override { return TextureGenerator::Type::Generator; }
   bool isValid();

private:
   TextureGeneratorSettings configurables;
   QString name;
   QString description;
   QString scriptContent;
   mutable QReadWriteLock mutex;
   int numSlots;
   bool valid;
   bool separateColorChannels;
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
