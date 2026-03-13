#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "units.h"
#include "FreeRTOS.h"
#include "queue.h"

/* Event types */
typedef enum {
    EVT_BRINGUP_START,     // starts the bringup sequence
    EVT_UNIT_READY,        // a unit finished its bringup step succesfilly
    EVT_UNIT_FAILED,       // a unit reported failure
    EVT_BRINGUP_DONE,      // global bringup sequence is finished
    EVT_BRINGUP_FAILED,    // global bringup sequence failed

    EVT_RRDU_MEAS_DONE,    // RRDU measurement completion events
    EVT_RRDU_MEAS_FAILED

} event_type_t;

/* Event object, type tells which event, data carries the payload relative to that event type
 * For exeample, for EVT_UNIT_READY: data .unit tells which unit
 *              for EVT_UNIT_FAILED: data.fail.unit and data.fail.reason, for RRDU measurement failure: data.rrdu.st could carry status/error
 *
 *  */
typedef struct {
    event_type_t type;
    union {
        unit_id_t unit;   // EVT_UNIT_READY
        struct {
            unit_id_t unit;
            int32_t reason; /* generic failure reason */
        } fail; /* for EVT_UNIT_FAILED */
        struct {
            int32_t st;
        } rrdu;
    } data;
} event_t;

/* Queue core API */
bool eventq_wait(event_t *out);
bool eventq_push_from_isr(const event_t *e, BaseType_t *hpw);

void eventq_init(void);
bool eventq_reset(void);
bool eventq_push(const event_t* e);
bool eventq_pop(event_t* out);

/* Convenience post_* API */
void post_bringup_start(void);
void post_unit_ready(unit_id_t u);
void post_unit_failed(unit_id_t u, int32_t reason);
void post_bringup_done(void);
void post_bringup_failed(void);

