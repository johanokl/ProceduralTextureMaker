#include "base/texturenode.h"
#include "base/jstexgen.h"
#include "base/textureproject.h"
#include "base/texturerendermanager.h"
#include "support/testgenerators.h"
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QTest>
#include <QThread>
#include <algorithm>
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
                             QMap<QString, int> sources = {}) {
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
   /// @brief Verifies a queued failure cannot overwrite the state of a newer successful render.
   void ignoresQueuedFailureFromOldRender();
   /// @brief Verifies cancellation affects one manager and a new render replaces stuck work.
   void cancelsOnlyItsOwnJavaScriptAndReplacesStuckRender();
   /// @brief Verifies native nodes can finish after Stop and their images are reused on Restart.
   void stopsNativeNodesAndRestartsUsingCompletedImages();
   /// @brief Verifies slow-node tracking and rejection of prompts for stopped renders.
   void tracksSlowNodesAndIgnoresObsoletePrompts();
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
   /// @brief Verifies background rendering routes inputs by names rather than map order.
   void routesNamedInputs();
   /// @brief Verifies timing samples are thread-safe and capped at ten recent calls.
   void recordsRollingTimingAcrossThreads();
};

void TextureRenderManagerTest::ignoresQueuedFailureFromOldRender() {
   TextureProject project;
   const TextureGeneratorPtr generator(new JsTexGen(QStringLiteral(
       "const generator={apiVersion:1,name:'QueuedFailure',type:'generator',inputs:[],"
       "settings:[{id:'fail',type:'boolean',default:true}],"
       "generate(size,settings,output){if(settings.fail)throw Error('obsolete failure');"
       "output.data.fill(255);}};")));
   QSignalSpy failures(&project, &TextureProject::renderFailed);
   const auto node = project.newNode(1, generator);
   // Let the worker queue its failure without delivering events on the project thread.
   QElapsedTimer timer;
   timer.start();
   while (project.canStopRendering() && timer.elapsed() < 2000) {
      QThread::msleep(1);
   }
   QVERIFY(!project.canStopRendering());
   node->setSettings({{QStringLiteral("fail"), false}});
   QTRY_VERIFY(!node->cachedImage(project.getThumbnailSize()).isNull());
   QCOMPARE(project.thumbnailRenderStatus(1).state, TextureRenderState::Ready);
   QCOMPARE(failures.count(), 0);
}

void TextureRenderManagerTest::cancelsOnlyItsOwnJavaScriptAndReplacesStuckRender() {
   using namespace std::chrono_literals;
   const TextureGeneratorPtr infinite(new JsTexGen(
       QStringLiteral("const generator={apiVersion:1,name:'Infinite',type:'generator',inputs:[],"
                      "generate(){while(true){}}};")));
   CallbackState firstState;
   CallbackState secondState;
   auto first = makeManager(firstState, 1);
   auto second = makeManager(secondState, 1);
   first->render({QSize(1, 1), {snapshot(1, infinite, 0)}});
   second->render({QSize(1, 1), {snapshot(1, infinite, 0)}});
   QTRY_VERIFY(first->slowNode(0ms).has_value());
   QTRY_VERIFY(second->slowNode(0ms).has_value());
   const auto secondNode = second->slowNode(0ms);
   first->cancel();
   first.reset();
   if (secondNode) {
      QVERIFY(second->isRunning(*secondNode));
   } else {
      QFAIL("Expected the second render to remain active");
   }
   TextureGeneratorPtr fast(new RecordingGenerator(QStringLiteral("Fast"), 0, 7));
   second->render({QSize(1, 1), {snapshot(2, fast, 7)}});
   QVERIFY(secondState.waitFor(1));
   second.reset();
   QVERIFY(firstState.failures.empty());
   QVERIFY(secondState.failures.empty());
   QCOMPARE(secondState.results.size(), std::size_t(1));
   QCOMPARE(secondState.results.front().nodeId, 2);
}

