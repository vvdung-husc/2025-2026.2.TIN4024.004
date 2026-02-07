/*THÔNG TIN NHÓM 1
1.
2. Trương Công Bin
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define DHTPIN 16
#define DHTTYPE DHT22

#define PIN_LED_GREEN 15
#define PIN_LED_YELLOW 2
#define PIN_LED_RED 4  

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);


void handleSensorLogic();
void updateDisplay();
void handleLedBlink();
void turnOffAllLeds();


struct TempState {
  float maxTemp;
  const char* label;
  int ledPin;
};


TempState states[] = {
  {13.0, "TOO COLD", PIN_LED_GREEN}, 
  {20.0, "COLD",     PIN_LED_GREEN}, 
  {25.0, "COOL",     PIN_LED_YELLOW},
  {30.0, "WARM",     PIN_LED_YELLOW},
  {35.0, "HOT",      PIN_LED_RED},   
  {999.0,"TOO HOT",  PIN_LED_RED} 
};

const long SENSOR_INTERVAL = 2000;
const long BLINK_INTERVAL = 500;
unsigned long previousSensorMillis = 0;
unsigned long previousBlinkMillis = 0;
bool ledState = LOW;

float currentTemp = 0;
float currentHum = 0;
int currentLedPin = -1;       
String currentStatus = "---";

void setup() {
  Serial.begin(115200);
  Wire.begin(13, 12);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED Failed"));
    for(;;);
  }
  
  dht.begin();
  
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
}
