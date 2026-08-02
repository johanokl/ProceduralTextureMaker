// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "projectfileservice.h"
#include "generators/texturegenerator.h"
#include "textureproject.h"
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QSaveFile>
#include <QSet>
#include <exception>
#include <utility>

namespace {

/// @brief Creates a failed project-file operation result.
/// @param error Failure category.
/// @param message Human-readable failure description.
/// @return Result containing the supplied error and message.
ProjectFileResult failure(const ProjectFileError error, QString message) {
   return ProjectFileResult{error, std::move(message)};
}

/// @brief Reads a strictly positive integer attribute from an XML element.
/// @param element Element containing the attribute.
/// @param name Attribute name.
/// @param value Destination for the parsed integer.
/// @return True when the attribute contains a positive integer.
bool positiveIntegerAttribute(const QDomElement& element, const QString& name, int& value) {
   bool ok = false;
   value = element.attribute(name).toInt(&ok);
   return ok && value > 0;
}

/// @brief Removes empty legacy setting elements that current loaders cannot interpret.
/// @param document Project document to normalize in place.
void removeLegacyEmptySettings(QDomDocument& document) {
   QDomElement nodes = document.documentElement().firstChildElement(QStringLiteral("Nodes"));
   for (QDomElement node = nodes.firstChildElement(QStringLiteral("Node")); !node.isNull();
        node = node.nextSiblingElement(QStringLiteral("Node"))) {
      QDomElement settings = node.firstChildElement(QStringLiteral("Settings"));
      for (QDomElement setting = settings.firstChildElement(QStringLiteral("setting"));
           !setting.isNull();) {
         const QDomElement next = setting.nextSiblingElement(QStringLiteral("setting"));
         if (setting.attribute(QStringLiteral("type")).isEmpty() &&
             setting.attribute(QStringLiteral("value")).isEmpty()) {
            settings.removeChild(setting);
         }
         setting = next;
      }
   }
}

}  // namespace

/// @brief Loads and validates a project from an XML file.
/// @param path Path of the project file to read.
/// @param project Project that receives the loaded graph.
/// @return The operation result, including an error message on failure.
ProjectFileResult ProjectFileService::load(const QString& path, TextureProject& project) {
   QFile input(path);
   if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return failure(ProjectFileError::InputOpen,
                     QStringLiteral("Could not open '%1': %2").arg(path, input.errorString()));
   }

   QDomDocument document;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
   const QDomDocument::ParseResult parseResult = document.setContent(&input);
   if (!parseResult) {
      return failure(ProjectFileError::XmlParse, QStringLiteral("XML parse error at %1:%2: %3")
                                                     .arg(parseResult.errorLine)
                                                     .arg(parseResult.errorColumn)
                                                     .arg(parseResult.errorMessage));
   }
#else
   QString errorMessage;
   int errorLine = 0;
   int errorColumn = 0;
   if (!document.setContent(&input, &errorMessage, &errorLine, &errorColumn)) {
      return failure(ProjectFileError::XmlParse, QStringLiteral("XML parse error at %1:%2: %3")
                                                     .arg(errorLine)
                                                     .arg(errorColumn)
                                                     .arg(errorMessage));
   }
#endif
   return loadDocument(document, project);
}

