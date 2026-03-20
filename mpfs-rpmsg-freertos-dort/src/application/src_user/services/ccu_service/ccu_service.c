#include <services/ccu_service/ccu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    CCU_SVC_RESET = 0,
    CCU_SVC_BRINGUP,
    CCU_SVC_READY
} ccu_state_t;

static ccu_state_t s_state = CCU_SVC_RESET;

void ccu_service_init(void)
{
    s_state = CCU_SVC_RESET;
}

void ccu_service_bringup_start(void)
{
    s_state = CCU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    ccu_service_simulate_ready();
}

void ccu_service_simulate_ready(void)
{
    s_state = CCU_SVC_READY;
    post_unit_ready(UNIT_CCU);
}

int ccu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != CCU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
