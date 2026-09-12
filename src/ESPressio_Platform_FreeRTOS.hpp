#pragma once

#include "ESPressio_Platform.hpp"

namespace ESPressio::Platform::FreeRTOS {

/** Compile-time identity of the generic FreeRTOS backend. */
struct Backend final : ESPressio::Platform::Backend {};

/**
 * Convenience alias for declaring generic FreeRTOS-backed providers without
 * repeating the backend identity. Concrete providers explicitly declare the
 * capabilities they supply and any capabilities they require.
 */
template <typename TCapabilities,
          typename TRequirements = ESPressio::Platform::RequirementSet<>>
using ProviderDeclaration = ESPressio::Platform::ProviderDeclaration<
    Backend,
    TCapabilities,
    TRequirements>;

} // namespace ESPressio::Platform::FreeRTOS

#include "ESPressio_Platform_FreeRTOSClock.hpp"
#include "ESPressio_Platform_FreeRTOSSynchronization.hpp"
#include "ESPressio_Platform_FreeRTOSQueue.hpp"
