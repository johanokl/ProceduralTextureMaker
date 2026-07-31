#include "base/settingsmanager.h"
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

/// @brief Verifies application setting defaults, updates, signals, and persistence.
class SettingsManagerTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Isolates persistent settings in a temporary directory.
   void initTestCase();

   /// @brief Verifies defaults, changes, notifications, and persisted values.
   void defaultsChangesAndPersistence();

private:
   /// @brief Owns the isolated settings directory for the duration of the test.
   std::unique_ptr<QTemporaryDir> directory;
};

void SettingsManagerTest::initTestCase() {
   QCoreApplication::setOrganizationName(QStringLiteral("PTM tests"));
   QCoreApplication::setApplicationName(QStringLiteral("settings"));
   directory = std::make_unique<QTemporaryDir>();
   QVERIFY(directory->isValid());
   QSettings::setDefaultFormat(QSettings::IniFormat);
   QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory->path());
   QSettings().clear();
}

void SettingsManagerTest::defaultsChangesAndPersistence() {
   SettingsManager settings;
   QCOMPARE(settings.getPreviewSize(), QSize(800, 800));
   QCOMPARE(settings.getThumbnailSize(), QSize(300, 300));
   QCOMPARE(settings.getBackgroundColor(), QColor(QStringLiteral("#c8c8c8")));

   QSignalSpy updates(&settings, &SettingsManager::settingsUpdated);
   settings.setPreviewSize(settings.getPreviewSize());
   settings.setPreviewSize(QSize(640, 480));
   settings.setThumbnailSize(QSize(96, 64));
   settings.setJSTextureGeneratorsEnabled(true);
   QCOMPARE(updates.count(), 3);
   settings.setPreviewSize(QSize());
   settings.setBackgroundColor(QColor());
   QCOMPARE(updates.count(), 3);
   QVERIFY(settings.saveSettings());

   SettingsManager loaded;
   QCOMPARE(loaded.getPreviewSize(), QSize(640, 480));
   QCOMPARE(loaded.getThumbnailSize(), QSize(96, 64));
   QVERIFY(loaded.getJSTextureGeneratorsEnabled());

   QSettings persisted;
   persisted.setValue(QStringLiteral("previewsize"), QSize(-1, 0));
   persisted.setValue(QStringLiteral("backgroundcolor"), QStringLiteral("invalid"));
   persisted.sync();
   SettingsManager recovered;
   QCOMPARE(recovered.getPreviewSize(), QSize(800, 800));
   QCOMPARE(recovered.getBackgroundColor(), QColor(QStringLiteral("#c8c8c8")));
}

QTEST_APPLESS_MAIN(SettingsManagerTest)
#include "settingsmanager_test.moc"
