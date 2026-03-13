#pragma once
#include <stdint.h>

/* RRDU debug stage markers for hardware bringup diagnostics. */
enum {
    RRDU_DBG_STAGE_RESET = 0,
    RRDU_DBG_STAGE_CONFIG = 1,
    RRDU_DBG_STAGE_HEALTH = 2,
    RRDU_DBG_STAGE_MEAS_START = 3,
    RRDU_DBG_STAGE_WAIT_MEAS = 4,
    RRDU_DBG_STAGE_READ_PUBLISH = 5,
    RRDU_DBG_STAGE_READY = 6,
    RRDU_DBG_STAGE_FAILED = 7
};

extern volatile int32_t g_rrdu_debug_state;
extern volatile uint32_t g_rrdu_debug_stage;
extern volatile uint32_t g_rrdu_debug_last_raw;
extern volatile uint32_t g_rrdu_debug_last_out;
extern volatile uint32_t g_rrdu_debug_fault;
extern volatile int32_t g_rrdu_debug_last_reason;

void rrdu_service_init(void);

/* Bring-up: ready olunca post_unit_ready(UNIT_RRDU) üretilecek */
void rrdu_service_bringup_start(void);

/* Donanım yokken zinciri ilerletmek için sim */
void rrdu_service_simulate_ready(void);

/* Nominal placeholder (şimdilik boş olabilir) */
int rrdu_service_measure_start(uint32_t meas_id);


void rrdu_service_on_meas_done(void);
void rrdu_service_on_meas_failed(int32_t st);

