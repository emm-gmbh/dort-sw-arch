#pragma once
#include <stdint.h>

void tcu_service_init(void);


void tcu_service_bringup_start(void);


void tcu_service_simulate_ready(void);


int tcu_service_measure_start(uint32_t meas_id);
