# ESPressio-Platform-FreeRTOS

Generic FreeRTOS backend for `ESPressio-Platform`.

This repository is intentionally distinct from `ESPressio-Platform-ESP-IDF`: portable FreeRTOS behaviour belongs here, while Espressif port extensions such as ESP-specific affinity or capability-aware allocation belong in the ESP-IDF backend.

The repository is being built as a clean-slate target backend and does not conform to legacy ESPressio platform contracts.

The initial capability groundwork defines only the distinct `ESPressio::Platform::FreeRTOS::Backend` identity and the `FreeRTOS::ProviderDeclaration` convenience alias. It deliberately makes no service-capability claim until the corresponding generic FreeRTOS provider exists and has been validated across the intended FreeRTOS contract.
