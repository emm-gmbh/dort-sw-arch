#include <services/alu_service/alu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    ALU_SVC_RESET = 0,
    ALU_SVC_BRINGUP,
    ALU_SVC_READY
} alu_state_t;

static alu_state_t s_state = ALU_SVC_RESET;

void alu_service_init(void)
{
    s_state = ALU_SVC_RESET;
}

void alu_service_bringup_start(void)
{
    s_state = ALU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
}

void alu_service_simulate_ready(void)
{
    s_state = ALU_SVC_READY;
    post_unit_ready(UNIT_ALU);
}

int alu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != ALU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
