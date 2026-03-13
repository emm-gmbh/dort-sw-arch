#include "rrdu_service.h"
#include "drivers/rrdu/rrdu_hal.h"
#include "core/event_queue.h"
#include "core/units.h"
#include "FreeRTOS.h"

#ifdef HOST_SIM
#include <stdio.h>
#define RRDU_LOG(...) do { printf(__VA_ARGS__); } while (0)
#else
#define RRDU_LOG(...) do { } while (0)
#endif

/*
 * Fake HAL completion in this project currently invokes callback from task
 * context. Keep this default for simulation. Set to 1 when wired to real ISR.
 */
#ifndef RRDU_MEAS_CB_ISR_CONTEXT
#define RRDU_MEAS_CB_ISR_CONTEXT 0
#endif

/* 0 = happy path, 1 = fault path (simde fault inject)  */
#ifndef RRDU_INJECT_FAULT
#define RRDU_INJECT_FAULT 1
#endif

typedef enum {
    RRDU_SVC_RESET = 0,

    RRDU_SVC_BU_CONFIG,
    RRDU_SVC_BU_HEALTH,
    RRDU_SVC_BU_MEAS_START,
    RRDU_SVC_BU_WAIT_MEAS,
    RRDU_SVC_BU_READ_AND_PUBLISH,

    RRDU_SVC_READY,
    RRDU_SVC_FAILED
} rrdu_state_t;

static rrdu_state_t s_state = RRDU_SVC_RESET;

static uint32_t s_last_raw = 0;
static uint32_t s_last_out = 0;
static uint32_t s_meas_id = 1;

volatile int32_t g_rrdu_debug_state = 0;
volatile uint32_t g_rrdu_debug_stage = RRDU_DBG_STAGE_RESET;
volatile uint32_t g_rrdu_debug_last_raw = 0;
volatile uint32_t g_rrdu_debug_last_out = 0;
volatile uint32_t g_rrdu_debug_fault = 0;
volatile int32_t g_rrdu_debug_last_reason = 0;

static void rrdu_advance(void);

static void rrdu_debug_snapshot(uint32_t stage, int32_t reason)
{
    uint32_t fault = 0u;

    g_rrdu_debug_state = (int32_t)s_state;
    g_rrdu_debug_stage = stage;
    g_rrdu_debug_last_raw = s_last_raw;
    g_rrdu_debug_last_out = s_last_out;
    g_rrdu_debug_last_reason = reason;

    if (rrdu_hal_read_fault(&fault) == RRDU_HAL_OK) {
        g_rrdu_debug_fault = fault;
    }
}

static uint32_t rrdu_scale_raw_to_u32(uint32_t raw)
{
    return raw;
}

static void rrdu_meas_cb(void *user, rrdu_hal_status_t st)
{
    event_t e = {0};

    (void)user;

    if (s_state != RRDU_SVC_BU_WAIT_MEAS) {
        return;
    }

    if (st == RRDU_HAL_OK) {
        e.type = EVT_RRDU_MEAS_DONE;
        e.data.rrdu.st = 0;
    } else {
        e.type = EVT_RRDU_MEAS_FAILED;
        e.data.rrdu.st = (int32_t)st;
    }

#if RRDU_MEAS_CB_ISR_CONTEXT
    {
        BaseType_t hpw = pdFALSE;
        bool ok = eventq_push_from_isr(&e, &hpw);
        configASSERT(ok == true);
        if (hpw == pdTRUE) {
            portYIELD_WITHIN_API();
        }
    }
#else
    {
        bool ok = eventq_push(&e);
        configASSERT(ok == true);
    }
#endif
}

void rrdu_service_init(void)
{
    rrdu_hal_init();
    s_state = RRDU_SVC_RESET;
    s_last_raw = 0;
    s_last_out = 0;
    s_meas_id = 1;

    rrdu_debug_snapshot(RRDU_DBG_STAGE_RESET, 0);
}

void rrdu_service_bringup_start(void)
{
    RRDU_LOG("[RRDU] bringup_start\n");

    /* Idempotent: ignore repeated start if already started. */
    if (s_state != RRDU_SVC_RESET) {
        return;
    }

    s_state = RRDU_SVC_BU_CONFIG;
    rrdu_debug_snapshot(RRDU_DBG_STAGE_CONFIG, 0);
    rrdu_advance();
}

