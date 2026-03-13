#pragma once
#include <stdint.h>

void ccu_service_init(void);


void ccu_service_bringup_start(void);


void ccu_service_simulate_ready(void);


int ccu_service_measure_start(uint32_t meas_id);
