#include "reftime_service.h"

#include "core/event_queue.h"
#include "core/units.h"

typedef enum {
    REFTIME_RESET = 0,
    REFTIME_WAIT_VALID,
    REFTIME_VALID
} reftime_state_t;

static reftime_state_t s_state = REFTIME_RESET;

void reftime_service_init(void)
{
    s_state = REFTIME_RESET;
}

void reftime_service_bringup_start(void)
{
    // Gerçekte burada: “ref time valid mi?” check/subscribe mekanizması olur.
    // Şimdilik sadece “valid bekleme” state’ine geçiyoruz.
    s_state = REFTIME_WAIT_VALID;

    // İstersen SIM modda otomatik valid yapabilirsin:
    reftime_service_simulate_valid();
}

void reftime_service_simulate_valid(void)
{
    s_state = REFTIME_VALID;
    post_unit_ready(UNIT_REFTIME);
}
