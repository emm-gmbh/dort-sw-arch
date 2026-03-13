#pragma once

#include <stdint.h>

/* 0=pending, 1=ready, -1=failed, -2=timeout */
extern volatile int32_t g_rrdu_validation_result;
extern volatile int32_t g_rrdu_validation_reason;

void rrdu_validation_run_once(void);
