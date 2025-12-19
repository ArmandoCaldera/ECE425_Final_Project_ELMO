// i2c1.h
#ifndef I2C1_H_
#define I2C1_H_

#include <stdint.h>

void I2C1_Init(void);
void I2C1_WriteBytes(uint8_t slave_address_8bit, uint8_t *data, uint32_t length);

#endif // I2C1_H_
