// i2c1.c
#include "TM4C123GH6PM.h"
#include "i2c1.h"
#include <stdint.h>

static void I2C1_WaitUntilDone(void)
{
	// Wait while busy
  while ((I2C1->MCS & 0x01) != 0) {}
}

void I2C1_Init(void)
{
  // Enable I2C1 and Port A clocks
  SYSCTL->RCGCI2C |= (1U << 1);   // I2C1
  SYSCTL->RCGCGPIO |= (1U << 0);  // GPIOA

  // Wait for Port A to be ready
  while ((SYSCTL->PRGPIO & (1U << 0)) == 0) {}

  // PA6 (SCL), PA7 (SDA) alternate function
  GPIOA->AFSEL |= (1U << 6) | (1U << 7);
  GPIOA->DEN   |= (1U << 6) | (1U << 7);

  // Set PCTL for PA6, PA7 to I2C (AF = 3)
  GPIOA->PCTL &= ~((0xFU << (6*4)) | (0xFU << (7*4)));
  GPIOA->PCTL |=  ((0x3U << (6*4)) | (0x3U << (7*4)));

  // SDA (PA7) open-drain
  GPIOA->ODR |= (1U << 7);

  // Enable I2C1 master
  I2C1->MCR = 0x10;   // Master mode

  // Set SCL frequency (assumes 16 MHz system clock, 100 kHz I2C)
  // TPR = (16MHz / (2*10*100kHz)) - 1 = 7
  I2C1->MTPR = 7;
}

void I2C1_WriteBytes(uint8_t slave_address_8bit, uint8_t *data, uint32_t length)
{
  uint32_t i;

	if (length == 0) return;

  // Load 7-bit address in bits 7:1, R/W=0 in bit 0
  // Your code already passes 8-bit address (e.g. 0x78 = 0x3C<<1)
  I2C1->MSA = slave_address_8bit;

  // Send first byte with START
  I2C1->MDR = data[0];
  I2C1->MCS = 0x03;  // START + RUN

  I2C1_WaitUntilDone();

  if (length == 1)
  {
		// STOP is already sent in the first transfer
    return;
  }

    // Middle bytes
  for (i = 1; i < (length - 1); i++)
  {
  	I2C1->MDR = data[i];
    I2C1->MCS = 0x01;  // RUN, no START, no STOP
    I2C1_WaitUntilDone();
  }

  // Last byte + STOP
  I2C1->MDR = data[length - 1];
  I2C1->MCS = 0x05;  // RUN + STOP
  I2C1_WaitUntilDone();
}
