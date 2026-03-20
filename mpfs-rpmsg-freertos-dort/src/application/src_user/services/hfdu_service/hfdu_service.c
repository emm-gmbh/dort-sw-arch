#include <services/hfdu_service/hfdu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    HFDU_SVC_RESET = 0,
    HFDU_SVC_BRINGUP,
    HFDU_SVC_READY
} hfdu_state_t;

static hfdu_state_t s_state = HFDU_SVC_RESET;

void hfdu_service_init(void)
{
    s_state = HFDU_SVC_RESET;
}

void hfdu_service_bringup_start(void)
{
    s_state = HFDU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    hfdu_service_simulate_ready();
}

void hfdu_service_simulate_ready(void)
{
    s_state = HFDU_SVC_READY;
    post_unit_ready(UNIT_HFDU);
}

int hfdu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != HFDU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
