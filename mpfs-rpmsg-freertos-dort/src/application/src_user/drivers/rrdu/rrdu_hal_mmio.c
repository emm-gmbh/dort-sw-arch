#include "rrdu_hal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/registers.h"
#include "drivers/mss/mss_gpio/mss_gpio.h"

#ifndef RRDU_MMIO_READY_TIMEOUT_LOOPS
#define RRDU_MMIO_READY_TIMEOUT_LOOPS 12000000u
#endif

#ifndef RRDU_MMIO_ENABLE_PULSE_LOOPS
#define RRDU_MMIO_ENABLE_PULSE_LOOPS 12000u
#endif

#define RRDU_MMIO_RESET_PIN MSS_GPIO_1
#define RRDU_MMIO_START_PIN MSS_GPIO_0

static uint32_t s_stat = 0u;
static uint32_t s_fault = 0u;
static uint32_t s_last_meas_id = 0u;
static uint32_t s_last_raw = 0u;
static uint32_t s_last_out = 0u;
static rrdu_hal_meas_cb_t s_cb = NULL;
static void *s_cb_user = NULL;
static int s_meas_inflight = 0;

static bool rrdu_mmio_is_ready(void)
{
    return (RRDU_READY_REG & RRDU_STATUS_READY_MASK) != 0u;
}

static void rrdu_mmio_delay(volatile uint32_t loops)
{
    while (loops-- > 0u) {
    }
}

static bool rrdu_mmio_wait_ready(bool ready, uint32_t timeout_loops)
{
    while (timeout_loops-- > 0u) {
        if (rrdu_mmio_is_ready() == ready) {
            return true;
        }
    }
    return false;
}

static void rrdu_mmio_update_stat(void)
{
    s_stat = RRDU_READY_REG;
}

static rrdu_hal_status_t rrdu_mmio_run_measurement(void)
{
    s_fault = 0u;

    /* Reset state machines in FPGA before a new bring-up measurement. */
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_RESET_PIN, 1u);
    rrdu_mmio_delay(RRDU_MMIO_ENABLE_PULSE_LOOPS);
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_RESET_PIN, 0u);

    /* Start measurement with a short pulse. */
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_START_PIN, 1u);
    rrdu_mmio_delay(RRDU_MMIO_ENABLE_PULSE_LOOPS);
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_START_PIN, 0u);

    /*
     * Ready goes low while the subsystem is measuring and returns high when a
     * new measurement has completed. Waiting for the low->high transition
     * avoids accepting a stale ready level from a previous cycle.
     */
    if (!rrdu_mmio_wait_ready(false, RRDU_MMIO_READY_TIMEOUT_LOOPS)) {
        s_fault |= 0x01u;
        return RRDU_HAL_ERR;
    }
    if (!rrdu_mmio_wait_ready(true, RRDU_MMIO_READY_TIMEOUT_LOOPS)) {
        s_fault |= 0x02u;
        return RRDU_HAL_ERR;
    }

    s_last_raw = RRDU_COUNT_REGS->meas_fine;
    rrdu_mmio_update_stat();
    return RRDU_HAL_OK;
}

void rrdu_hal_init(void)
{
    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config_all(GPIO2_LO, MSS_GPIO_INOUT_MODE);

    s_stat = 0u;
    s_fault = 0u;
    s_last_meas_id = 0u;
    s_last_raw = 0u;
    s_last_out = 0u;
    s_cb = NULL;
    s_cb_user = NULL;
    s_meas_inflight = 0;
}

rrdu_hal_status_t rrdu_hal_configure_defaults(void)
{
    rrdu_mmio_update_stat();
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_stat(uint32_t *stat)
{
    if (stat == NULL) {
        return RRDU_HAL_ERR;
    }
    rrdu_mmio_update_stat();
    *stat = s_stat;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_fault(uint32_t *fault)
{
    if (fault == NULL) {
        return RRDU_HAL_ERR;
    }
    *fault = s_fault;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_fault_clear(uint32_t mask)
{
    s_fault &= ~mask;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_meas_start_async(uint32_t meas_id,
                                            rrdu_hal_meas_cb_t cb,
                                            void *user)
{
    rrdu_hal_status_t run_status;
    rrdu_hal_meas_cb_t local_cb;
    void *local_user;

    if (cb == NULL) {
        return RRDU_HAL_ERR;
    }
    if (s_meas_inflight) {
        return RRDU_HAL_BUSY;
    }

    s_meas_inflight = 1;
    s_last_meas_id = meas_id;
    s_cb = cb;
    s_cb_user = user;

    run_status = rrdu_mmio_run_measurement();

    local_cb = s_cb;
    local_user = s_cb_user;
    s_cb = NULL;
    s_cb_user = NULL;
    s_meas_inflight = 0;

    if (local_cb != NULL) {
        local_cb(local_user, run_status);
    }

    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_read_meas_raw(uint32_t meas_id, uint32_t *raw)
{
    if ((raw == NULL) || (meas_id != s_last_meas_id) || (s_meas_inflight != 0)) {
        return RRDU_HAL_ERR;
    }

    *raw = s_last_raw;
    return RRDU_HAL_OK;
}

rrdu_hal_status_t rrdu_hal_write_output_u32(uint32_t out)
{
    s_last_out = out;
    (void)s_last_out;
    return RRDU_HAL_OK;
}

/* Keep symbol available for builds that still compile rrdu_service_simulate_ready(). */
void rrdu_hal_fake_complete_meas(uint32_t meas_id, uint32_t raw, uint32_t fault_mask)
{
    (void)meas_id;
    (void)raw;
    (void)fault_mask;
}
