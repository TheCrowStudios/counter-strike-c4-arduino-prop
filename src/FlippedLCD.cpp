#include "FlippedLCD.h"

struct CharMap
{
    char c;
    const uint8_t *bitmap;
};

static CharMap charTable[] = {
    {'A', FLIPPED_A}, {'B', FLIPPED_B}, {'C', FLIPPED_C}, {'D', FLIPPED_D}, {'E', FLIPPED_E}, {'F', FLIPPED_F}, {'G', FLIPPED_G}, {'H', FLIPPED_H}, {'I', FLIPPED_I}, {'J', FLIPPED_J}, {'K', FLIPPED_K}, {'L', FLIPPED_L}, {'M', FLIPPED_M}, {'N', FLIPPED_N}, {'O', FLIPPED_O}, {'P', FLIPPED_P}, {'Q', FLIPPED_Q}, {'R', FLIPPED_R}, {'S', FLIPPED_S}, {'T', FLIPPED_T}, {'U', FLIPPED_U}, {'V', FLIPPED_V}, {'W', FLIPPED_W}, {'X', FLIPPED_X}, {'Y', FLIPPED_Y}, {'Z', FLIPPED_Z}, {'0', FLIPPED_0}, {'1', FLIPPED_1}, {'2', FLIPPED_2}, {'3', FLIPPED_3}, {'4', FLIPPED_4}, {'5', FLIPPED_5}, {'6', FLIPPED_6}, {'7', FLIPPED_7}, {'8', FLIPPED_8}, {'9', FLIPPED_9}, {'*', FLIPPED_STAR}, {'_', FLIPPED_UNDERSCORE}, {'-', FLIPPED_DASH}, {':', FLIPPED_COLON}};

static const int tableSize = sizeof(charTable) / sizeof(CharMap);

FlippedLCD::FlippedLCD(LiquidCrystal &lcdInstance) : lcd(lcdInstance)
{
    memset(slot, 0xFF, sizeof(slot));
}

const uint8_t *FlippedLCD::getBitmap(char c)
{
    for (int i = 0; i < tableSize; i++)
    {
        if (charTable[i].c == c)
            return charTable[i].bitmap;
    }

    return nullptr;
}

void FlippedLCD::setCursor(uint8_t col, uint8_t row)
{
    lcd.setCursor((LCD_COLS - 1) - col, (LCD_ROWS - 1) - row);
}

void FlippedLCD::printFlipped(const char *str)
{
    int nextSlot = 0;

    Serial.println("printing flipped string");

    for (int i = 0; str[i]; i++)
    {
        char ch = str[i];

        if (!ch)
        {
            Serial.println("char not found");
            continue;
        }

        if (ch == ' ')
        {
            setCursor(i, 0); // set cursor
            lcd.print(ch);
            continue;
        }

        const uint8_t *bitmap = getBitmap(ch);

        int s;
        for (s = 0; s < MAX_SLOTS; s++)
        {
            if (slot[s] == ch) // already has that character in slot s
                break;
        }

        if (s == MAX_SLOTS)
        {
            s = nextSlot;
            lcd.createChar(s, (uint8_t *)bitmap);
            slot[s] = ch;
            nextSlot = (nextSlot + 1) % MAX_SLOTS;
        }

        setCursor(i, 0); // set cursor again as it is reset after createChar is called
        lcd.write(byte(s));
    }
}

void FlippedLCD::printFlipped(char ch)
{
    int nextSlot = 0;
    const uint8_t *bitmap = getBitmap(ch);

    Serial.println(ch);

    if (ch == ' ')
    {
        lcd.print(ch);
        return;
    }

    int s;
    for (s = 0; s < MAX_SLOTS; s++)
    {
        if (slot[s] == ch) // already has that character in slot s
            break;
    }

    if (s == MAX_SLOTS)
    {
        s = nextSlot;
        lcd.createChar(s, (uint8_t *)bitmap);
        slot[s] = ch;
        nextSlot = (nextSlot + 1) % MAX_SLOTS;
    }

    lcd.write(byte(s));
}

void FlippedLCD::flipBitmap(const uint8_t in[8], uint8_t out[8])
{
    for (int row = 0; row < 8; row++)
    {
        uint8_t original = in[7 - row]; // start from last row
        uint8_t flipped = 0;

        for (int bit = 0; bit < 5; bit++)
        {
            if (original & (1 << bit)) // ands the bits, checks for turned on bits
            {
                flipped |= (1 << (4 - bit)); // set the bits but flipped
            }
        }

        out[row] = flipped; // start from first row
    }
}