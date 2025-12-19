// oled_ssd1315.h
#ifndef OLED_SSD1315_H
#define OLED_SSD1315_H

#include <stdint.h>

void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t col, uint8_t page);
void OLED_WriteString(const char *text);
void OLED_WriteString_Large(const char *text);

#endif
