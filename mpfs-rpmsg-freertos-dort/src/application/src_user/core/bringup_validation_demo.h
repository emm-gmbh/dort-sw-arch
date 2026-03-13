#pragma once

#include <stdint.h>

/* 1 = bringup done, -1 = bringup failed, -2 = timeout */
extern volatile int32_t g_bringup_validation_result;
extern volatile int32_t g_bringup_validation_state;
extern volatile uint32_t g_bringup_trace_len;
extern volatile uint32_t g_bringup_trace[64];

void bringup_validation_run_once(void);
