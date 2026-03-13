#include "core/bringup_validation_demo.h"

#include "bringup/bringup_mgr.h"
#include "core/event_queue.h"
#include "core/units.h"

#include "services/reftime_service/reftime_service.h"
#include "services/rrdu_service/rrdu_service.h"
#include "services/olu_service/olu_service.h"
#include "services/ccu_service/ccu_service.h"
#include "services/hfdu_service/hfdu_service.h"
#include "services/ru_service/ru_service.h"
#include "services/alu_service/alu_service.h"
#include "services/tcu_service/tcu_service.h"
#include "services/patu_service/patu_service.h"
#include "services/ocu_service/ocu_service.h"
#include "services/hcu_service/hcu_service.h"

volatile int32_t g_bringup_validation_result = 0;
volatile int32_t g_bringup_validation_state = 0;
volatile uint32_t g_bringup_trace_len = 0;
volatile uint32_t g_bringup_trace[64] = {0};

static void trace(uint32_t v)
{
    if (g_bringup_trace_len < 64u) {
        g_bringup_trace[g_bringup_trace_len++] = v;
    }
}

static void simulate_current_unit(uint8_t idx)
{
    switch ((unit_id_t)idx) {
    case UNIT_REFTIME: reftime_service_simulate_valid(); break;
    case UNIT_RRDU:    /* auto-complete via fake HAL */ break;
    case UNIT_OLU:     olu_service_simulate_ready(); break;
    case UNIT_CCU:     ccu_service_simulate_ready(); break;
    case UNIT_HFDU:    hfdu_service_simulate_ready(); break;
    case UNIT_RU:      ru_service_simulate_ready(); break;
    case UNIT_ALU:     alu_service_simulate_ready(); break;
    case UNIT_TCU:     tcu_service_simulate_ready(); break;
    case UNIT_PATU:    patu_service_simulate_ready(); break;
    case UNIT_OCU:     ocu_service_simulate_ready(); break;
    case UNIT_HCU:     hcu_service_simulate_ready(); break;
    default: break;
    }
}

void bringup_validation_run_once(void)
{
    bringup_ctx_t bu;
    event_t ev;
    int safety = 512;

    g_bringup_validation_result = 0;
    g_bringup_validation_state = 0;
    g_bringup_trace_len = 0;

    eventq_init();
    configASSERT(eventq_reset() == true);
    bringup_init(&bu);

    reftime_service_init();
    rrdu_service_init();
    olu_service_init();
    ccu_service_init();
    hfdu_service_init();
    ru_service_init();
    alu_service_init();
    tcu_service_init();
    patu_service_init();
    ocu_service_init();
    hcu_service_init();

    post_bringup_start();

    while (safety-- > 0) {
        if (!eventq_pop(&ev)) {
            continue;
        }

        switch (ev.type) {
        case EVT_BRINGUP_START: trace(0x100u); break;
        case EVT_UNIT_READY: trace(0x200u + (uint32_t)ev.data.unit); break;
        case EVT_UNIT_FAILED: trace(0x300u + (uint32_t)ev.data.fail.unit); break;
        case EVT_RRDU_MEAS_DONE: trace(0x400u); break;
        case EVT_RRDU_MEAS_FAILED: trace(0x401u); break;
        case EVT_BRINGUP_DONE: trace(0x500u); break;
        case EVT_BRINGUP_FAILED: trace(0x501u); break;
        default: break;
        }

        bringup_handle_event(&bu, &ev);
        g_bringup_validation_state = (int32_t)bu.state;

        if (bu.state == BU_RUNNING && bu.idx < (uint8_t)UNIT_COUNT) {
            simulate_current_unit(bu.idx);
        }

        if (ev.type == EVT_BRINGUP_FAILED) {
            g_bringup_validation_result = -1;
            return;
        }

        if (ev.type == EVT_BRINGUP_DONE) {
            g_bringup_validation_result = (bu.state == BU_DONE) ? 1 : -1;
            return;
        }
    }

    g_bringup_validation_result = -2;
}

