#ifndef TESTGENERATORS_H
#define TESTGENERATORS_H

#include "generators/texturegenerator.h"
#include <QSemaphore>
#include <atomic>

/// @brief Deterministic generator with synchronization and failure controls for tests.
class RecordingGenerator final : public TextureGenerator {
public:
   /// @brief Constructs a generator with a configurable name, inputs, and output value.
   /// @param name Generator name exposed through the production interface.
   /// @param sourceSlots Number of accepted source slots.
   /// @param defaultValue Default grayscale channel value.
   explicit RecordingGenerator(QString name = QStringLiteral("Recording"), int sourceSlots = 0,
                               int defaultValue = 17);

   /// @brief Records the call, optionally blocks or fails, and fills the destination.
   /// @param size Requested output dimensions.
   /// @param destination Output pixel buffer.
   /// @param sources Images connected to the generator's source slots.
   /// @param settings Node settings containing the optional grayscale value.
   void generate(QSize size, TexturePixel* destination, QMap<int, TextureImagePtr> sources,
                 TextureNodeSettings* settings) const override;

   /// @brief Returns the test generator's setting schema.
   const TextureGeneratorSettings& getSettings() const override { return schema; }

   /// @brief Returns filter type when source slots exist, otherwise generator type.
   Type getType() const override { return sourceSlotCount == 0 ? Type::Generator : Type::Filter; }

   /// @brief Returns the configured number of source slots.
   int getNumSourceSlots() const override { return sourceSlotCount; }

   /// @brief Returns the configured generator name.
   QString getName() const override { return generatorName; }

   /// @brief Returns a short description identifying the test generator.
   QString getDescription() const override { return QStringLiteral("Test generator"); }

   /// @brief Blocks the next generation call until release() is invoked.
   void block() { blocking = true; }

   /// @brief Makes subsequent generation calls throw a planned exception.
   void fail() { throwing = true; }

   /// @brief Waits for a generation call to begin.
   /// @param timeoutMilliseconds Maximum time to wait.
   /// @return True when generation started before the timeout.
   bool waitUntilStarted(int timeoutMilliseconds = 5000) const;

   /// @brief Releases one generation call waiting after block().
   void release() { gate.release(); }

   /// @brief Returns the total number of generation calls.
   int callCount() const { return calls; }

private:
   /// @brief Name exposed through the generator interface.
   QString generatorName;
   /// @brief Number of accepted source slots.
   int sourceSlotCount;
   /// @brief Schema containing the configurable grayscale value.
   TextureGeneratorSettings schema;
   /// @brief Signals that a generation call has begun.
   mutable QSemaphore started;
   /// @brief Controls release of a deliberately blocked generation call.
   mutable QSemaphore gate;
   /// @brief Requests that the next generation call wait on the gate.
   mutable std::atomic_bool blocking = false;
   /// @brief Requests a planned exception from generation calls.
   mutable std::atomic_bool throwing = false;
   /// @brief Counts generation calls across worker threads.
   mutable std::atomic_int calls = 0;
};

#endif  // TESTGENERATORS_H
