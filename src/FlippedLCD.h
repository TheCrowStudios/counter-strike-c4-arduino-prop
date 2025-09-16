#ifndef FLIPPED_LCD_H
#define FLIPPED_LCD_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "FlippedChars.h"

#define MAX_SLOTS 8
#define LCD_COLS 16
#define LCD_ROWS 2

class FlippedLCD
{
public:
    FlippedLCD(LiquidCrystal &lcdInstance);

    void printFlipped(const char *str);
    void printFlipped(char ch, int col, int row);
    void setCursor(uint8_t col, uint8_t row);
    void flipBitmap(const uint8_t in[8], uint8_t out[8]);
    void incrementCol();

private:
    LiquidCrystal &lcd;
    char slot[MAX_SLOTS];

    const uint8_t *getBitmap(char c);
    uint8_t col;
    uint8_t row;
};

#endif