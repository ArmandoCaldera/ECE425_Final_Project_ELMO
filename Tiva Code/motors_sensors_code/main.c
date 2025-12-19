#include "TM4C123GH6PM.h"
#include "SysTick_Delay.h"
#include "PWM_Clock.h"
#include "PWM0_0.h"
#include "ultrasonic.h"
#include "oled_ssd1315.h"

// Motor-only functions (same as your WORKING code)
void Drive_Forward(void)
{
	PWM0_0_Forward();
}

void Drive_Stop(void)
{
  PWM0_0_Stop();
}

void Turn_Left(void)
{
  PWM0_0_Turn_Left();
}

// OLED display helpers
void Show_Forward(void)
{
  OLED_WriteString_Large("FORWARD");
}

void Show_Stop(void)
{
  OLED_WriteString_Large("STOP");
}

void Show_Waiting(void)
{
  OLED_WriteString_Large("WAITING");
}

void Show_Turn_Left(void)
{
  OLED_WriteString_Large("TURN LEFT");
}


int main(void)
{
  uint32_t distance;
  uint32_t threshold = 20;   // cm, or whatever units your function uses

  SysTick_Delay_Init();
  PWM_Clock_Init();
  OLED_Init();
  PWM0_0_Init(62500, 15000);   // same as your working code
  ultrasonic_Init();

  Drive_Stop();
  Show_Stop();
  SysTick_Delay1ms(5000);

  while (1)
  {
    // 1) First distance check
    distance = ultrasonic_sensor_distance_read();

    if (distance > threshold)
    {
      // Path is clear ? drive forward
      Drive_Forward();
      Show_Forward();
    }
      else
      {
        // Obstacle detected ? stop and wait
        Drive_Stop();
        Show_Waiting();
        SysTick_Delay1ms(5000);

        // 2) Recheck after waiting
        distance = ultrasonic_sensor_distance_read();

        if (distance > threshold)
        {
          // It cleared while waiting ? go forward
          Drive_Forward();
          Show_Forward();
        }
        else
        {
          // Still blocked ? TURN LEFT UNTIL PATH CLEARS
          Drive_Stop();
          Show_Stop();
          Show_Turn_Left();

          // keep turning left while obstacle is too close
          do
          {
            Turn_Left();
            SysTick_Delay1ms(200);   // adjust how "strong" each turn pulse is
            distance = ultrasonic_sensor_distance_read();
          } while (distance <= threshold);

            // Once clear, go forward again
            Drive_Forward();
            Show_Forward();
          }
			}

    SysTick_Delay1ms(50);   // small loop delay
  }
}
