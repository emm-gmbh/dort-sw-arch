#pragma once
#include <stdint.h>

void hfdu_service_init(void);


void hfdu_service_bringup_start(void);


void hfdu_service_simulate_ready(void);


int hfdu_service_measure_start(uint32_t meas_id);
