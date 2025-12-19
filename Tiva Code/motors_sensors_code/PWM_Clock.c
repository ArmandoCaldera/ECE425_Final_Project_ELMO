/**
 * @file PWM_Clock.c
 *
 * @brief Source file used to configure the PWM clock source.
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
 
#include "PWM_Clock.h"
 
void PWM_Clock_Init(void)
{
	// Enable the PWM clock divisor by setting the 
	// USEPWMDIV bit (Bit 20) in the RCC register
	SYSCTL->RCC |= 0x00100000;
	
	// Clear the PWMDIV field (Bits 19 to 17) in the RCC register
	SYSCTL->RCC &= ~0x000E0000;
	
	// Divide the PWM clock frequency by 16 by writing 0x3
	// to the PWMDIV field (Bits 19 to 17)
	// Note: Only the upper 3 bits are read/write. 0110 (0x6) -> 011 (0x3)
	// Refer to page 255 of the TM4C123G Microcontroller Datasheet
	SYSCTL->RCC |= 0x00060000;
}