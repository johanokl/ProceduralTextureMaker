// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "projectfileservice.h"
#include "base/texturegenerator.h"
#include "textureproject.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QColor>
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

}  // namespace

QColor ProjectFileService::parseColor(const QString& text) {
   if (!text.startsWith('#') || (text.size() != 7 && text.size() != 9)) {
      return {};
   }
   // QColor expects alpha first; project JSON puts alpha after the RGB channels.
   return QColor(text.size() == 9 ? "#" + text.right(2) + text.mid(1, 6) : text);
}

QByteArray ProjectFileService::serializeNodes(const QJsonArray& nodes) {
   if (nodes.isEmpty()) {
      return "[]\n";
   }
   const QStringList fields{"id", "name", "position", "generator", "settings", "slots"};
   QByteArray json = "[\n";
   for (qsizetype index = 0; index < nodes.size(); ++index) {
      json += "  {\n";
      const auto node = nodes.at(index).toObject();
      bool first = true;
      for (const QString& field : fields) {
         if (!node.contains(field)) {
            continue;
         }
         if (!first) {
            json += ",\n";
         }
         first = false;
         // Let Qt escape and format each value, then remove the temporary object wrapper.
         QByteArray member = QJsonDocument(QJsonObject{{field, node.value(field)}}).toJson();
         member = member.mid(2, member.size() - 5);
         const auto lines = member.split('\n');
         for (qsizetype lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
            if (lineIndex != 0) {
               json += '\n';
            }
            const auto& line = lines.at(lineIndex);
            qsizetype spaces = 0;
            while (spaces < line.size() && line.at(spaces) == ' ') {
               ++spaces;
            }
            // Qt uses four spaces; project files use two spaces per level.
            json += QByteArray(2 + spaces / 2, ' ') + line.mid(spaces);
         }
      }
      json += index + 1 < nodes.size() ? "\n  },\n" : "\n  }\n";
   }
   json += "]\n";
   return json;
}

ProjectFileResult ProjectFileService::load(const QString& path, TextureProject& project) {
   QFile input(path);
   if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return failure(ProjectFileError::InputOpen,
                     QStringLiteral("Could not open '%1': %2").arg(path, input.errorString()));
   }

   QJsonParseError error;
   const auto document = QJsonDocument::fromJson(input.readAll(), &error);
   if (error.error != QJsonParseError::NoError) {
      return failure(
          ProjectFileError::JsonParse,
          QString("JSON parse error at byte %1: %2").arg(error.offset).arg(error.errorString()));
   }
   return loadDocument(document, project, path);
}

