#include "base/texturenode.h"
#include "base/textureproject.h"
#include "base/texturerendermanager.h"
#include "support/testgenerators.h"
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace {

/// @brief Creates a small node snapshot for render-scheduling tests.
/// @param id Persisted node identifier.
/// @param generator Generator executed for the node.
/// @param value Grayscale value stored in the node settings.
/// @param sources Mapping from input slots to source node identifiers.
/// @return Fully initialized node snapshot.
TextureNodeSnapshot snapshot(const int id, const TextureGeneratorPtr& generator, const int value,
                             QMap<int, int> sources = {}) {
   TextureNodeSettings settings;
   settings.insert(QStringLiteral("value"), value);
   return TextureNodeSnapshot{id, 1, generator, settings, std::move(sources), {}};
}

/// @brief Collects render callbacks and provides bounded synchronization for tests.
struct CallbackState {
   /// @brief Protects callback result collections.
   std::mutex mutex;
   /// @brief Wakes tests when a render callback records a result.
   std::condition_variable condition;
   /// @brief Successful render results in publication order.
   std::vector<TextureRenderResult> results;
   /// @brief Render failures in publication order.
   std::vector<TextureRenderFailure> failures;

   /// @brief Waits until the requested number of successes or failures is recorded.
   /// @param count Minimum number of callbacks required.
   /// @param errors Selects failures instead of successful results when true.
   /// @return True when the count was reached before the timeout.
   bool waitFor(const std::size_t count, const bool errors = false) {
      std::unique_lock lock(mutex);
      return condition.wait_for(lock, std::chrono::seconds(5), [this, count, errors] {
         return (errors ? failures.size() : results.size()) >= count;
      });
   }
};

/// @brief Creates a render manager whose callbacks append to shared test state.
/// @param state Callback state that receives results and failures.
/// @param workerCount Number of worker threads to create.
/// @return Render manager configured for the test.
std::unique_ptr<TextureRenderManager> makeManager(CallbackState& state,
                                                  const std::size_t workerCount) {
   return std::make_unique<TextureRenderManager>(
       [&state](TextureRenderResult result) {
          std::lock_guard lock(state.mutex);
          state.results.push_back(std::move(result));
          state.condition.notify_all();
       },
       [&state](TextureRenderFailure failure) {
          std::lock_guard lock(state.mutex);
          state.failures.push_back(std::move(failure));
          state.condition.notify_all();
       },
       workerCount);
}

}  // namespace

/// @brief Verifies background graph scheduling, caching, cancellation, and publication.
class TextureRenderManagerTest : public QObject {
   Q_OBJECT

private slots:
   /// @brief Verifies independent graph branches execute concurrently.
   void rendersIndependentBranchesConcurrently();
   /// @brief Verifies cached nodes feed dependents without duplicate publication.
   void reusesCachedSnapshotsWithoutRepublishingThem();
   /// @brief Verifies results from superseded render requests are discarded.
   void discardsObsoleteActiveResults();
   /// @brief Verifies failures are reported without disabling later renders.
   void reportsFailureAndRemainsUsable();
   /// @brief Verifies the project publishes only its latest image on its owner thread.
   void projectPublishesLatestThumbnailOnOwnerThread();
};

void TextureRenderManagerTest::rendersIndependentBranchesConcurrently() {
   CallbackState state;
   auto* firstRaw = new RecordingGenerator(QStringLiteral("First branch"), 0, 1);
   auto* secondRaw = new RecordingGenerator(QStringLiteral("Second branch"), 0, 2);
   firstRaw->block();
   secondRaw->block();
   TextureGeneratorPtr first(firstRaw);
   TextureGeneratorPtr second(secondRaw);
   TextureGeneratorPtr join(new RecordingGenerator(QStringLiteral("Join"), 2, 3));
   const auto manager = makeManager(state, 2);
   TextureGraphSnapshot graph{
       QSize(2, 2),
       {snapshot(1, first, 1), snapshot(2, second, 2), snapshot(3, join, 3, {{0, 1}, {1, 2}})}};
   manager->render(std::move(graph));
   const bool firstStarted = firstRaw->waitUntilStarted();
   const bool secondStarted = secondRaw->waitUntilStarted();
   const int joinCallsWhileBlocked = static_cast<RecordingGenerator*>(join.data())->callCount();
   firstRaw->release();
   secondRaw->release();
   QVERIFY(firstStarted);
   QVERIFY(secondStarted);
   QCOMPARE(joinCallsWhileBlocked, 0);
   QVERIFY(state.waitFor(3));
   QCOMPARE(static_cast<RecordingGenerator*>(join.data())->callCount(), 1);
}

