#include "base/textureproject.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

/// @brief Performs a minimal offscreen smoke test of the graphical application.
class UiSmokeTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Isolates application settings and initializes the test environment.
   void initTestCase();
   /// @brief Verifies that a tracked project loads into a scene-backed window.
   void loadsProjectIntoSceneBackedWindow();

private:
   /// @brief Owns the isolated settings directory for the duration of the test.
   std::unique_ptr<QTemporaryDir> settingsDirectory;
};

void UiSmokeTest::initTestCase() {
   QStandardPaths::setTestModeEnabled(true);
   QCoreApplication::setOrganizationName(QStringLiteral("PTM tests"));
   QCoreApplication::setApplicationName(QStringLiteral("UI smoke"));
   settingsDirectory = std::make_unique<QTemporaryDir>();
   QVERIFY(settingsDirectory->isValid());
   QSettings::setDefaultFormat(QSettings::IniFormat);
   QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory->path());
   QSettings settings;
   settings.setValue(QStringLiteral("showhelpstartup"), false);
}

void UiSmokeTest::loadsProjectIntoSceneBackedWindow() {
   auto* application = qobject_cast<TexGenApplication*>(QCoreApplication::instance());
   QVERIFY(application != nullptr);
   MainWindow window(application);
   window.openFile(QStringLiteral(PTM_SOURCE_DIR "/examples/wall.txl"));
   QCOMPARE(window.getTextureProject()->getNumNodes(), 11);
   QCOMPARE(window.getTextureProject()->getSinkNodeIds(), QList<int>({13}));
}

/// @brief Runs the UI smoke test with the production application subclass.
/// @param argc Number of command-line arguments.
/// @param argv Command-line argument values.
/// @return Qt Test process exit code.
int main(int argc, char** argv) {
   TexGenApplication application(argc, argv);
   UiSmokeTest test;
   return QTest::qExec(&test, argc, argv);
}

#include "ui_smoke_test.moc"
