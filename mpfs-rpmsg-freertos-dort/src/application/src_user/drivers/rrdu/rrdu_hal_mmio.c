#include "rrdu_hal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/mss/mss_gpio/mss_gpio.h"

#ifndef RRDU_MMIO_COUNT_BASE_ADDR
#define RRDU_MMIO_COUNT_BASE_ADDR 0x65000000u
#endif

#ifndef RRDU_MMIO_READY_TIMEOUT_LOOPS
#define RRDU_MMIO_READY_TIMEOUT_LOOPS 12000000u
#endif

#ifndef RRDU_MMIO_DONE_TIMEOUT_LOOPS
#define RRDU_MMIO_DONE_TIMEOUT_LOOPS 30000000u
#endif

#ifndef RRDU_MMIO_ENABLE_PULSE_LOOPS
#define RRDU_MMIO_ENABLE_PULSE_LOOPS 12000u
#endif

#define RRDU_MMIO_RESET_PIN MSS_GPIO_1
#define RRDU_MMIO_START_PIN MSS_GPIO_0
#define RRDU_MMIO_FINE_DONE_MASK MSS_GPIO_2_MASK
#define RRDU_MMIO_READY_MASK MSS_GPIO_3_MASK
#define RRDU_MMIO_COARSE_DONE_MASK MSS_GPIO_4_MASK

/* Register order taken from the provided RRDU implementation. */
#define RRDU_MMIO_MEAS_COARSE_OFF 0x0u
#define RRDU_MMIO_REF_COARSE_OFF  0x4u
#define RRDU_MMIO_MEAS_FINE_OFF   0x8u
#define RRDU_MMIO_REF_FINE_OFF    0xCu

static volatile uint32_t *const s_mmio =
    (volatile uint32_t *)(uintptr_t)RRDU_MMIO_COUNT_BASE_ADDR;

static uint32_t s_stat = 0u;
static uint32_t s_fault = 0u;
static uint32_t s_last_meas_id = 0u;
static uint32_t s_last_raw = 0u;
static uint32_t s_last_out = 0u;
static rrdu_hal_meas_cb_t s_cb = NULL;
static void *s_cb_user = NULL;
static int s_meas_inflight = 0;

static uint32_t rrdu_mmio_gpio_inputs(void)
{
    return MSS_GPIO_get_inputs(GPIO2_LO);
}

static void rrdu_mmio_delay(volatile uint32_t loops)
{
    while (loops-- > 0u) {
    }
}

static bool rrdu_mmio_wait_mask(uint32_t mask, bool high, uint32_t timeout_loops)
{
    while (timeout_loops-- > 0u) {
        uint32_t inputs = rrdu_mmio_gpio_inputs();
        bool level = (inputs & mask) != 0u;
        if (level == high) {
            return true;
        }
    }
    return false;
}

static uint32_t rrdu_mmio_read_reg(uint32_t offset)
{
    return s_mmio[offset / sizeof(uint32_t)];
}

static void rrdu_mmio_update_stat(void)
{
    uint32_t inputs = rrdu_mmio_gpio_inputs();
    s_stat = 0u;
    if ((inputs & RRDU_MMIO_READY_MASK) != 0u) {
        s_stat |= (1u << 0);
    }
    if ((inputs & RRDU_MMIO_FINE_DONE_MASK) != 0u) {
        s_stat |= (1u << 1);
    }
    if ((inputs & RRDU_MMIO_COARSE_DONE_MASK) != 0u) {
        s_stat |= (1u << 2);
    }
}

static rrdu_hal_status_t rrdu_mmio_run_measurement(void)
{
    /* Reset state machines in FPGA and wait for ready/idle. */
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_RESET_PIN, 1u);
    if (!rrdu_mmio_wait_mask(RRDU_MMIO_READY_MASK, true, RRDU_MMIO_READY_TIMEOUT_LOOPS)) {
        s_fault |= 0x10u;
        MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_RESET_PIN, 0u);
        return RRDU_HAL_ERR;
    }
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_RESET_PIN, 0u);

    /* Start measurement with a short pulse. */
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_START_PIN, 1u);
    rrdu_mmio_delay(RRDU_MMIO_ENABLE_PULSE_LOOPS);
    MSS_GPIO_set_output(GPIO2_LO, RRDU_MMIO_START_PIN, 0u);

    /* Wait until fine and coarse done lines are asserted. */
    if (!rrdu_mmio_wait_mask(RRDU_MMIO_FINE_DONE_MASK, true, RRDU_MMIO_DONE_TIMEOUT_LOOPS)) {
        s_fault |= 0x01u;
        return RRDU_HAL_ERR;
    }
    if (!rrdu_mmio_wait_mask(RRDU_MMIO_COARSE_DONE_MASK, true, RRDU_MMIO_DONE_TIMEOUT_LOOPS)) {
        s_fault |= 0x02u;
        return RRDU_HAL_ERR;
    }

    s_last_raw = rrdu_mmio_read_reg(RRDU_MMIO_MEAS_FINE_OFF);
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
