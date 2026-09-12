#include "ESPressio_Platform_FreeRTOS.hpp"

extern "C" TickType_t xTaskGetTickCount(void) {
    return 100U;
}

extern "C" TickType_t xTaskGetTickCountFromISR(void) {
    return 101U;
}

using Clock = ESPressio::Platform::FreeRTOS::TickClock;

static_assert(ESPressio::Platform::Clock::IsClockSourceV<Clock>);
static_assert(ESPressio::Platform::Clock::IsInterruptReadableClockSourceV<Clock>);
static_assert(ESPressio::Platform::Clock::FrequencyHz<Clock> == 1000ULL);
static_assert(ESPressio::Platform::Clock::CounterWidthBits<Clock> == 32U);
static_assert(
    Clock::PlatformCapabilities::template PropertiesFor<
        ESPressio::Platform::Capability::Clock>::template Value<
            ESPressio::Platform::PropertyKey::ClockResolutionNanoseconds> ==
    1'000'000ULL);

int main() {
    Clock clock;
    return clock.Now() == 100U && clock.NowFromInterrupt() == 101U ? 0 : 1;
}
