#pragma once
#include <stdint.h>

void alu_service_init(void);


void alu_service_bringup_start(void);


void alu_service_simulate_ready(void);


int alu_service_measure_start(uint32_t meas_id);
