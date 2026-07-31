#include <QImage>
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
   /// @brief Verifies that help succeeds without starting the graphical interface.
   void showsHelp();

   /// @brief Verifies exporting a project whose output node is unambiguous.
   void exportsTrackedExampleByUniqueSink();

   /// @brief Verifies node listing and explicit selection for multiple output nodes.
   void listsNodesAndRequiresSelectionForMultipleSinks();

   /// @brief Verifies explicit loading of external JavaScript generators.
   void loadsJavaScriptGeneratorsExplicitly();

   /// @brief Verifies stable exit codes for usage and output failures.
   void returnsStableUsageAndOutputErrors();
};

void CliExportTest::showsHelp() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const ProcessResult result = runApplication({QStringLiteral("--help")}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY(result.standardOutput.contains("--no-gui"));
   QVERIFY(result.standardOutput.contains("--node"));
   QVERIFY(result.standardOutput.contains("--list-nodes"));

   const ProcessResult version = runApplication({QStringLiteral("--version")}, directory.path());
   QCOMPARE(version.exitCode, 0);
   QVERIFY(version.standardOutput.contains("ProceduralTextureMaker 0.1.0"));
}

void CliExportTest::exportsTrackedExampleByUniqueSink() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl");
   const QString output = directory.filePath(QStringLiteral("wall output 測試.png"));
   const ProcessResult result = runExporter(
       {QStringLiteral("--size"), QStringLiteral("32x24"), input, output}, directory.path());
   QCOMPARE(result.exitCode, 0);
   QVERIFY2(result.standardError.isEmpty(), result.standardError.constData());
   QVERIFY(result.standardOutput.contains("node 13"));
   const QImage image(output);
   QVERIFY(!image.isNull());
   QCOMPARE(image.size(), QSize(32, 24));
}

void CliExportTest::listsNodesAndRequiresSelectionForMultipleSinks() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/multi-sink.txl");
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
       QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/projects/javascript-solid.txl");
   const QString scripts = QStringLiteral(PTM_SOURCE_DIR "/tests/fixtures/javascript");
   const QString output = directory.filePath(QStringLiteral("javascript.png"));

   QCOMPARE(runExporter({input, output}, directory.path()).exitCode, 4);
   const ProcessResult result =
       runExporter({QStringLiteral("--js-dir"), scripts, QStringLiteral("--size"),
                    QStringLiteral("3x2"), input, output},
                   directory.path());
   QCOMPARE(result.exitCode, 0);
   const QImage image(output);
   QVERIFY(!image.isNull());
   QCOMPARE(image.size(), QSize(3, 2));
   QCOMPARE(image.pixelColor(0, 0), QColor(0x12, 0x34, 0x56, 0xff));
}

void CliExportTest::returnsStableUsageAndOutputErrors() {
   QTemporaryDir directory;
   QVERIFY(directory.isValid());
   const QString input = QStringLiteral(PTM_SOURCE_DIR "/examples/rose.txl");
   const QString output = directory.filePath(QStringLiteral("image.png"));
   QCOMPARE(
       runExporter({directory.filePath(QStringLiteral("missing.txl")), output}, directory.path())
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
