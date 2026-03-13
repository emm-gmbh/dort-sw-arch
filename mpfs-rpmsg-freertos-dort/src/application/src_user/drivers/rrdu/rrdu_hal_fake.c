#include "rrdu_hal.h"

#ifndef RRDU_FAKE_AUTO_COMPLETE
#define RRDU_FAKE_AUTO_COMPLETE 1
#endif

#ifndef RRDU_INJECT_FAULT
#define RRDU_INJECT_FAULT 1
#endif

static uint32_t s_stat = 0;
static uint32_t s_fault = 0;

static uint32_t s_last_meas_id = 0;
static uint32_t s_last_raw = 0;
static uint32_t s_last_out = 0;

static rrdu_hal_meas_cb_t s_cb = 0;
static void* s_cb_user = 0;
static int s_meas_inflight = 0;

void rrdu_hal_init(void)
{
    s_stat = 0;
    s_fault = 0;
    s_last_meas_id = 0;
    s_last_raw = 0;
    s_last_out = 0;
    s_cb = 0;
    s_cb_user = 0;
    s_meas_inflight = 0;
}

rrdu_hal_status_t rrdu_hal_configure_defaults(void)
{
    /* Fake: accept */
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_stat(uint32_t* stat)
{
    if (!stat) return RRDU_HAL_ERR;
    *stat = s_stat;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_fault(uint32_t* fault)
{
    if (!fault) return RRDU_HAL_ERR;
    *fault = s_fault;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_fault_clear(uint32_t mask)
{
    /* W1C style */
    s_fault &= ~mask;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_meas_start_async(uint32_t meas_id,
                                            rrdu_hal_meas_cb_t cb,
                                            void* user)
{
    if (s_meas_inflight) return RRDU_HAL_BUSY;

    s_meas_inflight = 1;
    s_last_meas_id = meas_id;
    s_cb = cb;
    s_cb_user = user;

#if RRDU_FAKE_AUTO_COMPLETE
    /* Simulation path: immediately complete with either success or fault. */
    rrdu_hal_fake_complete_meas(
        meas_id,
        0x12345678u,
        RRDU_INJECT_FAULT ? 0x1u : 0u
    );
#endif

    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_meas_raw(uint32_t meas_id, uint32_t* raw)
{
    if (!raw) return RRDU_HAL_ERR;
    if (!s_meas_inflight && meas_id == s_last_meas_id) {
        *raw = s_last_raw;
        return RRDU_HAL_OK;
    }

    if (meas_id != s_last_meas_id) return RRDU_HAL_ERR;

    *raw = s_last_raw;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_write_output_u32(uint32_t out)
{
    s_last_out = out;
    return RRDU_HAL_OK;
}

/* Host sim: think of this as "IRQ done". */
void rrdu_hal_fake_complete_meas(uint32_t meas_id, uint32_t raw, uint32_t fault_mask)
{
    if (!s_meas_inflight) return;
    if (meas_id != s_last_meas_id) return;

    s_last_raw = raw;
    s_fault |= fault_mask;

    s_meas_inflight = 0;

    if (s_cb) {
        rrdu_hal_meas_cb_t cb = s_cb;
        void* user = s_cb_user;
        s_cb = 0;
        s_cb_user = 0;

        cb(user, (fault_mask != 0) ? RRDU_HAL_ERR : RRDU_HAL_OK);
    }
}