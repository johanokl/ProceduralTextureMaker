#include <QImage>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTest>

namespace {

/// @brief Captures the observable result of a command-line application invocation.
struct ProcessResult {
   /// @brief Process exit code, or -1 when the process did not finish normally.
   int exitCode = -1;
   /// @brief Bytes written to standard output.
   QByteArray standardOutput;
   /// @brief Bytes written to standard error.
   QByteArray standardError;
};

/// @brief Runs the application with the supplied arguments.
/// @param arguments Arguments passed after the executable name.
/// @param workingDirectory Directory used by the child process.
/// @return Captured exit status and output streams.
ProcessResult runApplication(const QStringList& arguments, const QString& workingDirectory) {
   QProcess process;
   process.setProgram(QStringLiteral(PTM_APPLICATION_EXECUTABLE));
   process.setArguments(arguments);
   process.setWorkingDirectory(workingDirectory);
   QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
   environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
   process.setProcessEnvironment(environment);
   process.start();
   if (!process.waitForStarted(5000) || !process.waitForFinished(30000)) {
      process.kill();
      process.waitForFinished();
      return {-1, process.readAllStandardOutput(), process.readAllStandardError()};
   }
   return {process.exitCode(), process.readAllStandardOutput(), process.readAllStandardError()};
}

/// @brief Runs the application in explicit command-line export mode.
/// @param arguments Exporter arguments following `--no-gui`.
/// @param workingDirectory Directory used by the child process.
/// @return Captured exit status and output streams.
ProcessResult runExporter(QStringList arguments, const QString& workingDirectory) {
   arguments.prepend(QStringLiteral("--no-gui"));
   return runApplication(arguments, workingDirectory);
}

}  // namespace

/// @brief Verifies the public command-line interface through child processes.
class CliExportTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies CLI selection and listing use editable string IDs.
   void selectsNamedNode();
   /// @brief Checks an explicit timeout cancels stuck JavaScript and preserves the destination.
   void timesOutOnlyWhenRequested();

   /// @brief Verifies that help succeeds without starting the graphical interface.
   void showsHelp();

   /// @brief Verifies exporting a project whose output node is unambiguous.
   void exportsTrackedExampleByUniqueSink();

   /// @brief Verifies node listing and explicit selection for multiple output nodes.
   void listsNodesAndRequiresSelectionForMultipleSinks();

   /// @brief Verifies explicit loading of external JavaScript generators.
   void loadsJavaScriptGeneratorsExplicitly();

   /// @brief Verifies bundled generator source and the starter template are directly accessible.
   void printsJavaScriptSources();

   /// @brief Verifies stable exit codes for usage and output failures.
   void returnsStableUsageAndOutputErrors();
};

void CliExportTest::selectsNamedNode() {
   QTemporaryDir directory;
   const QString path = directory.filePath("named.ptmg");
   QFile input(path);
   QVERIFY(input.open(QIODevice::WriteOnly));
   input.write(R"([
  {
    "id": "blue-background",
    "name": "Background",
    "generator": "Fill",
    "settings": {
      "color": "#0000ffff"
    }
  }
])");
   input.close();
   auto result = runExporter({"--list-nodes", path}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardOutput.contains("blue-background\tBackground\tFill"));
   const QString output = directory.filePath("blue.png");
   result =
       runExporter({"--node", "blue-background", "--size", "2x2", path, output}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QCOMPARE(QImage(output).pixelColor(0, 0), QColor(Qt::blue));
   QVERIFY(result.standardOutput.contains("Exported node blue-background"));
   result = runExporter({"--node", "1", path, output}, directory.path());
   QCOMPARE(result.exitCode, 5);
}

void CliExportTest::timesOutOnlyWhenRequested() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   QFile script(directory.filePath(QStringLiteral("stuck.js")));
   QVERIFY(script.open(QIODevice::WriteOnly));
   script.write(
       "const generator={apiVersion:1,name:'SolidJS',type:'generator',inputs:[],"
       "generate(){while(true){}}};");
   script.close();
   const QString output = directory.filePath(QStringLiteral("image.png"));
   QFile existing(output);
   QVERIFY(existing.open(QIODevice::WriteOnly));
   existing.write("old image");
   existing.close();
   const QString input =
       QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/javascript-solid.ptmg");
   const auto result =
       runExporter({QStringLiteral("--js-dir"), directory.path(), QStringLiteral("--timeout"),
                    QStringLiteral("1"), QStringLiteral("--force"), input, output},
                   directory.path());
   QCOMPARE(result.exitCode, 6);
   QVERIFY(result.standardError.contains("timed out"));
   QVERIFY(existing.open(QIODevice::ReadOnly));
   QCOMPARE(existing.readAll(), QByteArray("old image"));
   QCOMPARE(runExporter({QStringLiteral("--js-dir"), directory.path(), QStringLiteral("--timeout"),
                         QStringLiteral("0"), input, output},
                        directory.path())
                .exitCode,
            2);
}