ProjectFileResult ProjectFileService::validate(const QJsonDocument& document,
                                               const TextureProject& project, const QString& path) {
   if (!document.isArray()) {
      return failure(ProjectFileError::Validation, "The project must be an array of nodes");
   }
   QSet<QString> missing;
   for (const auto& value : document.array()) {
      const QString name = value.toObject().value("generator").toString();
      if (!name.isEmpty() && !project.getGenerators().contains(name)) {
         missing.insert(name);
      }
   }
   if (!missing.isEmpty()) {
      QStringList names = missing.values();
      names.sort(Qt::CaseInsensitive);
      for (auto& name : names) {
         name = "'" + name + "'";
      }
      return failure(ProjectFileError::Validation,
                     names.size() == 1
                         ? QString("The project file '%1' could not be loaded because it "
                                   "references a texture generator that is not available: %2.")
                               .arg(path, names.first())
                         : QString("The project file '%1' could not be loaded because it "
                                   "references texture generators that are not available: %2.")
                               .arg(path, names.join(", ")));
   }
   QMap<QString, QSet<QString>> sources;
   for (const auto& value : document.array()) {
      const auto object = value.toObject();
      const QString id = object.value("id").toString();
      if (!value.isObject() || id.isEmpty() || id != id.trimmed() || sources.contains(id) ||
          !object.value("name").isString()) {
         return failure(ProjectFileError::Validation,
                        "Every node needs a unique nonempty string id and a string name");
      }
      const QString generatorName = object.value("generator").toString();
      const auto generator = project.getGenerators().value(generatorName);
      if (!generator) {
         return failure(ProjectFileError::Validation,
                        QString("The project file '%1' references an unavailable generator: '%2'")
                            .arg(path, generatorName));
      }
      if ((!object.value("slots").isUndefined() && !object.value("slots").isObject()) ||
          (!object.value("settings").isUndefined() && !object.value("settings").isObject())) {
         return failure(ProjectFileError::Validation, "Node settings and slots must be objects");
      }
      const auto position = object.value("position");
      if (!position.isUndefined() &&
          (!position.isObject() || !position.toObject().value("x").isDouble() ||
           !position.toObject().value("y").isDouble())) {
         return failure(ProjectFileError::Validation,
                        "Node position must contain numeric x and y coordinates");
      }
      QSet<QString> inputs;
      const auto connections = object.value("slots").toObject();
      for (auto source = connections.begin(); source != connections.end(); ++source) {
         if (!generator->getSourceSlots().contains(source.key()) || !source.value().isString() ||
             source.value().toString().isEmpty()) {
            return failure(ProjectFileError::Validation,
                           QString("Node '%1' has an invalid slot").arg(id));
         }
         inputs.insert(source.value().toString());
      }
      const auto settings = object.value("settings").toObject();
      for (auto setting = settings.begin(); setting != settings.end(); ++setting) {
         const auto* definition =
             findTextureGeneratorSetting(generator->getSettings(), setting.key());
         if (!definition) {
            continue;
         }
         const int type = definition->defaultvalue.metaType().id();
         const auto value = setting.value();
         if (((type == QMetaType::Int || type == QMetaType::Double) && !value.isDouble()) ||
             (type == QMetaType::Bool && !value.isBool()) ||
             ((type == QMetaType::QString || type == QMetaType::QStringList ||
               type == QMetaType::QColor) &&
              !value.isString()) ||
             (type == QMetaType::QColor && !parseColor(value.toString()).isValid()) ||
             (type == QMetaType::QStringList &&
              !definition->defaultvalue.toStringList().contains(value.toString()))) {
            return failure(ProjectFileError::Validation,
                           QString("Node '%1' has invalid setting '%2'").arg(id, setting.key()));
         }
      }
      sources.insert(id, inputs);
   }
   QMap<QString, int> indegree;
   QMap<QString, QStringList> receivers;
   for (auto node = sources.begin(); node != sources.end(); ++node) {
      indegree[node.key()] = node.value().size();
      for (const auto& source : node.value()) {
         if (!sources.contains(source)) {
            return failure(
                ProjectFileError::Validation,
                QString("Node '%1' references missing source '%2'").arg(node.key(), source));
         }
         receivers[source].append(node.key());
      }
   }
   QStringList ready;
   for (auto degree = indegree.begin(); degree != indegree.end(); ++degree) {
      if (degree.value() == 0) {
         ready.append(degree.key());
      }
   }
   int visited = 0;
   while (!ready.isEmpty()) {
      const auto source = ready.takeFirst();
      ++visited;
      for (const auto& receiver : receivers.value(source)) {
         if (--indegree[receiver] == 0) {
            ready.append(receiver);
         }
      }
   }
   if (visited != sources.size()) {
      return failure(ProjectFileError::Validation, "The project graph contains a cycle");
   }
   return {};
}

ProjectFileResult ProjectFileService::loadDocument(const QJsonDocument& document,
                                                   TextureProject& project, const QString& path) {
   auto validation = validate(document, project, path);
   if (!validation) {
      return validation;
   }
   try {
      TextureProject staged(false);
      for (const auto& generator : project.getGenerators()) {
         staged.addGenerator(generator);
      }
      staged.loadFromJSON(document);
      project.clear();
      project.loadFromJSON(document);
      return {};
   } catch (const std::exception& error) {
      return failure(ProjectFileError::Construction, QString::fromUtf8(error.what()));
   }
}

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
   const auto document = project.saveAsJSON();
   auto validation = validate(document, project, path);
   if (!validation) {
      return validation;
   }
   const QByteArray json = serializeNodes(document.array());
   if (output.write(json) != json.size()) {
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
