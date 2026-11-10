// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef EXECUTIONCONTROLLER_H
#define EXECUTIONCONTROLLER_H

#include <chrono>

class QJSEngine;
class JsExecutionWatchdog;

/// @brief Shares cancellation state for one graph render or generator scan.
class ExecutionController final {
public:
   /// @brief Creates an execution state that has not been cancelled.
   ExecutionController() = default;

   /// @brief Disables copying because each controller belongs to one render or scan.
   ExecutionController(const ExecutionController&) = delete;

   /// @brief Disables copy assignment to keep cancellation state with its owner.
   ExecutionController& operator=(const ExecutionController&) = delete;

   /// @brief Stops this owner's work and interrupts any registered JavaScript engines.
   /// @details Native generators finish their current node; the scheduler checks cancellation
   /// before starting another node.
   void cancel();

   /// @brief Returns whether this owner has been cancelled.
   bool isCancelled() const;

   /// @brief Makes an engine available for interruption while JavaScript runs.
   class Registration final {
   public:
      /// @brief Checks cancellation and registers the engine under the same lock.
      /// @param controller Cancellation state for this call; must outlive the registration.
      /// @param engine Engine about to run JavaScript; must outlive the registration.
      /// @param timeout Time limit for this call, or zero for no limit.
      /// @throws std::runtime_error if the controller has already been cancelled.
      Registration(ExecutionController& controller, QJSEngine& engine,
                   std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

      /// @brief Removes the engine from cancellation and timeout tracking.
      ~Registration();

      /// @brief Disables copying because each registration tracks one call.
      Registration(const Registration&) = delete;

      /// @brief Disables copy assignment to keep engine tracking unique.
      Registration& operator=(const Registration&) = delete;

      /// @brief Returns whether the watchdog stopped this call at its time limit.
      bool timedOut() const;

   private:
      friend class ExecutionController;
      friend class JsExecutionWatchdog;

      /// @brief Owner whose cancellation stops this call.
      ExecutionController& controller;
      /// @brief Engine that can be interrupted while this registration exists.
      QJSEngine& engine;
      /// @brief Time limit checked by the watchdog, or the maximum time point for no limit.
      std::chrono::steady_clock::time_point deadline;
      /// @brief Whether the watchdog interrupted this call at its time limit.
      bool expired = false;
   };

private:
   /// @brief Whether work has been cancelled, protected by the watchdog mutex.
   bool cancelled = false;
};

#endif  // EXECUTIONCONTROLLER_H
