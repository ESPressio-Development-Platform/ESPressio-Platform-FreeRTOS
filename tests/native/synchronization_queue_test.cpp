#include <cassert>
#include <cstdint>
#include <type_traits>

#include "ESPressio_Platform_FreeRTOS.hpp"

using namespace ESPressio::Platform;

#if !defined(configSUPPORT_STATIC_ALLOCATION) || (configSUPPORT_STATIC_ALLOCATION != 1)
#error "Native test requires static allocation"
#endif

#if !defined(configUSE_MUTEXES) || (configUSE_MUTEXES != 1)
#error "Native test requires mutex support"
#endif

using SyncProvider = FreeRTOS::StaticSynchronizationProvider;
using QueueProvider = FreeRTOS::StaticQueueProvider;
using TestPlatform = Composition<SyncProvider, QueueProvider>;
using TestQueue = QueueProvider::Queue<std::uint32_t, 3U>;

static_assert(Synchronization::IsProviderV<SyncProvider>);
static_assert(Queue::IsProviderV<QueueProvider>);
static_assert(TestPlatform::Provides<Capability::Synchronization>);
static_assert(TestPlatform::Provides<Capability::Queue>);
static_assert(TestPlatform::Provides<Capability::StaticSynchronizationStorage>);
static_assert(TestPlatform::Provides<Capability::StaticQueueStorage>);
static_assert(TestPlatform::Provides<Capability::InterruptSafeSignalling>);
static_assert(TestPlatform::Provides<Capability::QueueSendFromInterrupt>);
static_assert(TestPlatform::Provides<Capability::QueueReceiveFromInterrupt>);
static_assert(Synchronization::IsMutexV<SyncProvider::Mutex>);
static_assert(Synchronization::IsSignalV<SyncProvider::Signal>);
static_assert(Synchronization::IsInterruptSignalV<SyncProvider::Signal>);
static_assert(Queue::IsQueueV<TestQueue, std::uint32_t, 3U>);
static_assert(Queue::IsInterruptQueueV<TestQueue, std::uint32_t>);
static_assert(Queue::IsStaticQueueV<TestQueue>);
static_assert(TestQueue::PayloadStorageBytes == sizeof(std::uint32_t) * 3U);
static_assert(TestQueue::TotalStaticStorageBytes == sizeof(TestQueue));

#if defined(configUSE_RECURSIVE_MUTEXES) && (configUSE_RECURSIVE_MUTEXES == 1)
static_assert(TestPlatform::Provides<Capability::RecursiveMutex>);
static_assert(Synchronization::IsMutexV<SyncProvider::RecursiveMutex>);
#endif

int main() {
    SyncProvider::Mutex mutex;
    assert(mutex.TryLock());
    assert(!mutex.TryLock());
    assert(mutex.Unlock());

#if defined(configUSE_RECURSIVE_MUTEXES) && (configUSE_RECURSIVE_MUTEXES == 1)
    SyncProvider::RecursiveMutex recursive;
    assert(recursive.TryLock());
    assert(recursive.TryLock());
    assert(recursive.Unlock());
    assert(recursive.Unlock());
#endif

    SyncProvider::Signal signal;
    assert(!signal.TryWait());
    signal.Give();
    assert(signal.TryWait());
    const auto signalResult = signal.GiveFromInterrupt();
    assert(signalResult.StateChanged);
    assert(!signalResult.HigherPriorityWaiterWoken);
    assert(signal.TryWait());
    signal.Give();
    signal.Reset();
    assert(!signal.TryWait());

    TestQueue queue;
    assert(queue.Size() == 0U);
    assert(queue.TrySend(10U));
    queue.Send(20U);
    assert(queue.Size() == 2U);

    std::uint32_t value = 0U;
    assert(queue.TryReceive(value));
    assert(value == 10U);
    queue.Receive(value);
    assert(value == 20U);

    const auto sendFromInterrupt = queue.SendFromInterrupt(30U);
    assert(sendFromInterrupt.Succeeded);
    assert(!sendFromInterrupt.HigherPriorityWaiterWoken);
    const auto receiveFromInterrupt = queue.ReceiveFromInterrupt(value);
    assert(receiveFromInterrupt.Succeeded);
    assert(value == 30U);

    assert(queue.TrySend(40U));
    queue.Reset();
    assert(queue.Size() == 0U);
    return 0;
}
