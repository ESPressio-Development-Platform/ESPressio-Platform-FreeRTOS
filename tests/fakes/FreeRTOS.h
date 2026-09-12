#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;

#define pdFALSE 0
#define pdTRUE 1
#define pdFAIL 0
#define pdPASS 1

#define configTICK_RATE_HZ 1000U
#define configSUPPORT_STATIC_ALLOCATION 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1

#define portMAX_DELAY ((TickType_t)0xffffffffUL)

typedef struct StaticSemaphore {
    UBaseType_t Count;
    UBaseType_t RecursiveDepth;
    BaseType_t Recursive;
} StaticSemaphore_t;

typedef StaticSemaphore_t* SemaphoreHandle_t;

typedef struct StaticQueue {
    uint8_t* Storage;
    UBaseType_t Length;
    UBaseType_t ItemSize;
    UBaseType_t Head;
    UBaseType_t Tail;
    UBaseType_t Count;
} StaticQueue_t;

typedef StaticQueue_t* QueueHandle_t;
