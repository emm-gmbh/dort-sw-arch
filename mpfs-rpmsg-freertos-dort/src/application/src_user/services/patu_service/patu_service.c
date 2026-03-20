#include <services/patu_service/patu_service.h>
#include "core/event_queue.h"
#include "core/units.h"



typedef enum {
    PATU_SVC_RESET = 0,
    PATU_SVC_BRINGUP,
    PATU_SVC_READY
} patu_state_t;

static patu_state_t s_state = PATU_SVC_RESET;

void patu_service_init(void)
{
    s_state = PATU_SVC_RESET;
}

void patu_service_bringup_start(void)
{
    s_state = PATU_SVC_BRINGUP;
    /* Donanım yok: ready'yi sen tetikleyeceksin */
    patu_service_simulate_ready();
}

void patu_service_simulate_ready(void)
{
    s_state = PATU_SVC_READY;
    post_unit_ready(UNIT_PATU);
}

int patu_service_measure_start(uint32_t meas_id)
{
    (void)meas_id;
    if (s_state != PATU_SVC_READY) return -1;
    return -2; /* not implemented yet*/
}
