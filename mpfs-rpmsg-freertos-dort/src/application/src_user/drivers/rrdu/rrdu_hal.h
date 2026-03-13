#pragma once
#include <stdint.h>

typedef enum {
    RRDU_HAL_OK = 0,
    RRDU_HAL_ERR = -1,
    RRDU_HAL_BUSY = -2
} rrdu_hal_status_t;

typedef void (*rrdu_hal_meas_cb_t)(void* user, rrdu_hal_status_t st);

void rrdu_hal_init(void);

/* Bring-up/config */
rrdu_hal_status_t rrdu_hal_configure_defaults(void);
rrdu_hal_status_t rrdu_hal_read_stat(uint32_t* stat);
rrdu_hal_status_t rrdu_hal_read_fault(uint32_t* fault);
rrdu_hal_status_t rrdu_hal_fault_clear(uint32_t mask);

/* Async measurement */
rrdu_hal_status_t rrdu_hal_meas_start_async(uint32_t meas_id,
                                            rrdu_hal_meas_cb_t cb,
                                            void* user);
rrdu_hal_status_t rrdu_hal_read_meas_raw(uint32_t meas_id, uint32_t* raw);

/* Publish output (to FPGA reg later; sim’de sadece saklanır) */
rrdu_hal_status_t rrdu_hal_write_output_u32(uint32_t out);

/* ---- Host-sim helpers (fake HAL implements; mmio HAL can ignore) ---- */
void rrdu_hal_fake_complete_meas(uint32_t meas_id, uint32_t raw, uint32_t fault_mask);
