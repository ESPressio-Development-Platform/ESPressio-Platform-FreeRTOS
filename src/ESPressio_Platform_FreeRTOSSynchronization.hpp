#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

#include <cstddef>

#include "ESPressio_PlatformSynchronization.hpp"
#include "ESPressio_Platform_FreeRTOS.hpp"

namespace ESPressio::Platform::FreeRTOS {

#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1) && \
    defined(configUSE_MUTEXES) && (configUSE_MUTEXES == 1)

class StaticMutex final {
private:
    struct Storage {
        StaticSemaphore_t Control{};
        SemaphoreHandle_t Handle{nullptr};
    };

public:
    static constexpr std::size_t ControlStorageBytes = sizeof(StaticSemaphore_t);
    static constexpr std::size_t TotalStaticStorageBytes = sizeof(Storage);

    StaticMutex() noexcept {
        storage_.Handle = ::xSemaphoreCreateMutexStatic(&storage_.Control);
    }

    StaticMutex(const StaticMutex&) = delete;
    StaticMutex& operator=(const StaticMutex&) = delete;
    StaticMutex(StaticMutex&&) = delete;
    StaticMutex& operator=(StaticMutex&&) = delete;

    void Lock() noexcept {
        while (::xSemaphoreTake(storage_.Handle, portMAX_DELAY) != pdTRUE) {}
    }

    bool TryLock() noexcept {
        return ::xSemaphoreTake(storage_.Handle, TickType_t{0}) == pdTRUE;
    }

    bool Unlock() noexcept {
        return ::xSemaphoreGive(storage_.Handle) == pdTRUE;
    }

private:
    Storage storage_{};
};

static_assert(Synchronization::IsMutexV<StaticMutex>,
              "FreeRTOS StaticMutex must satisfy the Platform mutex contract");
static_assert(sizeof(StaticMutex) == StaticMutex::TotalStaticStorageBytes,
              "FreeRTOS StaticMutex resource accounting must equal object size");

class StaticSignal final {
private:
    struct Storage {
        StaticSemaphore_t Control{};
        SemaphoreHandle_t Handle{nullptr};
    };

public:
    static constexpr std::size_t ControlStorageBytes = sizeof(StaticSemaphore_t);
    static constexpr std::size_t TotalStaticStorageBytes = sizeof(Storage);

    StaticSignal() noexcept {
        storage_.Handle = ::xSemaphoreCreateBinaryStatic(&storage_.Control);
    }

    StaticSignal(const StaticSignal&) = delete;
    StaticSignal& operator=(const StaticSignal&) = delete;
    StaticSignal(StaticSignal&&) = delete;
    StaticSignal& operator=(StaticSignal&&) = delete;

    void Give() noexcept {
        (void)::xSemaphoreGive(storage_.Handle);
    }

    void Wait() noexcept {
        while (::xSemaphoreTake(storage_.Handle, portMAX_DELAY) != pdTRUE) {}
    }

    bool TryWait() noexcept {
        return ::xSemaphoreTake(storage_.Handle, TickType_t{0}) == pdTRUE;
    }

    void Reset() noexcept {
        (void)::xSemaphoreTake(storage_.Handle, TickType_t{0});
    }

    Synchronization::InterruptSignalResult GiveFromInterrupt() noexcept {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        const BaseType_t result =
            ::xSemaphoreGiveFromISR(storage_.Handle, &higherPriorityTaskWoken);
        return {
            result == pdTRUE,
            higherPriorityTaskWoken == pdTRUE
        };
    }

private:
    Storage storage_{};
};

static_assert(Synchronization::IsSignalV<StaticSignal>,
              "FreeRTOS StaticSignal must satisfy the Platform signal contract");
static_assert(Synchronization::IsInterruptSignalV<StaticSignal>,
              "FreeRTOS StaticSignal must satisfy the interrupt-safe signal contract");
static_assert(sizeof(StaticSignal) == StaticSignal::TotalStaticStorageBytes,
              "FreeRTOS StaticSignal resource accounting must equal object size");

#if defined(configUSE_RECURSIVE_MUTEXES) && (configUSE_RECURSIVE_MUTEXES == 1)

class StaticRecursiveMutex final {
private:
    struct Storage {
        StaticSemaphore_t Control{};
        SemaphoreHandle_t Handle{nullptr};
    };

public:
    static constexpr std::size_t ControlStorageBytes = sizeof(StaticSemaphore_t);
    static constexpr std::size_t TotalStaticStorageBytes = sizeof(Storage);

    StaticRecursiveMutex() noexcept {
        storage_.Handle = ::xSemaphoreCreateRecursiveMutexStatic(&storage_.Control);
    }

    StaticRecursiveMutex(const StaticRecursiveMutex&) = delete;
    StaticRecursiveMutex& operator=(const StaticRecursiveMutex&) = delete;
    StaticRecursiveMutex(StaticRecursiveMutex&&) = delete;
    StaticRecursiveMutex& operator=(StaticRecursiveMutex&&) = delete;

    void Lock() noexcept {
        while (::xSemaphoreTakeRecursive(storage_.Handle, portMAX_DELAY) != pdTRUE) {}
    }

    bool TryLock() noexcept {
        return ::xSemaphoreTakeRecursive(storage_.Handle, TickType_t{0}) == pdTRUE;
    }

    bool Unlock() noexcept {
        return ::xSemaphoreGiveRecursive(storage_.Handle) == pdTRUE;
    }

private:
    Storage storage_{};
};

static_assert(Synchronization::IsMutexV<StaticRecursiveMutex>,
              "FreeRTOS StaticRecursiveMutex must satisfy the Platform mutex contract");
static_assert(sizeof(StaticRecursiveMutex) == StaticRecursiveMutex::TotalStaticStorageBytes,
              "FreeRTOS StaticRecursiveMutex resource accounting must equal object size");

struct StaticSynchronizationProvider final
    : Synchronization::ProviderDeclaration<
          Backend,
          CapabilitySet<
              Capability::StaticSynchronizationStorage,
              Capability::RecursiveMutex,
              Capability::InterruptSafeSignalling>> {
    using Mutex = StaticMutex;
    using RecursiveMutex = StaticRecursiveMutex;
    using Signal = StaticSignal;
};

#else

struct StaticSynchronizationProvider final
    : Synchronization::ProviderDeclaration<
          Backend,
          CapabilitySet<
              Capability::StaticSynchronizationStorage,
              Capability::InterruptSafeSignalling>> {
    using Mutex = StaticMutex;
    using Signal = StaticSignal;
};

#endif

static_assert(Synchronization::IsProviderV<StaticSynchronizationProvider>,
              "FreeRTOS StaticSynchronizationProvider must satisfy the Platform synchronization provider contract");

#endif

} // namespace ESPressio::Platform::FreeRTOS
