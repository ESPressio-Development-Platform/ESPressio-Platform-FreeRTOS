#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

#include <limits>
#include <type_traits>

#include "ESPressio_PlatformClock.hpp"
#include "ESPressio_Platform_FreeRTOS.hpp"

namespace ESPressio::Platform::FreeRTOS {

namespace Detail {

inline constexpr std::uint64_t TickFrequencyHz =
    static_cast<std::uint64_t>(configTICK_RATE_HZ);

static_assert(TickFrequencyHz > 0U,
              "FreeRTOS configTICK_RATE_HZ must be greater than zero");

} // namespace Detail

/**
 * Monotonic scheduler-tick clock backed only by portable FreeRTOS APIs.
 *
 * The source intentionally exposes the native TickType_t wrap width rather
 * than attempting a hidden software epoch extension. Task and ISR reads use
 * the corresponding FreeRTOS APIs and share the same tick domain.
 */
class TickClock final
    : public Clock::MonotonicProviderDeclaration<
          Backend,
          Detail::TickFrequencyHz,
          std::numeric_limits<TickType_t>::digits,
          PropertySet<
              PropertyValue<
                  PropertyKey::ClockResolutionNanoseconds,
                  Clock::ResolutionNanosecondsForFrequency(
                      Detail::TickFrequencyHz)>>,
          CapabilitySet<Capability::InterruptReadableClock>> {
public:
    static_assert(std::is_integral_v<TickType_t> &&
                      std::is_unsigned_v<TickType_t>,
                  "FreeRTOS TickType_t must be an unsigned integral type");

    Clock::Tick Now() const noexcept {
        return static_cast<Clock::Tick>(::xTaskGetTickCount());
    }

    Clock::Tick NowFromInterrupt() const noexcept {
        return static_cast<Clock::Tick>(::xTaskGetTickCountFromISR());
    }
};

static_assert(Clock::IsClockSourceV<TickClock>,
              "FreeRTOS TickClock must satisfy the Platform Clock contract");
static_assert(Clock::IsInterruptReadableClockSourceV<TickClock>,
              "FreeRTOS TickClock must satisfy the interrupt-readable Clock contract");

} // namespace ESPressio::Platform::FreeRTOS
