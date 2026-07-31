// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "commandline.h"
#include "base/projectfileservice.h"
#include "base/textureexporter.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include "generators/builtinregistry.h"
#include "generators/javascript.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <optional>

namespace {

enum class ExitCode : int {
   Success = 0,
   Usage = 2,
   Input = 3,
   Project = 4,
   Node = 5,
   Render = 6,
   Output = 7
};

int exitCode(const ExitCode code) { return static_cast<int>(code); }

int reportError(const ExitCode code, const QString& message) {
   QTextStream(stderr) << "Error: " << message << Qt::endl;
   return exitCode(code);
}

std::optional<QSize> parseSize(const QString& value) {
   static const QRegularExpression expression(QStringLiteral("^(\\d+)[xX](\\d+)$"));
   const QRegularExpressionMatch match = expression.match(value);
   if (!match.hasMatch()) {
      return std::nullopt;
   }
   bool widthOk = false;
   bool heightOk = false;
   const int width = match.captured(1).toInt(&widthOk);
   const int height = match.captured(2).toInt(&heightOk);
   if (!widthOk || !heightOk || width <= 0 || height <= 0 ||
       static_cast<qint64>(width) * height > TextureExporter::MaximumPixelCount) {
      return std::nullopt;
   }
   return QSize(width, height);
}

ExitCode projectLoadExitCode(const ProjectFileError error) {
   if (error == ProjectFileError::InputOpen || error == ProjectFileError::XmlParse) {
      return ExitCode::Input;
   }
   return ExitCode::Project;
}

bool isCommandLineSwitch(const QByteArray& argument) {
   return argument == "--no-gui" || argument == "-h" || argument == "--help" ||
          argument == "--help-all" || argument == "-v" || argument == "--version";
}

/// @brief Adds the supported export options and positional arguments to a parser.
/// @param parser Parser to configure.
void configureParser(QCommandLineParser& parser) {
   parser.setApplicationDescription(
       QStringLiteral("Render a ProceduralTextureMaker project without opening the GUI."));
   parser.addHelpOption();
   parser.addVersionOption();
   parser.addOption({QStringLiteral("no-gui"),
                     QStringLiteral("Run without opening a window and enable export options.")});
   parser.addOption({{QStringLiteral("n"), QStringLiteral("node")},
                     QStringLiteral("Export this node ID."),
                     QStringLiteral("id")});
   parser.addOption({{QStringLiteral("s"), QStringLiteral("size")},
                     QStringLiteral("Output size as WIDTHxHEIGHT (default 800x800)."),
                     QStringLiteral("size"),
                     QStringLiteral("800x800")});
   parser.addOption({QStringLiteral("js-dir"),
                     QStringLiteral("Load JavaScript generators recursively from this directory."),
                     QStringLiteral("path")});
   parser.addOption({{QStringLiteral("f"), QStringLiteral("force")},
                     QStringLiteral("Replace an existing output file.")});
   parser.addOption(
       {QStringLiteral("list-nodes"), QStringLiteral("List project nodes without rendering.")});
   parser.addPositionalArgument(QStringLiteral("input.txl"), QStringLiteral("Input project file."));
   parser.addPositionalArgument(QStringLiteral("output.png"),
                                QStringLiteral("Output PNG (not needed with --list-nodes)."),
                                QStringLiteral("[output.png]"));
}

/// @brief Registers generators and loads the requested project file.
/// @param parser Parser containing JavaScript generator directory options.
/// @param inputPath Path of the project to load.
/// @param project Project that receives generators and loaded nodes.
/// @return Process exit code for the operation.
int loadProject(const QCommandLineParser& parser, const QString& inputPath,
                TextureProject& project) {
   registerBuiltInGenerators(project);
   for (const QString& directory : parser.values(QStringLiteral("js-dir"))) {
      const QString error = loadJavaScriptGenerators(project, directory);
      if (!error.isEmpty()) {
         return reportError(ExitCode::Project, error);
      }
   }

   const ProjectFileResult result = ProjectFileService::load(inputPath, project);
   if (!result) {
      return reportError(projectLoadExitCode(result.error), result.message);
   }
   return exitCode(ExitCode::Success);
}

/// @brief Prints all project nodes and marks nodes without receivers as sinks.
/// @param project Project whose nodes are listed.
/// @return Successful process exit code.
int listProjectNodes(const TextureProject& project) {
   const QList<int> sinkIds = project.getSinkNodeIds();
   QTextStream output(stdout);
   for (const int id : project.getNodeIds()) {
      const TextureNodePtr node = project.getNode(id);
      output << id << '\t' << node->getName() << '\t' << node->getGeneratorName();
      if (sinkIds.contains(id)) {
         output << "\tsink";
      }
      output << Qt::endl;
   }
   return exitCode(ExitCode::Success);
}

/// @brief Selects an explicit node or the project's unique sink node.
/// @param parser Parser containing the optional node selection.
/// @param project Loaded project whose node is selected.
/// @param nodeId Destination for the selected node identifier.
/// @return Successful process exit code, or a node-selection error code.
int selectNode(const QCommandLineParser& parser, const TextureProject& project, int& nodeId) {
   if (parser.isSet(QStringLiteral("node"))) {
      bool nodeOk = false;
      const QString value = parser.value(QStringLiteral("node"));
      nodeId = value.toInt(&nodeOk);
      if (!nodeOk || nodeId <= 0 || project.getNode(nodeId).isNull()) {
         return reportError(ExitCode::Node,
                            QStringLiteral("No project node matches --node '%1'").arg(value));
      }
      return exitCode(ExitCode::Success);
   }

   const QList<int> sinkIds = project.getSinkNodeIds();
   if (sinkIds.size() == 1) {
      nodeId = sinkIds.first();
      return exitCode(ExitCode::Success);
   }

   QStringList candidates;
   for (const int id : sinkIds) {
      const TextureNodePtr node = project.getNode(id);
      candidates.append(QStringLiteral("%1 (%2)").arg(id).arg(node->getName()));
   }
   return reportError(ExitCode::Node,
                      QStringLiteral("The project has %1 sink nodes; use --node. Candidates: %2")
                          .arg(sinkIds.size())
                          .arg(candidates.join(QStringLiteral(", "))));
}

/// @brief Exports the selected node according to parsed output options.
/// @param parser Parser containing overwrite behavior.
/// @param project Project containing the selected node.
/// @param nodeId Identifier of the node to export.
/// @param size Requested output dimensions.
/// @param outputPath Destination PNG path.
/// @return Process exit code for the export operation.
int exportNode(const QCommandLineParser& parser, TextureProject& project, const int nodeId,
               const QSize size, const QString& outputPath) {
   if (QFileInfo(outputPath).suffix().compare(QStringLiteral("png"), Qt::CaseInsensitive) != 0) {
      return reportError(ExitCode::Output, QStringLiteral("Only PNG output is supported"));
   }

   const TextureExportResult result = TextureExporter::exportPng(
       project, nodeId, size, outputPath, parser.isSet(QStringLiteral("force")));
   if (!result) {
      const ExitCode code = result.error == TextureExportError::InvalidNode ? ExitCode::Node
                            : result.error == TextureExportError::Render    ? ExitCode::Render
                                                                            : ExitCode::Output;
      return reportError(code, result.message);
   }

   QTextStream(stdout) << QStringLiteral("Exported node %1 at %2x%3 to %4")
                              .arg(nodeId)
                              .arg(size.width())
                              .arg(size.height())
                              .arg(outputPath)
                       << Qt::endl;
   return exitCode(ExitCode::Success);
}

}  // namespace

