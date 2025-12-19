#include "TM4C123GH6PM.h"
#include "PWM0_0.h"
#include <stdint.h>

void PWM0_0_Init(uint16_t period_constant, uint16_t duty_cycle)
{
  if (duty_cycle >= period_constant) return;

  // Enable PWM0 and GPIO ports B, C, E
  SYSCTL->RCGCPWM  |= 0x01;
  SYSCTL->RCGCGPIO |= 0x02;
  SYSCTL->RCGCGPIO |= 0x04;
  SYSCTL->RCGCGPIO |= 0x10;

  while ((SYSCTL->PRGPIO & 0x02) == 0) {}
  while ((SYSCTL->PRGPIO & 0x04) == 0) {}
  while ((SYSCTL->PRGPIO & 0x10) == 0) {}

  // PB4 (M0PWM2) and PB5 (M0PWM3)
  GPIOB->AFSEL |= 0x30;
  GPIOB->DEN   |= 0x30;
  GPIOB->DIR   |= 0x30;
  GPIOB->PCTL &= ~0x00FF0000;
  GPIOB->PCTL |=  0x00440000;

  // RIGHT motor direction pins (PC6, PC7)
  GPIOC->AFSEL &= ~0xC0;
  GPIOC->DEN   |=  0xC0;
  GPIOC->DIR   |=  0xC0;
  GPIOC->DATA  &= ~0xC0;

  // LEFT motor direction pins (PE2, PE3)
  GPIOE->AFSEL &= ~0x0C;
  GPIOE->DEN   |=  0x0C;
  GPIOE->DIR   |=  0x0C;
  GPIOE->DATA  &= ~0x0C;

    // PWM0 Generator 1
  PWM0->_1_CTL &= ~0x01;
  PWM0->_1_CTL &= ~0x02;

  PWM0->_1_GENA = 0x0000008C;
  PWM0->_1_GENB = 0x0000080C;

  PWM0->_1_LOAD = period_constant - 1;
  PWM0->_1_CMPA = duty_cycle - 1;
  PWM0->_1_CMPB = duty_cycle - 1;

  PWM0->_1_CTL |= 0x01;

  PWM0->ENABLE |= 0x0C;
}

void PWM0_0_Update_Duty_Cycle(uint16_t duty_cycle)
{
  PWM0->_1_CMPA = duty_cycle - 1;
  PWM0->_1_CMPB = duty_cycle - 1;
}

void PWM0_0_Forward(void)
{
  PWM0->ENABLE |= 0x0C;

  // Right motor forward
  GPIOC->DATA |=  0x40;
  GPIOC->DATA &= ~0x80;

  // Left motor forward
  GPIOE->DATA |=  0x04;
  GPIOE->DATA &= ~0x08;
}

void PWM0_0_Reverse(void)
{
	PWM0->ENABLE |= 0x0C;

  // Right motor reverse
  GPIOC->DATA &= ~0x40;
  GPIOC->DATA |=  0x80;

  // Left motor reverse
  GPIOE->DATA &= ~0x04;
  GPIOE->DATA |=  0x08;
}

void PWM0_0_Stop(void)
{
  PWM0->ENABLE &= ~0x0C;

	GPIOC->DATA &= ~0xC0;
  GPIOE->DATA &= ~0x0C;
}

// NEW TURN LEFT FUNCTION
void PWM0_0_Turn_Left(void)
{
	PWM0->ENABLE |= 0x0C;
	
	// Right motor forward
  GPIOC->DATA |=  0x40;
  GPIOC->DATA &= ~0x80;
	
	// Left motor reverse
  GPIOE->DATA &= ~0x04;
  GPIOE->DATA |=  0x08;
	
	
}
