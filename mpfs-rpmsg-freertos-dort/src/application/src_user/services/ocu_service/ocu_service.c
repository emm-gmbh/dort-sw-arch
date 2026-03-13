#include <services/ocu_service/ocu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    OCU_SVC_RESET = 0,
    OCU_SVC_BRINGUP,
    OCU_SVC_READY
} ocu_state_t;

static ocu_state_t s_state = OCU_SVC_RESET;

void ocu_service_init(void)
{
    s_state = OCU_SVC_RESET;
}

void ocu_service_bringup_start(void)
{
    s_state = OCU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
}

void ocu_service_simulate_ready(void)
{
    s_state = OCU_SVC_READY;
    post_unit_ready(UNIT_OCU);
}

int ocu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != OCU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
