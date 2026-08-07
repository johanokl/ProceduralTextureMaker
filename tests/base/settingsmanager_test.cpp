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
   QCOMPARE(settings.getBackgroundBrushColor(), QColor(QStringLiteral("#000000")));
   QCOMPARE(settings.getBackgroundBrush(), static_cast<int>(Qt::NoBrush));
   QCOMPARE(settings.getNodeBackgroundColor(), QColor(QStringLiteral("#ffffff")));
   QCOMPARE(settings.getNodeBackgroundBrushColor(), QColor(QStringLiteral("#dedede")));
   QCOMPARE(settings.getNodeBackgroundBrush(), static_cast<int>(Qt::CrossPattern));
   QCOMPARE(settings.getConnectionLabelSize(), 12);
   QVERIFY(settings.getDisplaySourceNames());
   QVERIFY(!settings.getDisplayReceiverNames());

   QSignalSpy updates(&settings, &SettingsManager::settingsUpdated);
   settings.setPreviewSize(settings.getPreviewSize());
   settings.setPreviewSize(QSize(640, 480));
   settings.setThumbnailSize(QSize(96, 64));
   settings.setJSTextureGeneratorsEnabled(true);
   settings.setConnectionLabelSize(18);
   settings.setDisplaySourceNames(false);
   settings.setDisplayReceiverNames(true);
   settings.setBackgroundColor(QColor(QStringLiteral("#abcdef")));
   settings.setBackgroundBrushColor(QColor(QStringLiteral("#123456")));
   settings.setBackgroundBrush(static_cast<int>(Qt::CrossPattern));
   settings.setNodeBackgroundColor(QColor(QStringLiteral("#fedcba")));
   settings.setNodeBackgroundBrushColor(QColor(QStringLiteral("#654321")));
   settings.setNodeBackgroundBrush(static_cast<int>(Qt::DiagCrossPattern));
   QCOMPARE(updates.count(), 12);
   settings.setPreviewSize(QSize());
   settings.setBackgroundColor(QColor());
   settings.setConnectionLabelSize(40);
   settings.setBackgroundBrushColor(QColor());
   settings.setNodeBackgroundColor(QColor());
   settings.setNodeBackgroundBrushColor(QColor());
   settings.setNodeBackgroundBrush(100);
   settings.setBackgroundBrush(static_cast<int>(Qt::LinearGradientPattern));
   settings.setNodeBackgroundBrush(static_cast<int>(Qt::TexturePattern));
   QCOMPARE(updates.count(), 12);
   QVERIFY(settings.saveSettings());

   SettingsManager loaded;
   QCOMPARE(loaded.getPreviewSize(), QSize(640, 480));
   QCOMPARE(loaded.getThumbnailSize(), QSize(96, 64));
   QVERIFY(loaded.getJSTextureGeneratorsEnabled());
   QCOMPARE(loaded.getConnectionLabelSize(), 18);
   QVERIFY(!loaded.getDisplaySourceNames());
   QVERIFY(loaded.getDisplayReceiverNames());
   QCOMPARE(loaded.getBackgroundColor(), QColor(QStringLiteral("#abcdef")));
   QCOMPARE(loaded.getBackgroundBrushColor(), QColor(QStringLiteral("#123456")));
   QCOMPARE(loaded.getBackgroundBrush(), static_cast<int>(Qt::CrossPattern));
   QCOMPARE(loaded.getNodeBackgroundColor(), QColor(QStringLiteral("#fedcba")));
   QCOMPARE(loaded.getNodeBackgroundBrushColor(), QColor(QStringLiteral("#654321")));
   QCOMPARE(loaded.getNodeBackgroundBrush(), static_cast<int>(Qt::DiagCrossPattern));

   QSettings persisted;
   persisted.setValue(QStringLiteral("previewsize"), QSize(-1, 0));
   persisted.setValue(QStringLiteral("backgroundcolor"), QStringLiteral("invalid"));
   persisted.setValue(QStringLiteral("backgroundbrushcolor"), QStringLiteral("invalid"));
   persisted.setValue(QStringLiteral("backgroundbrush"),
                      static_cast<int>(Qt::LinearGradientPattern));
   persisted.setValue(QStringLiteral("nodebackgroundcolor"), QStringLiteral("invalid"));
   persisted.setValue(QStringLiteral("nodebackgroundbrushcolor"), QStringLiteral("invalid"));
   persisted.setValue(QStringLiteral("nodebackgroundbrush"), static_cast<int>(Qt::TexturePattern));
   persisted.setValue(QStringLiteral("connectionlabelsize"), 40);
   persisted.sync();
   SettingsManager recovered;
   QCOMPARE(recovered.getPreviewSize(), QSize(800, 800));
   QCOMPARE(recovered.getBackgroundColor(), QColor(QStringLiteral("#c8c8c8")));
   QCOMPARE(recovered.getBackgroundBrushColor(), QColor(QStringLiteral("#000000")));
   QCOMPARE(recovered.getBackgroundBrush(), static_cast<int>(Qt::NoBrush));
   QCOMPARE(recovered.getNodeBackgroundColor(), QColor(QStringLiteral("#ffffff")));
   QCOMPARE(recovered.getNodeBackgroundBrushColor(), QColor(QStringLiteral("#dedede")));
   QCOMPARE(recovered.getNodeBackgroundBrush(), static_cast<int>(Qt::CrossPattern));
   QCOMPARE(recovered.getConnectionLabelSize(), 12);
}

QTEST_APPLESS_MAIN(SettingsManagerTest)
#include "settingsmanager_test.moc"
