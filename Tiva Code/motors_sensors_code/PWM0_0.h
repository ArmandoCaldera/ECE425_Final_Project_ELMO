/**
 * @file PWM0_0.h
 *
 * @brief Header file for the PWM0_0 driver.
 *
 * This file contains the function definitions for the PWM0_0 driver.
 * It uses the Module 0 PWM Generator 0 to generate a PWM signal with the PB6 pin.
 *
 * @note This driver assumes that the system clock's frequency is 50 MHz.
 *
 * @note This driver assumes that the PWM_Clock_Init function has been called
 * before calling the PWM0_0_Init function.
 *
 * @author Aaron Nanas
 */
 
#include "TM4C123GH6PM.h"

/**
 * @brief Initializes the PWM Module 0 Generator 0 with the specified period and duty cycle.
 *
 * This function initializes the PWM Module 0 Generator 0 with the given period constant and duty cycle.
 * It configures the PB6 pin to operate as a Module 0 PWM0 pin (M0PWM0) to output the PWM signal.
 * period_constant determines the PWM signal's frequency. The specified duty_cycle value must be less 
 * than the period_constant.
 *
 * @param period_constant The period constant for the PWM signal that determines the
 *                        PWM signal's frequency.
 *
 * @param duty_cycle The duty cycle, as a percentage of period_constant, for the PWM signal.
 *                   This value controls pulse width of the PWM signal.
 *
 * @return None
 */
void PWM0_0_Init(uint16_t period_constant, uint16_t duty_cycle);

/**
 * @brief Updates the PWM Module 0 Generator 0 duty cycle for the PWM signal on the PB6 pin (M0PWM0).
 *
 * @param duty_cycle The new duty cycle for the PWM signal on the PB6 pin (M0PWM0).
 *
 * @return None
 */
void PWM0_0_Update_Duty_Cycle(uint16_t duty_cycle);

 /**
 * @brief Drive both motors forward.
 */
void PWM0_0_Forward(void);

/**
 * @brief Drive both motors in reverse.
 */
void PWM0_0_Reverse(void);

/**
 * @brief Stop both motors (PWM off, all direction pins low).
 */
void PWM0_0_Stop(void);

void PWM0_0_Turn_Left(void);
