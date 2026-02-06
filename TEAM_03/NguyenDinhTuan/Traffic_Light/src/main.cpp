#include "main.h"

// Define Pins based on Wokwi diagram.json
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

#define CLK 15
#define DIO 2

#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE    21

TM1637Display display(CLK, DIO);
BUTTON btnBlue;
LED ledBlue;
Traffic_Blink traffic;

// --- GLOBAL STATE ---
bool displayMode = true; 

void ProcessButtonPressed()
{
  static int prevSecond = -1;
  static bool lastButtonState = false; 

  btnBlue.processPressed();
  bool currentButtonState = btnBlue.isPressed();

  // Edge Detection for Toggle
  if (currentButtonState == true && lastButtonState == false)
  {
      displayMode = !displayMode; 

      if (displayMode) 
      {
          ledBlue.setStatus(true);
          printf("*** DISPLAY TOGGLED: ON ***\n");
      } 
      else 
      {
          ledBlue.setStatus(false);
          display.clear(); 
          printf("*** DISPLAY TOGGLED: OFF ***\n");
      }
  }
  
  lastButtonState = currentButtonState;

  // Update Display
  if (displayMode)
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
  Serial.begin(115200);
  printf("*** PROJECT TRAFFIC LIGHT (SYNCED) ***\n");

  btnBlue.setup(PIN_BUTTON_DISPLAY);
  ledBlue.setup(PIN_LED_BLUE, "BLUE");
  
  display.setBrightness(0x0a);
  display.clear();

  traffic.setup_Pin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traffic.setup_WaitTime(5, 3, 7); 

  // Default ON
  displayMode = true;        
  ledBlue.setStatus(true);   
  printf("*** SYSTEM STARTED: DISPLAY ON ***\n");
}

void loop()
{
  ProcessButtonPressed();
  traffic.blink(displayMode);
}