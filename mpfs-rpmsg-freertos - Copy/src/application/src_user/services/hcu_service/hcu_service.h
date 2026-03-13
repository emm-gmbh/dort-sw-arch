#pragma once
#include <stdint.h>

void hcu_service_init(void);


void hcu_service_bringup_start(void);


void hcu_service_simulate_ready(void);


int hcu_service_measure_start(uint32_t meas_id);
