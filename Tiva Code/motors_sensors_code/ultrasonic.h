#ifndef ULTRASONIC_H
#define ULTRASONIC_H
#include "ultrasonic.h"

#include <stdint.h>

void ultrasonic_Init(void);
uint32_t ultrasonic_sensor_pulse_read(void);
uint32_t ultrasonic_sensor_distance_read(void);
#endif