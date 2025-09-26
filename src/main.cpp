#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include "SafeString.h"
#include "FlippedLCD.h"

#define LCD_FLIPPED

const int pinArmSwitch = 3;
const int pinPowerLed = 5;
const int pinBuzzer = 4;

// lcd pins
const int lcdRsPin = A4;
const int lcdEPin = A5;
const int lcdD4Pin = A3;
const int lcdD5Pin = A2;
const int lcdD6Pin = A1;
const int lcdD7Pin = A0;
int row = 0; // used for inputs

const byte ROWS = 4;
const byte COLS = 3;

// inputs
unsigned long lastCheck = 0;
const unsigned long checkInterval = 5;

enum State
{
  DISARMED,
  ARMED,
  TICKING,
  BLOWN,
  DEFUSED
};

State state = DISARMED;

const int codeLength = 7;
SafeString code = SafeString(codeLength + 1);       // code needed to defuse the bomb
SafeString defuseCode = SafeString(codeLength + 1); // defuse code entered;

byte rowPins[ROWS] = {11, 6, 7, 9};
byte colPins[COLS] = {10, 12, 8};

char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

LiquidCrystal lcd(lcdRsPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin);
#ifdef LCD_FLIPPED
FlippedLCD fLCD(lcd);
#endif

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long lastBlink = 0;
const unsigned long blinkDelay = 500;
const unsigned long blinkDuration = 250;

/// @brief power led, always blinks when there is power
void blinkPowerLed()
{
  if (millis() - lastBlink >= blinkDelay)
  {
    digitalWrite(pinPowerLed, HIGH);
    lastBlink = millis();
  }
  else if (millis() - lastBlink >= blinkDuration)
  {
    digitalWrite(pinPowerLed, LOW);
  }
}

inline void lcdSetCursor(int col, int row)
{
#ifndef LCD_FLIPPED
  lcd.setCursor((LCD_COLS - 1) - col, (LCD_ROWS - 1) - row);
#else
  fLCD.setCursor(col, row);
#endif
}


inline void lcdPrint(const char ch, int col, int row)
{
#ifndef LCD_FLIPPED
  lcdSetCursor(col, row);
  lcd.print(ch);
#else
  fLCD.printFlipped(ch, col, row);
#endif
}

inline void lcdPrint(const char str[])
{
#ifndef LCD_FLIPPED
  lcd.print(str);
#else
  fLCD.printFlipped(str);
#endif
}

/// @brief maps keypad input to string passed as argument
/// @param string
/// @return
bool inputKey(SafeString &string)
{
  char key = keypad.getKey();

  if (key)
  {
    if (key != '*' && key != '#' && string.length() < codeLength) // append char
    {
      if (string.length() == 0)
        lcd.clear();

      string.appendChar(key); // append the character to the stored code
      Serial.println("key pressed: " + String(key));
    }
    else if (key == '*' && string.length() > 0) // clear char
    {
      string.deleteLast();
    }

    SafeString output(16);
    for (int i = 0; i < 7; i++)
    {
      if (i < 7 - string.length())
        output.appendChar('-'); // append dashes up to the entered text
      else
        output.appendChar(string.getData()[i - (7 - string.length())]); // append entered code after the dashes
    }

    lcdSetCursor(5, 0);
    lcdPrint(output.getData()); // print the pressed key
  }

  return false;
}

/// @brief prints asterisks as a code placeholder
inline void printCodePlaceholder()
{
  lcd.clear();
  lcdSetCursor(5, 0);  // center it
  lcdPrint("*******"); // print asterisks
  lcdSetCursor(0, 0);
}

void armed()
{
  Serial.println("ARMED");
  code.clear(); // clear entered code

  bool codeEntered = true;

  while (state == ARMED)
  {
    unsigned long now = millis();

    if (now - lastCheck >= checkInterval)
    {
      inputKey(code);
      if (code.length() > 0)
        codeEntered = true;
      else if (code.length() == 0 && codeEntered) // check if code has been entered and cleared
      {
        printCodePlaceholder(); // print asterisks
        codeEntered = false;
      }

      if (code.length() == codeLength)
      {
        // printCodePlaceholder();
        state = TICKING; // code is 7 characters so we start the timer
        Serial.println("starting timer :3");
        return;
      }

      if (digitalRead(pinArmSwitch) == LOW)
        state = DISARMED;
    }

    blinkPowerLed();
  }
}

