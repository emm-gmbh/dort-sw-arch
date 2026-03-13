#include "bringup_mgr.h"
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

/* unit start function (stub for now) */
static void unit_start(unit_id_t u)
{
    switch (u) {
    case UNIT_REFTIME: reftime_service_bringup_start(); break;
    case UNIT_RRDU: rrdu_service_bringup_start(); break;
    case UNIT_OLU:  olu_service_bringup_start();  break;
    case UNIT_CCU:  ccu_service_bringup_start();  break;
    case UNIT_HFDU: hfdu_service_bringup_start(); break;
    case UNIT_RU:   ru_service_bringup_start();   break;
    case UNIT_ALU:  alu_service_bringup_start();  break;
    case UNIT_TCU:  tcu_service_bringup_start();  break;
    case UNIT_PATU: patu_service_bringup_start(); break;
    case UNIT_OCU:  ocu_service_bringup_start();  break;
    case UNIT_HCU:  hcu_service_bringup_start();  break;
    default:
        break;
    }
}

/* Start event triggered */
static void bringup_on_start(bringup_ctx_t* ctx)
{
    if (ctx->state != BU_IDLE) {
        return;
    }

    ctx->state = BU_RUNNING;
    ctx->idx = 0;
    unit_start((unit_id_t)ctx->idx);
}

/* When unit u is ready, go to next */
static void bringup_on_unit_ready(bringup_ctx_t* ctx, unit_id_t u)
{
    if (ctx->state != BU_RUNNING) return;

    unit_id_t expected = (unit_id_t)ctx->idx;
    if (u != expected) return; // out of order, ignore

    ctx->idx++;

    if (ctx->idx >= (uint8_t)UNIT_COUNT) {
        ctx->state = BU_DONE;
        post_bringup_done();
        return;
    }

    unit_start((unit_id_t)ctx->idx);
}

/* When a unit fails */
static void bringup_on_unit_failed(bringup_ctx_t* ctx, unit_id_t u, int32_t reason)
{
    (void)u;
    (void)reason;

    if (ctx->state != BU_RUNNING) {
        return;
    }

    ctx->state = BU_FAILED;
    post_bringup_failed();
}

/* Public init */
void bringup_init(bringup_ctx_t* ctx)
{
    if (!ctx) return;
    ctx->state = BU_IDLE;
    ctx->idx = 0;
}

/* Public event handler */
void bringup_handle_event(bringup_ctx_t* ctx, const event_t* ev)
{
    if (!ctx || !ev) return;

    switch (ev->type) {
    case EVT_BRINGUP_START:
        bringup_on_start(ctx);
        break;
    case EVT_UNIT_READY:
        bringup_on_unit_ready(ctx, ev->data.unit);
        break;
    case EVT_UNIT_FAILED:
        bringup_on_unit_failed(ctx, ev->data.fail.unit, ev->data.fail.reason);
        break;
    case EVT_RRDU_MEAS_DONE:
        rrdu_service_on_meas_done();
        break;
    case EVT_RRDU_MEAS_FAILED:
        rrdu_service_on_meas_failed(ev->data.rrdu.st);
        break;
    default:
        break;
    }
}