void CliExportTest::showsHelp() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const ProcessResult result = runApplication({QStringLiteral("--help")}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardOutput.contains("--no-gui"));
   QVERIFY(result.standardOutput.contains("--node"));
   QVERIFY(result.standardOutput.contains("--list-nodes"));
   QVERIFY(result.standardOutput.contains("--print-js-template"));

   const ProcessResult version = runApplication({QStringLiteral("--version")}, directory.path());
   QCOMPARE(version.exitCode, 0);
   QVERIFY(version.standardOutput.contains("ProceduralTextureMaker 0.1.0"));
}

void CliExportTest::exportsTrackedExampleByUniqueSink() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/examples/wall.ptmg");
   const QString output = directory.filePath(QStringLiteral("wall output 測試.png"));
   const ProcessResult result = runExporter(
       {QStringLiteral("--size"), QStringLiteral("32x24"), input, output}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardError.contains("100%"));
   QVERIFY(!result.standardError.contains("Error:"));
   QVERIFY(result.standardOutput.contains("node 13"));
   const QImage image(output);
   QVERIFY(!image.isNull());
   QCOMPARE(image.size(), QSize(32, 24));
}

void CliExportTest::listsNodesAndRequiresSelectionForMultipleSinks() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/multi-sink.ptmg");
   ProcessResult result = runExporter({QStringLiteral("--list-nodes"), input}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardOutput.contains("1\tFirst output\tFill\tsink"));
   QVERIFY(result.standardOutput.contains("2\tSecond output\tFill\tsink"));

   const QString output = directory.filePath(QStringLiteral("selected.png"));
   result = runExporter({input, output}, directory.path());
   QCOMPARE(result.exitCode, 5);
   QVERIFY(result.standardError.contains("Candidates"));
   result = runExporter({QStringLiteral("--node"), QStringLiteral("2"), QStringLiteral("--size"),
                         QStringLiteral("4x3"), input, output},
                        directory.path());
   QCOMPARE(result.exitCode, 0);
   QCOMPARE(QImage(output).pixelColor(0, 0), QColor(Qt::blue));
}

void CliExportTest::loadsJavaScriptGeneratorsExplicitly() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input =
       QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/javascript-solid.ptmg");
   const QString scripts = QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/javascript");
   const QString output = directory.filePath(QStringLiteral("javascript.png"));

   QCOMPARE(runExporter({input, output}, directory.path()).exitCode, 4);
   const ProcessResult result =
       runExporter({QStringLiteral("--js-dir"), scripts, QStringLiteral("--size"),
                    QStringLiteral("3x2"), input, output},
                   directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardError.contains("[..............................] 0%"));
   QVERIFY(result.standardError.contains("[##############################] 100%"));
   QVERIFY(result.standardError.contains("Ctrl+C to cancel"));
   const QImage image(output);
   QVERIFY(!image.isNull());
   QCOMPARE(image.size(), QSize(3, 2));
   QCOMPARE(image.pixelColor(0, 0), QColor(0x12, 0x34, 0x56, 0xff));
}

void CliExportTest::printsJavaScriptSources() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const ProcessResult templateResult =
       runApplication({QStringLiteral("--print-js-template")}, directory.path());
   QCOMPARE(templateResult.exitCode, 0);
   QVERIFY(templateResult.standardOutput.contains("apiVersion: 1"));
   QVERIFY(templateResult.standardOutput.contains("My Generator"));
   QVERIFY(templateResult.standardOutput.contains("settings: ["));
   QVERIFY(templateResult.standardOutput.contains("id: \"color\""));

   const ProcessResult maskResult = runApplication(
       {QStringLiteral("--print-js-generator"), QStringLiteral("Mask")}, directory.path());
   QCOMPARE(maskResult.exitCode, 0);
   QVERIFY(maskResult.standardOutput.contains("name: \"Mask\""));
   QVERIFY(maskResult.standardOutput.contains("type: \"combiner\""));
}

void CliExportTest::returnsStableUsageAndOutputErrors() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/examples/rose.ptmg");
   const QString output = directory.filePath(QStringLiteral("image.png"));
   QCOMPARE(
       runExporter({directory.filePath(QStringLiteral("missing.ptmg")), output}, directory.path())
           .exitCode,
       3);
   QCOMPARE(runExporter({QStringLiteral("--size"), QStringLiteral("bad"), input, output},
                        directory.path())
                .exitCode,
            2);
   QCOMPARE(runExporter({QStringLiteral("--size"), QStringLiteral("4x4"), input, output},
                        directory.path())
                .exitCode,
            0);
   QCOMPARE(runExporter({QStringLiteral("--size"), QStringLiteral("4x4"), input, output},
                        directory.path())
                .exitCode,
            7);
}

QTEST_APPLESS_MAIN(CliExportTest)
#include "cli_export_test.moc"
