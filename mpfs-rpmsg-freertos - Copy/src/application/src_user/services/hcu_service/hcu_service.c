#include <services/hcu_service/hcu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    HCU_SVC_RESET = 0,
    HCU_SVC_BRINGUP,
    HCU_SVC_READY
} hcu_state_t;

static hcu_state_t s_state = HCU_SVC_RESET;

void hcu_service_init(void)
{
    s_state = HCU_SVC_RESET;
}

void hcu_service_bringup_start(void)
{
    s_state = HCU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
}

void hcu_service_simulate_ready(void)
{
    s_state = HCU_SVC_READY;
    post_unit_ready(UNIT_HCU);
}

int hcu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != HCU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
