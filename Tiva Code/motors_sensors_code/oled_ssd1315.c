// oled_ssd1315.c
#include "TM4C123GH6PM.h"
#include "i2c1.h"
#include "oled_ssd1315.h"
#include <stdint.h>

// 8-bit I2C address (0x3C << 1) = 0x78
static const uint8_t OLED_Address = 0x78;

// Control bytes
static const uint8_t OLED_Control_Command = 0x00;
static const uint8_t OLED_Control_Data    = 0x40;

// Track current cursor
static uint8_t CurrentCol = 0;
static uint8_t CurrentPage = 0;

// 5x7 font (subset, enough for FORWARD, STOP, TURN LEFT, WAIT, etc.)
static const uint8_t Font5x7[][5] = {
    // Space (index 0)
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' '

    // 'A' (index 1)
    {0x7C, 0x12, 0x11, 0x12, 0x7C},
    // 'D' (2)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // 'E' (3)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // 'F' (4)
    {0x7F, 0x09, 0x09, 0x09, 0x01},
    // 'G' (5)
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    // 'I' (6)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // 'L' (7)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // 'N' (8)
    {0x7F, 0x02, 0x0C, 0x10, 0x7F},
    // 'O' (9)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // 'P' (10)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // 'R' (11)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // 'S' (12)
    {0x26, 0x49, 0x49, 0x49, 0x32},
    // 'T' (13)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // 'U' (14)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // 'W' (15)
    {0x7F, 0x20, 0x18, 0x20, 0x7F},
    // 'Y' (16)
    {0x07, 0x08, 0x70, 0x08, 0x07}
};

// Map ASCII char to index in Font5x7
static uint8_t Font_Index(char c)
{
    if (c == ' ') return 0;

    if (c == 'A') return 1;
    if (c == 'D') return 2;
    if (c == 'E') return 3;
    if (c == 'F') return 4;
    if (c == 'G') return 5;
    if (c == 'I') return 6;
    if (c == 'L') return 7;
    if (c == 'N') return 8;
    if (c == 'O') return 9;
    if (c == 'P') return 10;
    if (c == 'R') return 11;
    if (c == 'S') return 12;
    if (c == 'T') return 13;
    if (c == 'U') return 14;
    if (c == 'W') return 15;
    if (c == 'Y') return 16;

    // Unknown -> space
    return 0;
}

static void OLED_SendCommand(uint8_t cmd)
{
    uint8_t buffer[2];
    buffer[0] = OLED_Control_Command;
    buffer[1] = cmd;
    I2C1_WriteBytes(OLED_Address, buffer, 2);
}

static void OLED_SendData(uint8_t *data, uint32_t length)
{
    uint8_t buffer[2];
    uint32_t i;

    buffer[0] = OLED_Control_Data;

    for (i = 0; i < length; i++)
    {
        buffer[1] = data[i];
        I2C1_WriteBytes(OLED_Address, buffer, 2);
    }
}

void OLED_SetCursor(uint8_t col, uint8_t page)
{
    uint8_t low;
    uint8_t high;
    uint8_t highCmd;

    CurrentCol = col;
    CurrentPage = page;

    // Set page address
    OLED_SendCommand(0xB0 + page);

    // Split column into high and low nibble without shifts
    high = 0;
    while ((uint8_t)(col - (high * 16)) >= 16)
    {
        high = (uint8_t)(high + 1);
    }
    low = (uint8_t)(col - (high * 16));

    // Low column bits command
    OLED_SendCommand(low);

    // High column bits command (0x10 + high)
    highCmd = (uint8_t)(0x10 + high);
    OLED_SendCommand(highCmd);
}

void OLED_Clear(void)
{
    uint8_t empty = 0x00;
    uint8_t page;
    uint16_t col;

    for (page = 0; page < 8; page++)
    {
        OLED_SetCursor(0, page);
        for (col = 0; col < 128; col++)
        {
            OLED_SendData(&empty, 1);
        }
    }

    OLED_SetCursor(0, 0);
}

static void OLED_WriteChar(char c)
{
    uint8_t index = Font_Index(c);
    uint8_t column;
    uint8_t i;
    uint8_t space = 0x00;

    for (i = 0; i < 5; i++)
    {
        column = Font5x7[index][i];
        OLED_SendData(&column, 1);
    }

    // one blank column as spacing
    OLED_SendData(&space, 1);
}

void OLED_WriteString(const char *text)
{
    while (*text != 0)
    {
        OLED_WriteChar(*text);
        text++;
    }
}

// ---------- BIG FONT (3x width, 2x height, one word, centered) ----------

