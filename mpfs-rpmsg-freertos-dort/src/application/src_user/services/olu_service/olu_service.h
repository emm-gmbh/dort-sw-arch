#pragma once
#include <stdint.h>

void olu_service_init(void);


void olu_service_bringup_start(void);


void olu_service_simulate_ready(void);


int olu_service_measure_start(uint32_t meas_id);
