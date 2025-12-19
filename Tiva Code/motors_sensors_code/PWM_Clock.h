/**
 * @file PWM_Clock.h
 *
 * @brief Header file used to configure the PWM clock source.
 *
 * This file contains the function definitions for configuring the PWM clock.
 * It updates the RCC register from SYSCTL to update the PWM clock and use
 * the PWM clock divider as the source.
 *
 * When the PWM divisor is used, it is applied to the clock for both PWM modules.
 *
 * @note This driver assumes that the system clock's frequency is 50 MHz.
 *
 * @author Aaron Nanas
 */

#include "TM4C123GH6PM.h"

/**
 * @brief Initializes the PWM clock source.
 *
 * This function configures the PWM modules to use a divided PWM clock. 
 * It enables the PWM clock divisor using the RCC register and sets 
 * the divisor to divide the PWM clock frequency by 16.
 *
 * @param None
 *
 * @return None
 */
void PWM_Clock_Init(void);