// One large character: 5 columns scaled 3x horizontally + 3 blank cols = 18 px
static void OLED_WriteChar_Large_At(uint8_t colStart, uint8_t pageTop, char c)
{
    uint8_t index = Font_Index(c);
    uint8_t column;
    uint8_t i;
    uint8_t space = 0x00;
    uint8_t pageBottom;
    uint8_t colLocal;

    const uint8_t scaleX      = 3;
    const uint8_t spacingCols = 3;
    const uint8_t charWidth   = (uint8_t)(5 * scaleX + spacingCols); // 18 pixels

    pageBottom = (uint8_t)(pageTop + 1);   // 2 pages tall (16 px)

    // -------- TOP half (pageTop) --------
    OLED_SetCursor(colStart, pageTop);

    for (i = 0; i < 5; i++)
    {
        column = Font5x7[index][i];

        // Write same column 3 times for 3x width
        OLED_SendData(&column, 1);
        OLED_SendData(&column, 1);
        OLED_SendData(&column, 1);
    }

    // Spacing: three blank columns
    OLED_SendData(&space, 1);
    OLED_SendData(&space, 1);
    OLED_SendData(&space, 1);

    // -------- BOTTOM half (pageBottom) --------
    OLED_SetCursor(colStart, pageBottom);

    for (i = 0; i < 5; i++)
    {
        column = Font5x7[index][i];

        OLED_SendData(&column, 1);
        OLED_SendData(&column, 1);
        OLED_SendData(&column, 1);
    }

    OLED_SendData(&space, 1);
    OLED_SendData(&space, 1);
    OLED_SendData(&space, 1);

    // Advance cursor for caller
    colLocal = (uint8_t)(colStart + charWidth);
    OLED_SetCursor(colLocal, pageTop);
}

void OLED_WriteString_Large(const char *text)
{
    // 1) Extract and count ONLY the first word (up to space or end)
    uint8_t len = 0;
    const char *p = text;

    while (*p != 0 && *p != ' ')
    {
        len++;
        p++;
    }

    if (len == 0)
        return;

    const uint8_t scaleX      = 3;
    const uint8_t spacingCols = 3;
    const uint8_t charWidth   = (uint8_t)(5 * scaleX + spacingCols); // 18 pixels
    uint8_t totalWidth        = (uint8_t)(len * charWidth);

    // 2) Clear entire screen so NO old word remains
    OLED_Clear();

    // 3) Compute centered starting column
    uint8_t colStart = 0;
    if (totalWidth < 128)
    {
        colStart = (uint8_t)((128 - totalWidth) / 2);
    }

    // 4) Vertically center: use pages 3 and 4 (middle of 0..7)
    uint8_t pageTop = 3;

    // 5) Draw just the first word
    uint8_t i;
    uint8_t x = colStart;
    for (i = 0; i < len; i++)
    {
        OLED_WriteChar_Large_At(x, pageTop, text[i]);
        x = (uint8_t)(x + charWidth);
    }
}

void OLED_Init(void)
{
    I2C1_Init();

    // Basic init for SSD1315/SSD1306 128x64
    OLED_SendCommand(0xAE); // display off
    OLED_SendCommand(0x20); // memory addr mode
    OLED_SendCommand(0x00); // horizontal
    OLED_SendCommand(0xB0); // page 0
    OLED_SendCommand(0xC8); // COM scan dir remap
    OLED_SendCommand(0x00); // low column
    OLED_SendCommand(0x10); // high column
    OLED_SendCommand(0x40); // start line
    OLED_SendCommand(0x81); // contrast
    OLED_SendCommand(0x7F);
    OLED_SendCommand(0xA1); // segment remap
    OLED_SendCommand(0xA6); // normal display
    OLED_SendCommand(0xA8); // multiplex
    OLED_SendCommand(0x3F); // 1/64
    OLED_SendCommand(0xA4); // display follows RAM
    OLED_SendCommand(0xD3); // display offset
    OLED_SendCommand(0x00);
    OLED_SendCommand(0xD5); // clock divide
    OLED_SendCommand(0x80);
    OLED_SendCommand(0xD9); // pre-charge
    OLED_SendCommand(0xF1);
    OLED_SendCommand(0xDA); // COM pins
    OLED_SendCommand(0x12);
    OLED_SendCommand(0xDB); // VCOM detect
    OLED_SendCommand(0x40);
    OLED_SendCommand(0x8D); // charge pump
    OLED_SendCommand(0x14);
    OLED_SendCommand(0xAF); // display on

    OLED_Clear();
}
