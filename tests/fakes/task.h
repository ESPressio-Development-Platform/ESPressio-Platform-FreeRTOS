#pragma once
#include "FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
TickType_t xTaskGetTickCount(void);
TickType_t xTaskGetTickCountFromISR(void);
#ifdef __cplusplus
}
#endif
