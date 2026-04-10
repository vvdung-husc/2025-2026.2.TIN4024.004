
#include <Arduino.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

#define DHT_PIN     D3        
#define DHT_TYPE    DHT11    


// --- LED ---
#define LED_RED     D6        // LED đỏ
#define LED_BLUE    D4        // LED xanh

// --- MQ2 ---
#define MQ2_PIN     A0

// OLED SH1106 I2C: SDA=D2, SCL=D1
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// DHT
DHT dht(DHT_PIN, DHT_TYPE);

// BIẾN
float temperature = 0.0;
float humidity    = 0.0;
int   gasValue    = 0;
bool  ledState    = false;
int   dhtErrors   = 0;

unsigned long lastDHTRead    = 0;
unsigned long lastLEDBlink   = 0;
unsigned long lastOLEDUpdate = 0;

const unsigned long DHT_INTERVAL  = 2000;
const unsigned long LED_INTERVAL  = 500;
const unsigned long OLED_INTERVAL = 1000;

// ĐỌC DHT
void readDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h) && t != 0 && h != 0) {
    temperature = t;
    humidity    = h;
    dhtErrors   = 0;
    Serial.printf("[DHT OK] Nhiet do: %.1f C | Do am: %.1f%%\n", temperature, humidity);
  } else {
    dhtErrors++;
    Serial.printf("[DHT ERR #%d] Khong doc duoc! DHT_PIN=D%d, Type=%s\n",
                  dhtErrors,
                  DHT_PIN - D0,
                  (DHT_TYPE == DHT11) ? "DHT11" : "DHT22");
    if (dhtErrors == 3) {
      Serial.println("  --> Goi y: Thu doi DHT_PIN thanh D4, D5, D6...");
      Serial.println("  --> Hoac doi DHT_TYPE: DHT11 <-> DHT22");
    }
  }
}

// ĐỌC MQ2
void readMQ2() {
  gasValue = analogRead(MQ2_PIN);
  Serial.printf("[MQ2] Gas: %d\n", gasValue);
}

// HIỂN THỊ OLED
void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(8, 10, "==  IOT SENSOR  ==");
  u8g2.drawHLine(0, 13, 128);

  char buf[32];

  if (dhtErrors > 3) {
    u8g2.drawStr(0, 26, "DHT: LOI! Xem Serial");
    u8g2.drawStr(0, 38, "Doi lai chan DHT_PIN");
  } else {
    snprintf(buf, sizeof(buf), "Nhiet do: %.1f C", temperature);
    u8g2.drawStr(0, 26, buf);
    snprintf(buf, sizeof(buf), "Do am   : %.1f %%", humidity);
    u8g2.drawStr(0, 38, buf);
  }

  snprintf(buf, sizeof(buf), "Gas(MQ2): %d", gasValue);
  u8g2.drawStr(0, 50, buf);

  if (gasValue > 500) {
    u8g2.drawStr(0, 62, "! CANH BAO KHI GAS !");
  } else {
    snprintf(buf, sizeof(buf), "LED: %s  Gas: OK", ledState ? "ON " : "OFF");
    u8g2.drawStr(0, 62, buf);
  }

  u8g2.sendBuffer();
}

// SETUP
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=============================");
  Serial.println("   ESP32_Project BOOT");
  Serial.println("=============================");
  Serial.printf("DHT_PIN  : D%d (GPIO%d)\n", DHT_PIN - D0, DHT_PIN);
  Serial.printf("DHT_TYPE : %s\n", (DHT_TYPE == DHT11) ? "DHT11" : "DHT22");
  Serial.println("-----------------------------");

  // LED
  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_BLUE,   OUTPUT);

  digitalWrite(LED_RED,    LOW);
  digitalWrite(LED_BLUE,   HIGH); // active LOW → HIGH = tắt

  // DHT cần 2 giây để ổn định sau khi cấp nguồn
  dht.begin();
  Serial.println("[DHT] Dang khoi dong, cho 2 giay...");
  delay(2000);

  // OLED
  Wire.begin(D2, D1);
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(20, 25, "ESP8266 IoT");
  u8g2.drawStr(10, 42, "Linh Lê Thị Thuỳ");
  u8g2.sendBuffer();
  delay(1500);

  readDHT();
  readMQ2();
  updateOLED();

  Serial.println("[READY] San sang!\n");
}

//LOOP
void loop() {
  unsigned long now = millis();

  //LED NHẤP NHÁY mỗi 500ms
  if (now - lastLEDBlink >= LED_INTERVAL) {
    lastLEDBlink = now;
    ledState = !ledState;

    digitalWrite(LED_RED,  ledState ? HIGH : LOW); // LED đỏ: HIGH = sáng
    digitalWrite(LED_BLUE, ledState ? LOW : HIGH); // LED xanh ESP8266: LOW = sáng
  }

  // ✅ ĐỌC CẢM BIẾN mỗi 2 giây
  if (now - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = now;
    readDHT();
    readMQ2();
  }

  // ✅ CẬP NHẬT OLED mỗi 1 giây
  if (now - lastOLEDUpdate >= OLED_INTERVAL) {
    lastOLEDUpdate = now;
    updateOLED();
  }
}