bool useCommandLineMode(const int argc, char* argv[]) {
   for (int index = 1; index < argc; ++index) {
      if (isCommandLineSwitch(QByteArray(argv[index]))) {
         return true;
      }
   }
   return false;
}

int runCommandLine(QCoreApplication& application) {
   QCommandLineParser parser;
   configureParser(parser);
   parser.process(application);

   const QStringList positional = parser.positionalArguments();
   const bool listNodes = parser.isSet(QStringLiteral("list-nodes"));
   if (positional.isEmpty() || positional.size() > 2 || (!listNodes && positional.size() != 2)) {
      QTextStream(stderr) << parser.helpText();
      return reportError(ExitCode::Usage, QStringLiteral("Expected an input and output path"));
   }

   const std::optional<QSize> exportSize = parseSize(parser.value(QStringLiteral("size")));
   if (!exportSize) {
      return reportError(
          ExitCode::Usage,
          QStringLiteral("Invalid --size; use positive WIDTHxHEIGHT within %1 pixels")
              .arg(TextureExporter::MaximumPixelCount));
   }

   TextureProject project(false);
   const int loadResult = loadProject(parser, positional.at(0), project);
   if (loadResult != exitCode(ExitCode::Success)) {
      return loadResult;
   }

   if (listNodes) {
      return listProjectNodes(project);
   }

   int nodeId = 0;
   const int selectionResult = selectNode(parser, project, nodeId);
   if (selectionResult != exitCode(ExitCode::Success)) {
      return selectionResult;
   }

   return exportNode(parser, project, nodeId, *exportSize, positional.at(1));
}
