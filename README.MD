# ESPressio-Platform-FreeRTOS

Generic FreeRTOS backend for `ESPressio-Platform`.

This repository is intentionally distinct from `ESPressio-Platform-ESP-IDF`: portable FreeRTOS behaviour belongs here, while Espressif port extensions such as ESP-specific affinity or capability-aware allocation belong in the ESP-IDF backend.

The repository is a clean-slate target backend and does not conform to legacy ESPressio platform contracts.

## Clock

`FreeRTOS::TickClock` is the first concrete provider. It uses only the portable FreeRTOS task tick APIs:

- task context: `xTaskGetTickCount()`
- interrupt context: `xTaskGetTickCountFromISR()`

It provides:

- `Capability::Clock`
- `Capability::MonotonicClock`
- `Capability::InterruptReadableClock`
- `ClockFrequencyHz = configTICK_RATE_HZ`
- `ClockCounterWidthBits` derived from `TickType_t`
- `ClockResolutionNanoseconds` conservatively rounded up from the configured tick frequency

The provider deliberately preserves the native FreeRTOS counter width instead of hiding wrap with a stateful software extension. `Platform::Clock::Elapsed()` performs the corresponding modular elapsed-tick calculation.

No ESP-IDF FreeRTOS extensions are used.