/// @brief Validates the structure and references of a parsed project document.
/// @param document Parsed project document to validate.
/// @param project Project providing the available generator registry.
/// @return Success when the document is valid, otherwise a validation failure.
ProjectFileResult ProjectFileService::validate(const QDomDocument& document,
                                               const TextureProject& project) {
   const QDomElement root = document.documentElement();
   if (root.isNull() || root.tagName() != QStringLiteral("TextureSet")) {
      return failure(ProjectFileError::Validation,
                     QStringLiteral("The document root must be TextureSet"));
   }
   const QDomElement nodesElement = root.firstChildElement(QStringLiteral("Nodes"));
   if (nodesElement.isNull()) {
      return failure(ProjectFileError::Validation,
                     QStringLiteral("The document does not contain a Nodes element"));
   }

   QMap<int, QMap<QString, int>> sourcesByNode;

   for (QDomElement node = nodesElement.firstChildElement(); !node.isNull();
        node = node.nextSiblingElement()) {
      if (node.tagName() != QStringLiteral("Node")) {
         continue;
      }
      int nodeId = 0;
      if (!positiveIntegerAttribute(node, QStringLiteral("id"), nodeId)) {
         return failure(ProjectFileError::Validation,
                        QStringLiteral("Every Node must have a positive integer id"));
      }
      if (sourcesByNode.contains(nodeId)) {
         return failure(ProjectFileError::Validation,
                        QStringLiteral("Duplicate node id %1").arg(nodeId));
      }
      const QDomElement generatorElement = node.firstChildElement(QStringLiteral("generator"));
      const QString generatorName = generatorElement.attribute(QStringLiteral("name"));
      const TextureGeneratorPtr generator = project.getGenerator(generatorName);
      if (generatorElement.isNull() || generatorName.isEmpty() || generator.isNull()) {
         return failure(
             ProjectFileError::Validation,
             QStringLiteral("Node %1 uses unknown generator '%2'").arg(nodeId).arg(generatorName));
      }

      QMap<QString, int> sources;
      const QDomElement sourcesElement = node.firstChildElement(QStringLiteral("Sources"));
      for (QDomElement source = sourcesElement.firstChildElement(); !source.isNull();
           source = source.nextSiblingElement()) {
         if (source.tagName() != QStringLiteral("source")) {
            continue;
         }
         bool sourceOk = false;
         const QString serializedSlot = source.attribute(QStringLiteral("slot"));
         const QString slot = generator->resolveSourceSlot(serializedSlot);
         const int sourceId = source.attribute(QStringLiteral("source")).toInt(&sourceOk);
         if (slot.isNull() || !sourceOk || sourceId <= 0 || sourceId == nodeId ||
             sources.contains(slot)) {
            return failure(
                ProjectFileError::Validation,
                QStringLiteral("Node %1 has an invalid or duplicate source slot").arg(nodeId));
         }
         sources.insert(slot, sourceId);
      }
      sourcesByNode.insert(nodeId, sources);
   }

   QMap<int, int> indegree;
   QMap<int, QList<int>> receivers;
   for (auto node = sourcesByNode.cbegin(); node != sourcesByNode.cend(); ++node) {
      indegree.insert(node.key(), 0);
   }
   for (auto node = sourcesByNode.cbegin(); node != sourcesByNode.cend(); ++node) {
      QSet<int> uniqueSources;
      for (const int sourceId : node.value()) {
         if (!sourcesByNode.contains(sourceId)) {
            return failure(ProjectFileError::Validation,
                           QStringLiteral("Node %1 references missing source %2")
                               .arg(node.key())
                               .arg(sourceId));
         }
         uniqueSources.insert(sourceId);
      }
      indegree[node.key()] = uniqueSources.size();
      for (const int sourceId : uniqueSources) {
         receivers[sourceId].append(node.key());
      }
   }

   QList<int> ready;
   for (auto degree = indegree.cbegin(); degree != indegree.cend(); ++degree) {
      if (degree.value() == 0) {
         ready.append(degree.key());
      }
   }
   int visited = 0;
   while (!ready.isEmpty()) {
      const int sourceId = ready.takeFirst();
      ++visited;
      for (const int receiverId : receivers.value(sourceId)) {
         const int remaining = --indegree[receiverId];
         if (remaining == 0) {
            ready.append(receiverId);
         }
      }
   }
   if (visited != sourcesByNode.size()) {
      return failure(ProjectFileError::Validation,
                     QStringLiteral("The project graph contains a cycle"));
   }
   return {};
}

/// @brief Constructs a project from a parsed and validated XML document.
/// @param document Parsed project document.
/// @param project Project that receives the loaded graph.
/// @return The operation result, including construction failures.
ProjectFileResult ProjectFileService::loadDocument(const QDomDocument& document,
                                                   TextureProject& project) {
   ProjectFileResult validation = validate(document, project);
   if (!validation) {
      return validation;
   }

   QDomDocument normalized = document.cloneNode(true).toDocument();
   removeLegacyEmptySettings(normalized);
   try {
      TextureProject staged(false);
      for (const TextureGeneratorPtr& generator : project.getGenerators()) {
         staged.addGenerator(generator);
      }
      staged.loadFromXML(normalized);
      if (staged.findLoops()) {
         return failure(ProjectFileError::Construction,
                        QStringLiteral("The staged project graph contains a cycle"));
      }

      project.clear();
      project.loadFromXML(normalized);
      return {};
   } catch (const std::exception& error) {
      return failure(ProjectFileError::Construction, QString::fromUtf8(error.what()));
   } catch (...) {
      return failure(ProjectFileError::Construction,
                     QStringLiteral("Unknown error while constructing the project"));
   }
}

/// @brief Saves a project atomically as an XML file.
/// @param path Destination path for the project file.
/// @param project Project to serialize and mark as saved after a successful commit.
/// @param overwrite Whether an existing destination may be replaced.
/// @return The operation result, including an error message on failure.
ProjectFileResult ProjectFileService::save(const QString& path, TextureProject& project,
                                           const bool overwrite) {
   const QFileInfo destination(path);
   if (destination.exists() && !overwrite) {
      return failure(ProjectFileError::OutputExists,
                     QStringLiteral("The destination '%1' already exists").arg(path));
   }

   QSaveFile output(path);
   if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return failure(ProjectFileError::OutputOpen,
                     QStringLiteral("Could not open '%1': %2").arg(path, output.errorString()));
   }
   const QByteArray xml = project.saveAsXML().toString(3).toUtf8();
   if (output.write(xml) != xml.size()) {
      output.cancelWriting();
      return failure(ProjectFileError::OutputWrite,
                     QStringLiteral("Could not write '%1': %2").arg(path, output.errorString()));
   }
   if (!output.commit()) {
      return failure(ProjectFileError::OutputCommit,
                     QStringLiteral("Could not commit '%1': %2").arg(path, output.errorString()));
   }
   project.markSaved();
   return {};
}
