#pragma once
#include <stdint.h>

void ocu_service_init(void);


void ocu_service_bringup_start(void);


void ocu_service_simulate_ready(void);


int ocu_service_measure_start(uint32_t meas_id);
