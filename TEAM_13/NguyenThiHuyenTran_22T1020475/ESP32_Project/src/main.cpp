#include <Arduino.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// ===== DHT =====
#define DHT_PIN   D3
#define DHT_TYPE  DHT11

// ===== LED =====
#define LED_RED   D6
#define LED_BLUE  D4   // active LOW

// ===== MQ2 =====
#define MQ2_PIN   A0

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== DHT =====
DHT dht(DHT_PIN, DHT_TYPE);

// ===== VARIABLES =====
float temperature = 0;
float humidity    = 0;
int gasValue      = 0;

bool ledState = false;
int dhtErrors = 0;

unsigned long lastDHTRead    = 0;
unsigned long lastLEDBlink   = 0;
unsigned long lastOLEDUpdate = 0;

const unsigned long DHT_INTERVAL  = 2000;
const unsigned long LED_INTERVAL  = 500;
const unsigned long OLED_INTERVAL = 1000;


// ===== READ DHT =====
void readDHT() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    temperature = t;
    humidity = h;
    dhtErrors = 0;

    Serial.printf("[DHT] Temp: %.1f C | Hum: %.1f %%\n", temperature, humidity);
  }
  else {
    dhtErrors++;
    Serial.printf("[DHT ERROR] %d\n", dhtErrors);
  }
}

// ===== READ MQ2 =====
void readMQ2() {

  int sum = 0;

  for(int i=0;i<5;i++){
    sum += analogRead(MQ2_PIN);
    delay(5);
  }

  gasValue = sum / 5;

  Serial.printf("[MQ2] Gas value: %d\n", gasValue);
}


// ===== OLED DISPLAY =====
void updateOLED() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(15,10,"ESP8266 IOT SENSOR");
  u8g2.drawHLine(0,12,128);

  char buf[32];

// HIỂN THỊ NHIỆT ĐỘ - ĐỘ ẨM
  if(dhtErrors > 3){
    u8g2.drawStr(0,30,"DHT SENSOR ERROR");
  }
  else{
    snprintf(buf,sizeof(buf),"Temp: %.1f C",temperature);
    u8g2.drawStr(0,26,buf);

    snprintf(buf,sizeof(buf),"Hum : %.1f %%",humidity);
    u8g2.drawStr(0,38,buf);
  }

  // HIỂN THỊ GAS
  snprintf(buf,sizeof(buf),"Gas : %d",gasValue);
  u8g2.drawStr(0,50,buf);

   // HIỂN THỊ LED
  snprintf(buf,sizeof(buf),"LED: %s", ledState ? "ON" : "OFF");
  u8g2.drawStr(80,50,buf);

    // CẢNH BÁO GAS
  if(gasValue > 500){
    u8g2.drawStr(0,62,"!!! GAS ALERT !!!");
  }
  else{
    u8g2.drawStr(0,62,"Status: Normal");
  }

  u8g2.sendBuffer();
}

// ===== SETUP =====
void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("==== ESP8266 IOT PROJECT ====");

  // LED
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, HIGH);

  // DHT
  dht.begin();
  Serial.println("DHT Starting...");
  delay(2000);

  // OLED
  Wire.begin(D2, D1);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(25,30,"SYSTEM START");
  u8g2.sendBuffer();
  delay(1500);

  readDHT();
  readMQ2();
  updateOLED();

  Serial.println("System Ready");
}

// ===== LOOP =====
void loop() {
  unsigned long now = millis();
  // ===== LED BLINK =====
  if(now - lastLEDBlink >= LED_INTERVAL){
    lastLEDBlink = now;
    ledState = !ledState;
    digitalWrite(LED_RED, ledState);
    digitalWrite(LED_BLUE, !ledState);
  }

  // ===== SENSOR READ =====
  if(now - lastDHTRead >= DHT_INTERVAL){
    lastDHTRead = now;
    readDHT();
    readMQ2();
  }

  // ===== OLED UPDATE =====
  if(now - lastOLEDUpdate >= OLED_INTERVAL){
    lastOLEDUpdate = now;
    updateOLED();
  }
}