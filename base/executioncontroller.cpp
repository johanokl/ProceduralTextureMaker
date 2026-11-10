// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "executioncontroller.h"
#include <QJSEngine>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>

/// @brief Uses one shared thread to interrupt JavaScript calls that reach their time limits.
class JsExecutionWatchdog final {
public:
   /// @brief Returns the application-wide watchdog, starting its thread on first use.
   static JsExecutionWatchdog& instance() {
      static JsExecutionWatchdog watchdog;
      return watchdog;
   }

   /// @brief Protects cancellation flags, engine registrations, and time limits.
   std::mutex mutex;
   /// @brief Wakes the watchdog when registrations change or shutdown begins.
   std::condition_variable changed;
   /// @brief Active calls whose engines are still available for interruption.
   std::set<ExecutionController::Registration*> registrations;

private:
   /// @brief Starts the thread that checks registered time limits.
   JsExecutionWatchdog() : worker([this] { run(); }) {}

   /// @brief Wakes the watchdog and waits for its thread to finish.
   ~JsExecutionWatchdog() {
      {
         std::lock_guard lock(mutex);
         stopping = true;
      }
      changed.notify_one();
      worker.join();
   }

   /// @brief Waits for the next time limit and interrupts calls that have exceeded it.
   void run() {
      std::unique_lock lock(mutex);
      while (!stopping) {
         auto next = std::chrono::steady_clock::time_point::max();
         const auto now = std::chrono::steady_clock::now();
         for (auto* registration : registrations) {
            if (registration->expired) {
               continue;
            }
            if (registration->deadline <= now) {
               registration->expired = true;
               registration->engine.setInterrupted(true);
            } else {
               next = std::min(next, registration->deadline);
            }
         }
         if (next == std::chrono::steady_clock::time_point::max()) {
            changed.wait(lock);
         } else {
            changed.wait_until(lock, next);
         }
      }
   }

   /// @brief Whether the watchdog thread should exit.
   bool stopping = false;
   /// @brief Thread that can interrupt an engine while its own thread is busy.
   std::thread worker;
};

void ExecutionController::cancel() {
   auto& watchdog = JsExecutionWatchdog::instance();
   std::lock_guard lock(watchdog.mutex);
   cancelled = true;
   for (auto* registration : watchdog.registrations) {
      if (&registration->controller == this) {
         registration->engine.setInterrupted(true);
      }
   }
}

bool ExecutionController::isCancelled() const {
   auto& watchdog = JsExecutionWatchdog::instance();
   std::lock_guard lock(watchdog.mutex);
   return cancelled;
}

ExecutionController::Registration::Registration(ExecutionController& controller, QJSEngine& engine,
                                                std::chrono::milliseconds timeout)
    : controller(controller),
      engine(engine),
      deadline(timeout.count() > 0 ? std::chrono::steady_clock::now() + timeout
                                   : std::chrono::steady_clock::time_point::max()) {
   auto& watchdog = JsExecutionWatchdog::instance();
   std::lock_guard lock(watchdog.mutex);
   if (controller.cancelled) {
      throw std::runtime_error("JavaScript execution cancelled");
   }
   engine.setInterrupted(false);
   watchdog.registrations.insert(this);
   watchdog.changed.notify_one();
}

ExecutionController::Registration::~Registration() {
   auto& watchdog = JsExecutionWatchdog::instance();
   std::lock_guard lock(watchdog.mutex);
   watchdog.registrations.erase(this);
   watchdog.changed.notify_one();
}

bool ExecutionController::Registration::timedOut() const {
   auto& watchdog = JsExecutionWatchdog::instance();
   std::lock_guard lock(watchdog.mutex);
   return expired;
}