static void rrdu_advance(void)
{
    while (1) {
        if (s_state == RRDU_SVC_BU_CONFIG) {
            RRDU_LOG("[RRDU] CONFIG\n");
            rrdu_debug_snapshot(RRDU_DBG_STAGE_CONFIG, 0);

            if (rrdu_hal_configure_defaults() != RRDU_HAL_OK) {
                s_state = RRDU_SVC_FAILED;
                rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, -100);
                post_unit_failed(UNIT_RRDU, -100);
                return;
            }
            s_state = RRDU_SVC_BU_HEALTH;
            continue;
        }

        if (s_state == RRDU_SVC_BU_HEALTH) {
            uint32_t stat = 0;
            uint32_t fault = 0;

            RRDU_LOG("[RRDU] HEALTH\n");
            rrdu_debug_snapshot(RRDU_DBG_STAGE_HEALTH, 0);

            if ((rrdu_hal_read_stat(&stat) != RRDU_HAL_OK) ||
                (rrdu_hal_read_fault(&fault) != RRDU_HAL_OK)) {
                s_state = RRDU_SVC_FAILED;
                rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, -101);
                post_unit_failed(UNIT_RRDU, -101);
                return;
            }

            if (fault != 0u) {
                RRDU_LOG("[RRDU] HEALTH fault=0x%08lx\n", (unsigned long)fault);
                s_state = RRDU_SVC_FAILED;
                rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, (int32_t)fault);
                post_unit_failed(UNIT_RRDU, (int32_t)fault);
                return;
            }

            RRDU_LOG("[RRDU] HEALTH ok\n");
            s_state = RRDU_SVC_BU_MEAS_START;
            continue;
        }

        if (s_state == RRDU_SVC_BU_MEAS_START) {
            RRDU_LOG("[RRDU] MEAS_START id=%lu\n", (unsigned long)s_meas_id);
            rrdu_debug_snapshot(RRDU_DBG_STAGE_MEAS_START, 0);

            /* Set wait state before async start so immediate completion is not lost. */
            s_state = RRDU_SVC_BU_WAIT_MEAS;
            rrdu_debug_snapshot(RRDU_DBG_STAGE_WAIT_MEAS, 0);

            if (rrdu_hal_meas_start_async(s_meas_id, rrdu_meas_cb, 0) != RRDU_HAL_OK) {
                s_state = RRDU_SVC_FAILED;
                rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, -102);
                post_unit_failed(UNIT_RRDU, -102);
                return;
            }
            return;
        }

        if (s_state == RRDU_SVC_BU_READ_AND_PUBLISH) {
            RRDU_LOG("[RRDU] READ_AND_PUBLISH\n");
            rrdu_debug_snapshot(RRDU_DBG_STAGE_READ_PUBLISH, 0);

            if (rrdu_hal_read_meas_raw(s_meas_id, &s_last_raw) != RRDU_HAL_OK) {
                s_state = RRDU_SVC_FAILED;
                rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, -103);
                post_unit_failed(UNIT_RRDU, -103);
                return;
            }

            s_last_out = rrdu_scale_raw_to_u32(s_last_raw);
            (void)rrdu_hal_write_output_u32(s_last_out);

            RRDU_LOG("[RRDU] PUBLISH raw=0x%08lx out=0x%08lx\n",
                     (unsigned long)s_last_raw,
                     (unsigned long)s_last_out);
            RRDU_LOG("[RRDU] READY\n");

            s_state = RRDU_SVC_READY;
            rrdu_debug_snapshot(RRDU_DBG_STAGE_READY, 0);
            post_unit_ready(UNIT_RRDU);
            return;
        }

        return;
    }
}

/* Host sim: trigger fake completion. */
void rrdu_service_simulate_ready(void)
{
#if defined(RRDU_HAL_BACKEND_MMIO) && (RRDU_HAL_BACKEND_MMIO == 1)
    /* In MMIO backend mode, simulation trigger is intentionally disabled. */
    return;
#else
    if ((s_state != RRDU_SVC_BU_WAIT_MEAS) && (s_state != RRDU_SVC_BU_MEAS_START)) {
        return;
    }

    RRDU_LOG("[RRDU] inject_fault=%d\n", (int)RRDU_INJECT_FAULT);

    rrdu_hal_fake_complete_meas(
        s_meas_id,
        0x12345678u,
        RRDU_INJECT_FAULT ? 0x1u : 0u
    );
#endif
}

int rrdu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != RRDU_SVC_READY) {
        return -1;
    }
    return -2;
}

void rrdu_service_on_meas_done(void)
{
    if (s_state != RRDU_SVC_BU_WAIT_MEAS) {
        return;
    }

    s_state = RRDU_SVC_BU_READ_AND_PUBLISH;
    rrdu_debug_snapshot(RRDU_DBG_STAGE_READ_PUBLISH, 0);
    rrdu_advance();
}

void rrdu_service_on_meas_failed(int32_t st)
{
    uint32_t fault = 0u;

    if (s_state != RRDU_SVC_BU_WAIT_MEAS) {
        return;
    }

    s_state = RRDU_SVC_FAILED;

    /* Prefer backend-specific fault detail when available. */
    if ((rrdu_hal_read_fault(&fault) == RRDU_HAL_OK) && (fault != 0u)) {
        rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, (int32_t)fault);
        post_unit_failed(UNIT_RRDU, (int32_t)fault);
    } else {
        rrdu_debug_snapshot(RRDU_DBG_STAGE_FAILED, st);
        post_unit_failed(UNIT_RRDU, st);
    }
}
