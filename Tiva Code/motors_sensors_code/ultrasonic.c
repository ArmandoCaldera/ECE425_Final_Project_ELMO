#include "TM4C123GH6PM.h"
#include "SysTick_Delay.h"
#include "ultrasonic.h"

// HC-SR04:
// Trigger -> PD2 (output)
// Echo    -> PE5 (input)


void ultrasonic_Init(void)
{
  // Enable Port D and Port E clocks
  SYSCTL->RCGCGPIO |= 0x08;   // Port D 
  SYSCTL->RCGCGPIO |= 0x10;   // Port E 

  while ((SYSCTL->PRGPIO & 0x08) == 0) {}
  while ((SYSCTL->PRGPIO & 0x10) == 0) {}

  // PD2 as output (Trigger) 
  GPIOD->DIR   |=  0x04;
  GPIOD->DEN   |=  0x04;
  GPIOD->AFSEL &= ~0x04;

  // PE5 as input (Echo) with pulldown
  GPIOE->DIR   &= ~0x20;
  GPIOE->DEN   |=  0x20;
  GPIOE->PDR   |=  0x20;
  GPIOE->AFSEL &= ~0x20;
}

uint32_t ultrasonic_sensor_pulse_read(void)
{
  uint32_t count;
  uint32_t timeout;

  // Ensure low before triggering
  GPIOD->DATA &= ~0x04;
  SysTick_Delay1us(5);

  // 10 µs trigger pulse 
  GPIOD->DATA |=  0x04;
  SysTick_Delay1us(10);
  GPIOD->DATA &= ~0x04;

  // Wait for echo HIGH (rising edge) with timeout
  timeout = 0;
  while ((GPIOE->DATA & 0x20) == 0)
  {
    SysTick_Delay1us(1);
    timeout++;

    if (timeout > 25000)
    {
      return 0; // No echo detected 
    }
  }

  // Measure HIGH time (pulse width)
  count = 0;
  timeout = 0;

  while ((GPIOE->DATA & 0x20) != 0)
  {
    SysTick_Delay1us(1);
    count++;

    timeout++;
    if (timeout > 25000)
    {
      break;
    }
  }

  return count;
}

uint32_t ultrasonic_sensor_distance_read(void)
{
  uint32_t pulse = ultrasonic_sensor_pulse_read();

  if (pulse == 0)
  {
    // No valid echo ? treat as far distance
    return 1000;
  }

  // Convert to cm: distance ˜ pulse_us / 58
  return pulse / 58;
}