void TextureRenderManagerTest::stopsNativeNodesAndRestartsUsingCompletedImages() {
   TextureProject project;
   auto* sourceRaw = new RecordingGenerator(QStringLiteral("Source"), 0, 17);
   auto* receiverRaw = new RecordingGenerator(QStringLiteral("Receiver"), 1, 18);
   TextureGeneratorPtr source(sourceRaw);
   TextureGeneratorPtr receiver(receiverRaw);
   project.stopRendering();
   auto first = project.newNode(1, source);
   auto second = project.newNode(2, receiver);
   second->setSourceSlot(QStringLiteral("Image"), first->getId());
   sourceRaw->block();
   project.restartRendering();
   const bool started = sourceRaw->waitUntilStarted();
   project.stopRendering();
   sourceRaw->release();
   QVERIFY(started);
   QSignalSpy available(&project, &TextureProject::imageAvailable);
   QTRY_COMPARE(available.count(), 1);
   QCOMPARE(receiverRaw->callCount(), 0);
   QCOMPARE(project.thumbnailRenderStatus(1).state, TextureRenderState::Ready);
   QCOMPARE(project.thumbnailRenderStatus(2).state, TextureRenderState::Stopped);
   QVERIFY(project.isRenderingStopped());
   // Edits while stopped must not resume rendering.
   project.notifyImageUpdated(second->getId());
   QCOMPARE(receiverRaw->callCount(), 0);
   project.restartRendering();
   QTRY_COMPARE(available.count(), 2);
   QCOMPARE(sourceRaw->callCount(), 1);
   QCOMPARE(receiverRaw->callCount(), 1);
}

void TextureRenderManagerTest::tracksSlowNodesAndIgnoresObsoletePrompts() {
   using namespace std::chrono_literals;
   CallbackState state;
   auto* generatorRaw = new RecordingGenerator(QStringLiteral("Slow"), 0, 1);
   TextureGeneratorPtr generator(generatorRaw);
   generatorRaw->block();
   auto manager = makeManager(state, 1);
   manager->render({QSize(1, 1), {snapshot(1, generator, 1)}});
   const bool started = generatorRaw->waitUntilStarted();
   auto slow = manager->slowNode(0ms);
   if (slow) {
      manager->continueRendering(*slow);
   }
   const bool deferred = !manager->slowNode().has_value();
   manager->stop();
   const bool stopped = slow && !manager->isRunning(*slow);
   generatorRaw->release();
   manager.reset();
   QVERIFY(started);
   QVERIFY(slow.has_value());
   if (slow) {
      QCOMPARE(slow->generatorName, QStringLiteral("Slow"));
   }
   QVERIFY(deferred);
   QVERIFY(stopped);
}

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
       {snapshot(1, first, 1), snapshot(2, second, 2),
        snapshot(3, join, 3, {{QStringLiteral("Input 1"), 1}, {QStringLiteral("Input 2"), 2}})}};
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
   manager->render(TextureGraphSnapshot{
       QSize(2, 2), {cachedSnapshot, snapshot(2, receiver, 2, {{QStringLiteral("Image"), 1}})}});
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

void TextureRenderManagerTest::routesNamedInputs() {
   CallbackState state;
   TextureGeneratorPtr first(new RecordingGenerator(QStringLiteral("First"), 0, 17));
   TextureGeneratorPtr second(new RecordingGenerator(QStringLiteral("Second"), 0, 83));
   TextureGeneratorPtr named(new NamedInputGenerator);
   const auto manager = makeManager(state, 1);
   manager->render(TextureGraphSnapshot{
       QSize(2, 2),
       {snapshot(1, first, 17), snapshot(2, second, 83),
        snapshot(3, named, 0, {{QStringLiteral("Zulu"), 1}, {QStringLiteral("Alpha"), 2}})}});
   QVERIFY(state.waitFor(3));
   std::lock_guard lock(state.mutex);
   const auto result =
       std::find_if(state.results.cbegin(), state.results.cend(),
                    [](const TextureRenderResult& candidate) { return candidate.nodeId == 3; });
   QVERIFY(result != state.results.cend());
   QCOMPARE(result->image->data()[0].r, static_cast<unsigned char>(17));
}

void TextureRenderManagerTest::recordsRollingTimingAcrossThreads() {
   CallbackState state;
   TextureGeneratorPtr generator(new RecordingGenerator(QStringLiteral("Timed"), 0, 17));
   const auto manager = makeManager(state, 4);
   std::vector<TextureNodeSnapshot> nodes;
   for (int id = 1; id <= 12; ++id) {
      nodes.push_back(snapshot(id, generator, 17));
   }

   manager->render(TextureGraphSnapshot{QSize(2, 2), nodes});
   QVERIFY(state.waitFor(12));
   const TextureGenerator::GenerationTiming timing = generator->getGenerationTiming();
   QCOMPARE(timing.runCount, 10);
   QVERIFY(timing.averageMilliseconds >= 0.0);
}

QTEST_GUILESS_MAIN(TextureRenderManagerTest)
#include "texturerendermanager_test.moc"
