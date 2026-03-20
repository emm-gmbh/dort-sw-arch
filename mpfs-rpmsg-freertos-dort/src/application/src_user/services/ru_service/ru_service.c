#include <services/ru_service/ru_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    RU_SVC_RESET = 0,
    RU_SVC_BRINGUP,
    RU_SVC_READY
} ru_state_t;

static ru_state_t s_state = RU_SVC_RESET;

void ru_service_init(void)
{
    s_state = RU_SVC_RESET;
}

void ru_service_bringup_start(void)
{
    s_state = RU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    ru_service_simulate_ready();
}

void ru_service_simulate_ready(void)
{
    s_state = RU_SVC_READY;
    post_unit_ready(UNIT_RU);
}

int ru_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != RU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
