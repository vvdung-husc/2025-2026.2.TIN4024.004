#include "main.h"
//#include <TM1637Display.h>

#define PIN_LED_RED 27
#define PIN_LED_YELLOW 26
#define PIN_LED_GREEN 25

// Module connection pins (Digital Pins)
#define CLK 18
#define DIO 19

#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE 21

TM1637Display display(CLK, DIO);
BUTTON btnBlue;
LED ledBlue;
// LED ledYellow;
Traffic_Blink traffic;

void ProcessButtonPressed()
{
  static int prevSecond = -1;
  static bool prevButton = false;
  btnBlue.processPressed();

  bool pressed = btnBlue.isPressed();
  if (prevButton != pressed)
  {
    if (pressed)
    {
      ledBlue.setStatus(true);
      printf("*** DISPLAY ON ***\n");
    }
    else
    {
      ledBlue.setStatus(false);
      display.clear();
      printf("*** DISPLAY OFF ***\n");
    }
    prevButton = pressed;
  }

  if (pressed)
  {
    int secondCount = traffic.getCount();
    if (prevSecond != secondCount)
    {
      prevSecond = secondCount;
      display.showNumberDec(secondCount);
    }
  }
}

void setup()
{
  printf("*** PROJECT TRAFFIC LIGHT ***\n");
  // ledYellow.setup(PIN_LED_YELLOW, "YELLOW");
  btnBlue.setup(PIN_BUTTON_DISPLAY);
  ledBlue.setup(PIN_LED_BLUE, "BLUE");
  display.setBrightness(0x0a);
  display.clear();
  traffic.setup_Pin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traffic.setup_WaitTime(5, 3, 7);
}

void loop()
{
  // ledYellow.blink();

  ProcessButtonPressed();

  traffic.blink(btnBlue.isPressed());
  
}