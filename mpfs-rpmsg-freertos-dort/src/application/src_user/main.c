#include "bringup/bringup_mgr.h"
#include "core/event_queue.h"
#include <stdbool.h>

#include "mpfs_hal/mss_hal.h"

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

int main(void)
{
    eventq_init();
    bringup_ctx_t bu;
    bringup_init(&bu);

    /* RRDU low-level path drives GPIO2 reset/start pins on Icicle Kit. */
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO2, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);

    reftime_service_init();
    rrdu_service_init();  // callback register + irq enable
    olu_service_init();
    ccu_service_init();
    hfdu_service_init();
    ru_service_init();
    alu_service_init();
    tcu_service_init();
    patu_service_init();
    ocu_service_init();
    hcu_service_init();


    post_bringup_start(); // pushing bringup started event



    while (1) {
        event_t ev;
        if (eventq_pop(&ev)) {
            bringup_handle_event(&bu, &ev);
        }

        // burada ileride: tc, fault, tlm vs. event üretir
    }
}

