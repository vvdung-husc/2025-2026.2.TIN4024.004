#include "main.h"

Trafic_Blink traficLight;
LDR ldrSensor;
TM1637Display display(PIN_CLK, PIN_DIO);

void setup() {
  Serial.begin(115200);
  printf("Welcome IoT (Your wiring)\n");

  ldrSensor.DAY_THRESHOLD = 2000;      // bạn có thể chỉnh 1500~2500 tùy wokwi
  ldrSensor.setup(PIN_LDR, true);      // VCC = 5V (mạch bạn nối 5V)

  traficLight.setupPin(
    PIN_LED_RED,
    PIN_LED_YELLOW,
    PIN_LED_GREEN,
    PIN_LED_BLUE,
    PIN_BUTTON_BLUE
  );
  traficLight.setupWaitTime(5, 3, 7);  // red, yellow, green (seconds)

  display.setBrightness(0x0A);
  display.clear();
}

void loop() {
  traficLight.run(ldrSensor, display, true);
}
