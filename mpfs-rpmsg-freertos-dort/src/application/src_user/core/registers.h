#pragma once
#include <stdint.h>

/* RRDU Register Map */

#define RRDU_COUNT_BASE_ADDR        0x65000000u
#define RRDU_READY_BASE_ADDR        0x20122000u

#define RRDU_MEAS_COARSE_OFFSET     0x00u
#define RRDU_REF_COARSE_OFFSET      0x04u
#define RRDU_MEAS_FINE_OFFSET       0x08u
#define RRDU_REF_FINE_OFFSET        0x0Cu

#define RRDU_STATUS_READY_MASK      0x00000008u

typedef struct
{
    volatile uint32_t meas_coarse;
    volatile uint32_t ref_coarse;
    volatile uint32_t meas_fine;
    volatile uint32_t ref_fine;
} rrdu_count_regs_t;

#define RRDU_COUNT_REGS ((rrdu_count_regs_t *)RRDU_COUNT_BASE_ADDR)
#define RRDU_READY_REG (*(volatile uint32_t *)RRDU_READY_BASE_ADDR)


/* PATU Register Map */

#define PATU_BASE_ADDR              0xC1050000u

#define PATU_CTRL_OFFSET            0x0000u
#define PATU_CFG_OFFSET             0x0004u
#define PATU_FSM_VX_OFFSET          0x0008u
#define PATU_FSM_VY_OFFSET          0x000Cu
#define PATU_STAT_OFFSET            0x0010u
#define PATU_FAULT_OFFSET           0x0014u
#define PATU_ERROR_X_OFFSET         0x0018u
#define PATU_ERROR_Y_OFFSET         0x001Cu
#define PATU_PID_P_X_OFFSET         0x0020u
#define PATU_PID_I_X_OFFSET         0x0024u
#define PATU_PID_D_X_OFFSET         0x0028u
#define PATU_PID_P_Y_OFFSET         0x002Cu
#define PATU_PID_I_Y_OFFSET         0x0030u
#define PATU_PID_D_Y_OFFSET         0x0034u
#define PATU_TEST_LASER_CTRL_OFFSET 0x0038u
#define PATU_FSM_DAC_X_OFFSET       0x003Cu
#define PATU_FSM_DAC_Y_OFFSET       0x0040u
#define PATU_ERR_ADC_X_OFFSET       0x0044u
#define PATU_ERR_ADC_Y_OFFSET       0x0048u

typedef struct
{
    volatile uint32_t ctrl;            /* PATU_CTRL: uint32 bitfield */
    volatile uint32_t cfg;             /* PATU_CFG: uint32 bitfield */
    volatile uint32_t fsm_vx;          /* PATU_FSM_VX: float32 */
    volatile uint32_t fsm_vy;          /* PATU_FSM_VY: float32 */
    volatile uint32_t stat;            /* PATU_STAT: uint32 bitfield */
    volatile uint32_t fault;           /* PATU_FAULT: uint32 bitset, W1C */
    volatile uint32_t error_x;         /* PATU_ERROR_X: float32 */
    volatile uint32_t error_y;         /* PATU_ERROR_Y: float32 */
    volatile uint32_t pid_p_x;         /* PATU_PID_P_X: float32 */
    volatile uint32_t pid_i_x;         /* PATU_PID_I_X: float32 */
    volatile uint32_t pid_d_x;         /* PATU_PID_D_X: float32 */
    volatile uint32_t pid_p_y;         /* PATU_PID_P_Y: float32 */
    volatile uint32_t pid_i_y;         /* PATU_PID_I_Y: float32 */
    volatile uint32_t pid_d_y;         /* PATU_PID_D_Y: float32 */
    volatile uint32_t test_laser_ctrl; /* PATU_TEST_LASER_CTRL: uint32 */
    volatile uint32_t fsm_dac_x;       /* PATU_FSM_DAC_X: uint32 */
    volatile uint32_t fsm_dac_y;       /* PATU_FSM_DAC_Y: uint32 */
    volatile uint32_t err_adc_x;       /* PATU_ERR_ADC_X: uint32 */
    volatile uint32_t err_adc_y;       /* PATU_ERR_ADC_Y: uint32 */
} patu_regs_t;

#define PATU_REGS ((patu_regs_t *)PATU_BASE_ADDR)
