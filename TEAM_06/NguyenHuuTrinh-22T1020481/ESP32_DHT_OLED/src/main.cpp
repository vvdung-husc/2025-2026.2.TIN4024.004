#include "main.h"

#define PIN_LED_RED     4   
#define PIN_LED_YELLOW  2   
#define PIN_LED_GREEN   15  
#define PIN_DHT         16  
#define PIN_OLED_SDA    13  
#define PIN_OLED_SCL    12  

#define DHT_TYPE        DHT22
#define OLED_ADDR       0x3C

MyLED ledRed;
MyLED ledYellow;
MyLED ledGreen;
MyDHT dhtSensor;
MyOLED oledDisplay;
SystemController mySystem;

void setup() {
    Serial.begin(SERIAL_BAUD);
    
    // Setup Hardware
    ledRed.setup(PIN_LED_RED);
    ledYellow.setup(PIN_LED_YELLOW);
    ledGreen.setup(PIN_LED_GREEN);
    dhtSensor.setup(PIN_DHT, DHT_TYPE);
    oledDisplay.setup(PIN_OLED_SDA, PIN_OLED_SCL, OLED_ADDR);
    
    Serial.println("System Started...");
}

void loop() {
    // Chạy toàn bộ hệ thống
    mySystem.run(ledGreen, ledYellow, ledRed, dhtSensor, oledDisplay);
}