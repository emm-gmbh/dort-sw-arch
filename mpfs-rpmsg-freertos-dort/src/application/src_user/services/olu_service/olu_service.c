#include "olu_service.h"
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    OLU_SVC_RESET = 0,
    OLU_SVC_BRINGUP,
    OLU_SVC_READY
} olu_state_t;

static olu_state_t s_state = OLU_SVC_RESET;

void olu_service_init(void)
{
    s_state = OLU_SVC_RESET;
}

void olu_service_bringup_start(void)
{
    s_state = OLU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    olu_service_simulate_ready();
}

void olu_service_simulate_ready(void)
{
    s_state = OLU_SVC_READY;
    post_unit_ready(UNIT_OLU);
}

int olu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != OLU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
