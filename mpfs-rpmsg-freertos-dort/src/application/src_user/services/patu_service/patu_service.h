#pragma once
#include <stdint.h>

void patu_service_init(void);


void patu_service_bringup_start(void);


void patu_service_simulate_ready(void);


int patu_service_measure_start(uint32_t meas_id);
