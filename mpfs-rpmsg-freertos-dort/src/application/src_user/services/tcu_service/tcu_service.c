#include <services/tcu_service/tcu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    TCU_SVC_RESET = 0,
    TCU_SVC_BRINGUP,
    TCU_SVC_READY
} tcu_state_t;

static tcu_state_t s_state = TCU_SVC_RESET;

void tcu_service_init(void)
{
    s_state = TCU_SVC_RESET;
}

void tcu_service_bringup_start(void)
{
    s_state = TCU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    tcu_service_simulate_ready();
}

void tcu_service_simulate_ready(void)
{
    s_state = TCU_SVC_READY;
    post_unit_ready(UNIT_TCU);
}

int tcu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != TCU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