void ticking()
{
  const int countDownBeepsFreq = 3600; // beep frequency in hz
  float fLastBeep = 0;                 // time of last beep in ms
  unsigned long startTime = millis();
  unsigned long fTimer = millis() - startTime; // current timer in ms
  const unsigned long fTimerMax = 40000;       // max timer until detonation in ms
  const int blowingBeepsFreq = 2000;           // blowing beep frequency in hz
  const int blowingBeepsDelay = 80;            // blowing beep delay
  int blowingBeeps = 0;                        // final blowing beeps sounded
  const int blowingBeepsMax = 12;              // final beeps after relay sound before bomb blows

  defuseCode.clear(); // clear entered code
  row = 0;

  float fDelay = 0;

  const int movingDelay = 50;
  const int minMoveCounter = 0;
  const int maxMoveCounter = 15;
  int lastMoved = millis();
  int moveCounter = 0;
  bool moveRight = true;

  while (state == TICKING)
  {
    if ((fTimer - fLastBeep) >= fDelay)
    {
      float fComplete = 1.0 - (float)fTimer / float(fTimerMax); // percentage completion
      float fFreq = max(0.1 + 0.9 * fComplete, 0.15);           // frequency of the beeps
      fDelay = fFreq * 1000;                                    // delay in ms
      Serial.println(String(fComplete) + " " + String(fFreq) + " " + String(fDelay));

      tone(pinBuzzer, countDownBeepsFreq, 1);                                                            // beep
      Serial.println("beep " + String(fTimer) + " time since last beep: " + String(fTimer - fLastBeep)); // for debugging
      fLastBeep = fTimer;

      // boom
      if (fTimer >= fTimerMax) // timer expired
      {
        lcd.clear();
        lcdSetCursor(2, 0);
        lcdPrint("BYE BYE :3");

        // final blowing beeps
        while (blowingBeeps < blowingBeepsMax)
        {
          tone(pinBuzzer, blowingBeepsFreq, 25);
          blowingBeeps++;
          delay(blowingBeepsDelay);
          blinkPowerLed();
        }
        state = BLOWN;
        return;
      }
    }

    inputKey(defuseCode);

    if (defuseCode.length() == 0 && millis() - lastMoved >= movingDelay) // move placeholder across the screen like in cs
    {
      lcd.clear();
      lcdPrint('*', moveCounter, 0);
      lastMoved = millis();
      // moveCounter = (moveCounter + 1) % 9;

      // ping pong
      moveCounter += moveRight ? 1 : -1;
      if (moveCounter > maxMoveCounter)
      {
        moveRight = false;
        moveCounter = maxMoveCounter - 1;
      }
      if (moveCounter < minMoveCounter)
      {
        moveRight = true;
        moveCounter = minMoveCounter + 1;
      }
    }
    else if (defuseCode.length() == 7)
    {
      if (strcmp(defuseCode.getData(), code.getData()) == 0) // codes match
      {
        lcd.clear();
        lcdSetCursor(4, 0);
        lcdPrint("DEFUSED");
        state = DEFUSED;
        row = 0;
      }
      else // incorrect code entered
      {
        defuseCode.clear(); // clear entered code
        row = 0;            // reset row
        printCodePlaceholder();
      }
    }

    blinkPowerLed();
    fTimer = millis() - startTime; // current timer in ms
  }
}

void setup()
{
  Serial.begin(9600);
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcdPrint("hi :3");
  pinMode(pinArmSwitch, INPUT);
  pinMode(pinPowerLed, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
}

void loop()
{
  switch (state)
  {
  case DISARMED:
    if (digitalRead(pinArmSwitch) == HIGH) // check arm switch
      state = ARMED;
    lcd.clear(); // clear lcd
    break;
  case ARMED:
    armed();
    break;
  case TICKING:
    ticking();
    break;
  case BLOWN:
    if (digitalRead(pinArmSwitch) == LOW) // have to flip switch off and on to arm it again
      state = DISARMED;
    break;
  case DEFUSED:
    if (digitalRead(pinArmSwitch) == LOW) // have to flip switch off and on to arm it again
      state = DISARMED;
    break;
  }

  blinkPowerLed();
}