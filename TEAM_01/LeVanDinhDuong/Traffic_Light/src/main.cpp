#include "main.h"

#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

#define CLK 15
#define DIO 2

#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE       21
#define PIN_LDR            13  

TM1637Display display(CLK, DIO);
BUTTON btnBlue;
LED ledBlue;
Traffic_Blink traffic;
SENSOR_LDR ldr; 

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

void ProcessLDR()
{
    static unsigned long ulTimer = 0;
    if (!IsReady(ulTimer, 1000)) 
        return;
    int lightValue = ldr.getLux();
    bool isNight = ldr.isDark(2000); 
    if (isNight) {
        display.setBrightness(0x01); 
    } else {
        display.setBrightness(0x0f); 
    }
}

void setup()
{
  Serial.begin(115200); 
  printf("*** PROJECT TRAFFIC LIGHT + LDR ***\n");

  // Setup các module
  btnBlue.setup(PIN_BUTTON_DISPLAY);
  ledBlue.setup(PIN_LED_BLUE, "BLUE");
  ldr.setup(PIN_LDR);

  display.setBrightness(0x0f);
  display.clear();

  traffic.setup_Pin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traffic.setup_WaitTime(5, 3, 7); 
}

void loop()
{
  ProcessButtonPressed();
  ProcessLDR();
  traffic.blink(btnBlue.isPressed());
}