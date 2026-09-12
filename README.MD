# ESPressio-Platform-FreeRTOS

Generic FreeRTOS backend for `ESPressio-Platform`.

This repository is intentionally distinct from `ESPressio-Platform-ESP-IDF`: portable FreeRTOS behaviour belongs here, while Espressif port extensions such as ESP-specific affinity or capability-aware allocation belong in the ESP-IDF backend.

The repository is a clean-slate target backend and does not conform to legacy ESPressio platform contracts.

## Clock

`FreeRTOS::TickClock` uses only the portable FreeRTOS task tick APIs:

- task context: `xTaskGetTickCount()`
- interrupt context: `xTaskGetTickCountFromISR()`

It provides monotonic, interrupt-readable Platform Clock semantics with `ClockFrequencyHz = configTICK_RATE_HZ`, counter width derived from `TickType_t`, and conservative compile-time resolution metadata.

The provider preserves the native counter width rather than hiding wrap with a stateful software epoch extension.

## Synchronization

When `configSUPPORT_STATIC_ALLOCATION == 1` and `configUSE_MUTEXES == 1`, the backend exposes `FreeRTOS::StaticSynchronizationProvider`.

The provider supplies:

- `StaticMutex`, created with `xSemaphoreCreateMutexStatic`;
- `StaticSignal`, implemented with a statically allocated binary semaphore;
- `StaticRecursiveMutex` when `configUSE_RECURSIVE_MUTEXES == 1`.

The provider advertises `Capability::StaticSynchronizationStorage` and `Capability::InterruptSafeSignalling`; it additionally advertises `Capability::RecursiveMutex` only when the selected FreeRTOS configuration enables recursive mutexes.

`StaticSignal::GiveFromInterrupt()` uses the portable `xSemaphoreGiveFromISR` API and returns whether a higher-priority task was unblocked. The provider deliberately does not execute a port-specific ISR yield macro itself; scheduler handoff remains the caller/port integration responsibility.

All synchronization objects own their FreeRTOS control storage directly. No heap-backed creation fallback is used. `ControlStorageBytes` and `TotalStaticStorageBytes` are compile-time constants, and the implementation asserts that the total reported static storage equals the concrete object size.

## Queue

When `configSUPPORT_STATIC_ALLOCATION == 1`, the backend exposes `FreeRTOS::StaticQueueProvider`.

Its queue family is selected at compile time:

```cpp
using QueueProvider = Platform::ProviderFor<Capability::Queue>;
using MessageQueue = QueueProvider::Queue<Message, 32>;
```

Each `StaticQueue<T, Depth>`:

- is created with `xQueueCreateStatic`;
- owns its payload array, FreeRTOS `StaticQueue_t` control block, and native handle inside the object;
- exposes compile-time `Depth`, `ElementSizeBytes`, `PayloadStorageBytes`, `ControlStorageBytes`, and `TotalStaticStorageBytes`;
- supports blocking `Send`/`Receive`, immediate `TrySend`/`TryReceive`, `Size`, and `Reset`;
- advertises and implements `QueueSendFromInterrupt` and `QueueReceiveFromInterrupt` using the portable FreeRTOS `FromISR` APIs.

FreeRTOS queues copy object representations into queue storage, so this backend restricts `T` to trivially copyable types. That restriction belongs to this concrete provider; it is not imposed by the generic Platform Queue contract.

There is no dynamic queue-allocation fallback. If static allocation is not enabled by the selected FreeRTOS configuration, `StaticQueueProvider` is absent at compile time.

## Hardware-facing Platform capabilities

`ESPressio-Platform` defines I2C, SPI, GPIO, ByteStream and Entropy contracts, but this backend intentionally provides none of them. FreeRTOS is a scheduler/kernel and does not itself define portable hardware-controller, stream-device, or entropy-source APIs for these facilities.

Hardware/framework backends such as `ESPressio-Platform-Arduino`, `ESPressio-Platform-ESP-IDF`, hardware-specific libraries, or future MCU-specific providers can satisfy those capabilities in the same composition as the FreeRTOS Clock, Synchronization and Queue providers.
