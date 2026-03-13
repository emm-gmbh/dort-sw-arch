#include "core/rrdu_validation_demo.h"

#include "core/event_queue.h"
#include "core/units.h"
#include "services/rrdu_service/rrdu_service.h"

volatile int32_t g_rrdu_validation_result = 0;
volatile int32_t g_rrdu_validation_reason = 0;

void rrdu_validation_run_once(void)
{
    event_t ev;
    int safety = 64;

    g_rrdu_validation_result = 0;
    g_rrdu_validation_reason = 0;

    eventq_init();
    configASSERT(eventq_reset() == true);
    rrdu_service_init();
    rrdu_service_bringup_start();

    while (safety-- > 0) {
        if (!eventq_pop(&ev)) {
            continue;
        }

        if (ev.type == EVT_RRDU_MEAS_DONE) {
            rrdu_service_on_meas_done();
            continue;
        }

        if (ev.type == EVT_RRDU_MEAS_FAILED) {
            rrdu_service_on_meas_failed(ev.data.rrdu.st);
            continue;
        }

        if (ev.type == EVT_UNIT_READY && ev.data.unit == UNIT_RRDU) {
            g_rrdu_validation_result = 1;
            g_rrdu_validation_reason = 0;
            return;
        }

        if (ev.type == EVT_UNIT_FAILED && ev.data.fail.unit == UNIT_RRDU) {
            g_rrdu_validation_result = -1;
            g_rrdu_validation_reason = ev.data.fail.reason;
            return;
        }
    }

    g_rrdu_validation_result = -2;
    g_rrdu_validation_reason = -999;
}