void TextureRenderManagerTest::reusesCachedSnapshotsWithoutRepublishingThem() {
   CallbackState state;
   auto* cachedRaw = new RecordingGenerator(QStringLiteral("Cached"), 0, 1);
   cachedRaw->fail();
   TextureGeneratorPtr cachedGenerator(cachedRaw);
   TextureGeneratorPtr receiver(new RecordingGenerator(QStringLiteral("Receiver"), 1, 2));
   TextureNodeSnapshot cachedSnapshot = snapshot(1, cachedGenerator, 1);
   cachedSnapshot.cachedImage = TextureImage::create(QSize(2, 2));
   const auto manager = makeManager(state, 1);
   manager->render(
       TextureGraphSnapshot{QSize(2, 2), {cachedSnapshot, snapshot(2, receiver, 2, {{0, 1}})}});
   QVERIFY(state.waitFor(1));
   QCOMPARE(cachedRaw->callCount(), 0);
   std::lock_guard lock(state.mutex);
   QCOMPARE(state.results.size(), std::size_t(1));
   QCOMPARE(state.results.front().nodeId, 2);
   QCOMPARE(state.failures.size(), std::size_t(0));
}

void TextureRenderManagerTest::discardsObsoleteActiveResults() {
   CallbackState state;
   auto* blockingRaw = new RecordingGenerator(QStringLiteral("Old"), 0, 10);
   blockingRaw->block();
   TextureGeneratorPtr blocking(blockingRaw);
   TextureGeneratorPtr current(new RecordingGenerator(QStringLiteral("New"), 0, 77));
   {
      const auto manager = makeManager(state, 2);
      manager->render(TextureGraphSnapshot{QSize(2, 2), {snapshot(1, blocking, 10)}});
      QVERIFY(blockingRaw->waitUntilStarted());
      manager->render(TextureGraphSnapshot{QSize(2, 2), {snapshot(1, current, 77)}});
      QVERIFY(state.waitFor(1));
      blockingRaw->release();
   }
   std::lock_guard lock(state.mutex);
   QCOMPARE(state.results.size(), std::size_t(1));
   QCOMPARE(state.results.front().image->data()[0].r, static_cast<unsigned char>(77));
   QCOMPARE(state.failures.size(), std::size_t(0));
}

void TextureRenderManagerTest::reportsFailureAndRemainsUsable() {
   CallbackState state;
   auto* failingRaw = new RecordingGenerator(QStringLiteral("Failure"));
   failingRaw->fail();
   TextureGeneratorPtr failing(failingRaw);
   TextureGeneratorPtr healthy(new RecordingGenerator(QStringLiteral("Healthy"), 0, 42));
   const auto manager = makeManager(state, 1);
   manager->render(TextureGraphSnapshot{QSize(2, 2), {snapshot(4, failing, 1)}});
   QVERIFY(state.waitFor(1, true));
   manager->render(TextureGraphSnapshot{QSize(2, 2), {snapshot(5, healthy, 42)}});
   QVERIFY(state.waitFor(1));
   std::lock_guard lock(state.mutex);
   QCOMPARE(state.failures.front().nodeId, 4);
   QVERIFY(state.failures.front().message.contains(QStringLiteral("planned")));
   QCOMPARE(state.results.front().nodeId, 5);
}

void TextureRenderManagerTest::projectPublishesLatestThumbnailOnOwnerThread() {
   TextureProject project(true);
   auto* generator = new RecordingGenerator(QStringLiteral("Async"), 0, 10);
   generator->block();
   project.addGenerator(TextureGeneratorPtr(generator));

   bool ownerThread = false;
   QObject::connect(&project, &TextureProject::imageAvailable, &project,
                    [&] { ownerThread = QThread::currentThread() == project.thread(); });
   QSignalSpy available(&project, &TextureProject::imageAvailable);
   const TextureNodePtr node = project.newNode(1, project.getGenerator(QStringLiteral("Async")));
   QVERIFY(generator->waitUntilStarted());
   TextureNodeSettings settings = node->getSettings();
   settings[QStringLiteral("value")] = 88;
   node->setSettings(settings);
   generator->release();
   if (available.isEmpty()) {
      QVERIFY(available.wait(5000));
   }
   QCOMPARE(node->cachedImage(project.getThumbnailSize())->data()[0].r,
            static_cast<unsigned char>(88));
   QVERIFY(ownerThread);
}

QTEST_GUILESS_MAIN(TextureRenderManagerTest)
#include "texturerendermanager_test.moc"
