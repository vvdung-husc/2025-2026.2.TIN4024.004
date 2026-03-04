#include "main.h"

// --- Định nghĩa chân (Pin Definitions) ---
#define PIN_LED_RED     4   // led1
#define PIN_LED_YELLOW  2   // led2
#define PIN_LED_GREEN   15  // led3
#define PIN_DHT         16  // dht1
#define PIN_OLED_SDA    13  // oled1 SDA
#define PIN_OLED_SCL    12  // oled1 SCL

#define DHT_TYPE        DHT22
#define OLED_ADDR       0x3C

// --- Khởi tạo đối tượng ---
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