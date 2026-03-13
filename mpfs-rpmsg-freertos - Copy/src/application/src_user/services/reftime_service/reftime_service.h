#pragma once

void reftime_service_init(void);

/* Bring-up adımı: reference time valid/lock olana kadar bekler.
   Ready olduğunda post_unit_ready(UNIT_REFTIME) üretilecek. */
void reftime_service_bringup_start(void);

/* Donanım yokken zinciri ilerletmek için */
void reftime_service_simulate_valid(void);

