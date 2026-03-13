#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "core/units.h"
#include "core/event_queue.h"


/* Sequencer State */
typedef enum {
    BU_IDLE,
    BU_RUNNING,
    BU_DONE,
    BU_FAILED
} bu_state_t;

/* Sequencer context: memory */
typedef struct {
    bu_state_t state;
    uint8_t idx; // the unit expected to be ready from 0 to UNIT_COUNT-1
} bringup_ctx_t;


/* API */
void bringup_init(bringup_ctx_t* ctx);
void bringup_handle_event(bringup_ctx_t* ctx, const event_t* ev);

