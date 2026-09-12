#pragma once

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* buffer) {
    if (buffer == NULL) return NULL;
    buffer->Count = 1U;
    buffer->RecursiveDepth = 0U;
    buffer->Recursive = pdFALSE;
    return buffer;
}

static inline SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t* buffer) {
    if (buffer == NULL) return NULL;
    buffer->Count = 1U;
    buffer->RecursiveDepth = 0U;
    buffer->Recursive = pdTRUE;
    return buffer;
}

static inline SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* buffer) {
    if (buffer == NULL) return NULL;
    buffer->Count = 0U;
    buffer->RecursiveDepth = 0U;
    buffer->Recursive = pdFALSE;
    return buffer;
}

static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t handle, TickType_t ticks) {
    (void)ticks;
    if (handle == NULL || handle->Count == 0U) return pdFALSE;
    handle->Count = 0U;
    return pdTRUE;
}

static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t handle) {
    if (handle == NULL || handle->Count != 0U) return pdFALSE;
    handle->Count = 1U;
    return pdTRUE;
}

static inline BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t handle, TickType_t ticks) {
    (void)ticks;
    if (handle == NULL || handle->Recursive == pdFALSE) return pdFALSE;
    if (handle->RecursiveDepth > 0U) {
        ++handle->RecursiveDepth;
        return pdTRUE;
    }
    if (handle->Count == 0U) return pdFALSE;
    handle->Count = 0U;
    handle->RecursiveDepth = 1U;
    return pdTRUE;
}

static inline BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t handle) {
    if (handle == NULL || handle->Recursive == pdFALSE || handle->RecursiveDepth == 0U)
        return pdFALSE;
    --handle->RecursiveDepth;
    if (handle->RecursiveDepth == 0U) handle->Count = 1U;
    return pdTRUE;
}

static inline BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t handle,
                                                BaseType_t* higherPriorityTaskWoken) {
    if (higherPriorityTaskWoken != NULL) *higherPriorityTaskWoken = pdFALSE;
    return xSemaphoreGive(handle);
}

#ifdef __cplusplus
}
#endif
