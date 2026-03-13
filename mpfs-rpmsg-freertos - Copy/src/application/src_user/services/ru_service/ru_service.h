#pragma once
#include <stdint.h>

void ru_service_init(void);


void ru_service_bringup_start(void);


void ru_service_simulate_ready(void);


int ru_service_measure_start(uint32_t meas_id);
