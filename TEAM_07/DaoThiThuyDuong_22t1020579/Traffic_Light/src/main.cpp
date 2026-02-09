#include "main.h"

//Định nghĩa chân cho đèn LED
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

//Định nghĩa cho LDR (Light Dependent Resistor)
#define PIN_LDR 13   // Analog GPIO13 connected to LDR

// TM1637
#define PIN_CLK  15
#define PIN_DIO  2

// LED + Button xanh dương
#define PIN_LED_BLUE      21
#define PIN_BUTTON_BLUE   23

Trafic_Blink traficLight;
LDR ldrSensor;

TM1637Display display(PIN_CLK, PIN_DIO);

void setup() {
  printf("Welcome IoT\n");

  ldrSensor.DAY_THRESHOLD = 2000; // Ngưỡng ánh sáng ban ngày
  ldrSensor.setup(PIN_LDR, false); // VCC = 3.3V hoặc 5V tùy bạn (đang để false như cũ)

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN, PIN_LED_BLUE, PIN_BUTTON_BLUE);
  traficLight.setupWaitTime(5, 3, 7); // seconds

  display.setBrightness(0x0A);
  display.clear();
}

void loop() {
  traficLight.run(ldrSensor, display);
}
