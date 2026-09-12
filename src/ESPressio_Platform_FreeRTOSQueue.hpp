#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
}

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "ESPressio_PlatformQueue.hpp"
#include "ESPressio_Platform_FreeRTOS.hpp"

namespace ESPressio::Platform::FreeRTOS {

#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)

template <typename T, std::size_t TDepth>
class StaticQueue final {
public:
    static_assert(TDepth > 0U,
                  "FreeRTOS StaticQueue depth must be greater than zero");
    static_assert(std::is_trivially_copyable_v<T>,
                  "FreeRTOS queues copy object representations; element type must be trivially copyable");
    static_assert(TDepth <= static_cast<std::size_t>(std::numeric_limits<UBaseType_t>::max()),
                  "FreeRTOS StaticQueue depth exceeds UBaseType_t range");
    static_assert(sizeof(T) <= static_cast<std::size_t>(std::numeric_limits<UBaseType_t>::max()),
                  "FreeRTOS StaticQueue element size exceeds UBaseType_t range");

    static constexpr std::size_t Depth = TDepth;
    static constexpr std::size_t ElementSizeBytes = sizeof(T);
    static constexpr std::size_t PayloadStorageBytes = sizeof(T) * TDepth;
    static constexpr std::size_t ControlStorageBytes = sizeof(StaticQueue_t);

private:
    struct Storage {
        std::array<std::uint8_t, PayloadStorageBytes> Payload{};
        StaticQueue_t Control{};
        QueueHandle_t Handle{nullptr};
    };

public:
    static constexpr std::size_t TotalStaticStorageBytes = sizeof(Storage);

    StaticQueue() noexcept {
        storage_.Handle = ::xQueueCreateStatic(
            static_cast<UBaseType_t>(TDepth),
            static_cast<UBaseType_t>(sizeof(T)),
            storage_.Payload.data(),
            &storage_.Control);
    }

    StaticQueue(const StaticQueue&) = delete;
    StaticQueue& operator=(const StaticQueue&) = delete;
    StaticQueue(StaticQueue&&) = delete;
    StaticQueue& operator=(StaticQueue&&) = delete;

    void Send(const T& value) noexcept {
        while (::xQueueSend(storage_.Handle, &value, portMAX_DELAY) != pdPASS) {}
    }

    bool TrySend(const T& value) noexcept {
        return ::xQueueSend(storage_.Handle, &value, TickType_t{0}) == pdPASS;
    }

    void Receive(T& value) noexcept {
        while (::xQueueReceive(storage_.Handle, &value, portMAX_DELAY) != pdPASS) {}
    }

    bool TryReceive(T& value) noexcept {
        return ::xQueueReceive(storage_.Handle, &value, TickType_t{0}) == pdPASS;
    }

    std::size_t Size() const noexcept {
        return static_cast<std::size_t>(::uxQueueMessagesWaiting(storage_.Handle));
    }

    void Reset() noexcept {
        (void)::xQueueReset(storage_.Handle);
    }

    Queue::InterruptOperationResult SendFromInterrupt(const T& value) noexcept {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        const BaseType_t result =
            ::xQueueSendFromISR(storage_.Handle, &value, &higherPriorityTaskWoken);
        return {
            result == pdPASS,
            higherPriorityTaskWoken == pdTRUE
        };
    }

    Queue::InterruptOperationResult ReceiveFromInterrupt(T& value) noexcept {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        const BaseType_t result =
            ::xQueueReceiveFromISR(storage_.Handle, &value, &higherPriorityTaskWoken);
        return {
            result == pdPASS,
            higherPriorityTaskWoken == pdTRUE
        };
    }

private:
    Storage storage_{};
};

template <typename T, std::size_t TDepth>
inline constexpr bool StaticQueueResourceAccountingValid =
    sizeof(StaticQueue<T, TDepth>) == StaticQueue<T, TDepth>::TotalStaticStorageBytes;

struct StaticQueueProvider final
    : Queue::ProviderDeclaration<
          Backend,
          CapabilitySet<
              Capability::StaticQueueStorage,
              Capability::QueueSendFromInterrupt,
              Capability::QueueReceiveFromInterrupt>> {
    template <typename T, std::size_t TDepth>
    using Queue = StaticQueue<T, TDepth>;
};

static_assert(Queue::IsQueueV<StaticQueue<std::uint32_t, 2U>, std::uint32_t, 2U>,
              "FreeRTOS StaticQueue must satisfy the Platform queue contract");
static_assert(Queue::IsInterruptQueueV<StaticQueue<std::uint32_t, 2U>, std::uint32_t>,
              "FreeRTOS StaticQueue must satisfy the Platform interrupt queue contract");
static_assert(Queue::IsStaticQueueV<StaticQueue<std::uint32_t, 2U>>,
              "FreeRTOS StaticQueue must satisfy the Platform static-storage queue contract");
static_assert(StaticQueueResourceAccountingValid<std::uint32_t, 2U>,
              "FreeRTOS StaticQueue resource accounting must equal object size");
static_assert(Queue::IsProviderV<StaticQueueProvider>,
              "FreeRTOS StaticQueueProvider must satisfy the Platform queue provider contract");

#endif

} // namespace ESPressio::Platform::FreeRTOS
