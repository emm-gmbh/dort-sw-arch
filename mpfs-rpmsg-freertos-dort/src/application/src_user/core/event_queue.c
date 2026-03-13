#include "event_queue.h"

#include "FreeRTOS.h"
#include "queue.h"

#define EVENTQ_DEPTH 32

static QueueHandle_t g_eventq = NULL; // FreeRTOS queue instance handle

void eventq_init(void)
{
    /* Idempotent init to avoid queue leaks and state replacement. */
    if (g_eventq != NULL) {
        return;
    }

    g_eventq = xQueueCreate(EVENTQ_DEPTH, sizeof(event_t));
    configASSERT(g_eventq != NULL);
}

bool eventq_reset(void)
{
    if (g_eventq == NULL) return false;
    return xQueueReset(g_eventq) == pdPASS;
}

bool eventq_push(const event_t *e)
{
    /* Task context push (non-blocking) */
    if (g_eventq == NULL) return false;
    return xQueueSend(g_eventq, e, 0) == pdPASS;
}

bool eventq_pop(event_t *out)
{
    /* Task context pop (non-blocking) */
    if (g_eventq == NULL) return false;
    return xQueueReceive(g_eventq, out, 0) == pdPASS;
}

/* Optional but recommended: blocking wait used by DortMainTask */
bool eventq_wait(event_t *out)
{
    if (g_eventq == NULL) return false;
    return xQueueReceive(g_eventq, out, portMAX_DELAY) == pdPASS;
}

/* Optional but critical for later: ISR-safe push */
bool eventq_push_from_isr(const event_t *e, BaseType_t *hpw)
{
    if (g_eventq == NULL) return false;
    return xQueueSendFromISR(g_eventq, e, hpw) == pdPASS;
}

/* post_* helpers */
void post_bringup_start(void)
{
    event_t e = { .type = EVT_BRINGUP_START };
    configASSERT(eventq_push(&e) == true);
}

void post_unit_ready(unit_id_t u)
{
    event_t e = { .type = EVT_UNIT_READY };
    e.data.unit = u;
    configASSERT(eventq_push(&e) == true);
}

void post_unit_failed(unit_id_t u, int32_t reason)
{
    event_t e = { .type = EVT_UNIT_FAILED };
    e.data.fail.unit = u;
    e.data.fail.reason = reason;
    configASSERT(eventq_push(&e) == true);
}

void post_bringup_done(void)
{
    event_t e = { .type = EVT_BRINGUP_DONE };
    configASSERT(eventq_push(&e) == true);
}

void post_bringup_failed(void)
{
    event_t e = { .type = EVT_BRINGUP_FAILED };
    configASSERT(eventq_push(&e) == true);
}

