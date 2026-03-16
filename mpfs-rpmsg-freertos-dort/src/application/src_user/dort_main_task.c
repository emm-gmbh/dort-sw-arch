#include "FreeRTOS.h"
#include "task.h"

#include "bringup/bringup_mgr.h"
#include "core/event_queue.h"



/* Temporary, just proves the task runs.
   We will replace this with  bringup + event loop next. */
void DortMainTask(void *pvParameters)
{

    (void)pvParameters;


    eventq_init();  // Initializes the event queue`s internal state, starting as empty


    bringup_ctx_t bu;       // bringup state machine context (memory of bringup)
    event_t ev;             // storage for each event received

    bringup_init(&bu);      // initializes bringup context to a known state



   post_bringup_start(); // instead of calling bringup functions directly, we post an event


/* Main dispatch loop */
   for (;;)
   {
       if (eventq_wait(&ev)) {  // blocking call, the task sleeps until an event is available
           bringup_handle_event(&bu, &ev);  // central dispatcher for system bringup and runtime events
       }
   }


}
