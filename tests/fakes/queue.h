#pragma once

#include "FreeRTOS.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline QueueHandle_t xQueueCreateStatic(UBaseType_t length,
                                                UBaseType_t itemSize,
                                                uint8_t* storage,
                                                StaticQueue_t* control) {
    if (length == 0U || itemSize == 0U || storage == NULL || control == NULL) return NULL;
    control->Storage = storage;
    control->Length = length;
    control->ItemSize = itemSize;
    control->Head = 0U;
    control->Tail = 0U;
    control->Count = 0U;
    return control;
}

static inline BaseType_t xQueueSend(QueueHandle_t handle,
                                     const void* item,
                                     TickType_t ticks) {
    (void)ticks;
    if (handle == NULL || item == NULL || handle->Count >= handle->Length) return pdFAIL;
    memcpy(handle->Storage + (handle->Tail * handle->ItemSize), item, handle->ItemSize);
    handle->Tail = (handle->Tail + 1U) % handle->Length;
    ++handle->Count;
    return pdPASS;
}

static inline BaseType_t xQueueReceive(QueueHandle_t handle,
                                        void* item,
                                        TickType_t ticks) {
    (void)ticks;
    if (handle == NULL || item == NULL || handle->Count == 0U) return pdFAIL;
    memcpy(item, handle->Storage + (handle->Head * handle->ItemSize), handle->ItemSize);
    handle->Head = (handle->Head + 1U) % handle->Length;
    --handle->Count;
    return pdPASS;
}

static inline UBaseType_t uxQueueMessagesWaiting(QueueHandle_t handle) {
    return handle == NULL ? 0U : handle->Count;
}

static inline BaseType_t xQueueReset(QueueHandle_t handle) {
    if (handle == NULL) return pdFAIL;
    handle->Head = 0U;
    handle->Tail = 0U;
    handle->Count = 0U;
    return pdPASS;
}

static inline BaseType_t xQueueSendFromISR(QueueHandle_t handle,
                                            const void* item,
                                            BaseType_t* higherPriorityTaskWoken) {
    if (higherPriorityTaskWoken != NULL) *higherPriorityTaskWoken = pdFALSE;
    return xQueueSend(handle, item, TickType_t{0});
}

static inline BaseType_t xQueueReceiveFromISR(QueueHandle_t handle,
                                               void* item,
                                               BaseType_t* higherPriorityTaskWoken) {
    if (higherPriorityTaskWoken != NULL) *higherPriorityTaskWoken = pdFALSE;
    return xQueueReceive(handle, item, TickType_t{0});
}

#ifdef __cplusplus
}
#endif
