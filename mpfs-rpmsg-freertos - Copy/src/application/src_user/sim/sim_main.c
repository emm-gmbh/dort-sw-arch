#include <stdio.h>
#include <stdbool.h>

#include "core/event_queue.h"
#include "core/units.h"
#include "bringup/bringup_mgr.h"

#include "services/reftime_service/reftime_service.h"
#include "services/rrdu_service/rrdu_service.h"

static const char* unit_name(unit_id_t u)
{
    switch (u) {
    case UNIT_REFTIME: return "REFTIME";
    case UNIT_RRDU:    return "RRDU";
    case UNIT_OLU:     return "OLU";
    case UNIT_CCU:     return "CCU";
    case UNIT_HFDU:    return "HFDU";
    case UNIT_RU:      return "RU";
    case UNIT_ALU:     return "ALU";
    case UNIT_TCU:     return "TCU";
    case UNIT_PATU:    return "PATU";
    case UNIT_OCU:     return "OCU";
    case UNIT_HCU:     return "HCU";
    default:           return "?";
    }
}

int main(void)
{
    eventq_init();
    bringup_ctx_t bu;
    bringup_init(&bu);


    reftime_service_init();
    rrdu_service_init();

    // Bring-up start event
    post_bringup_start();

    // SIM: donanım yokken "hazır" eventleri üret
    // Not: out-of-order olursa bringup_mgr ignore edeceği için sorun değil.
    reftime_service_simulate_valid();
    rrdu_service_simulate_ready();

    // Event loop: sonsuz değil, sınırlı tur (host sim için)
    for (int i = 0; i < 100; i++) {
        event_t ev;
        if (!eventq_pop(&ev)) {
            // Queue boşsa simde çıkabiliriz
            break;
        }

        printf("[SIM] ev.type=%d", (int)ev.type);
        if (ev.type == EVT_UNIT_READY) {
            printf(" unit=%s", unit_name(ev.data.unit));
        }
        printf("\n");

        bringup_handle_event(&bu, &ev);

        // Bu noktada bringup_ctx içinde idx/state var:
        // idx = beklenen unit index'i
        unit_id_t expected = (unit_id_t)bu.idx;
        printf("[SIM] bringup state=%d idx=%u expected=%s\n",
               (int)bu.state, (unsigned)bu.idx,
               (bu.idx < (unsigned)UNIT_COUNT) ? unit_name(expected) : "DONE");
    }

    return 0;
}


