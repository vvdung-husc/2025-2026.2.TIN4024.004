/*
	THÔNG TIN NHÓM 3
	1. Nguyễn Đình Tuấn
	2. Nguyễn Đăng Hưng
	3. 
  4.
  5.
  6.
*/
#define BLYNK_TEMPLATE_ID "TMPL6lGFdsfCS"
#define BLYNK_TEMPLATE_NAME "IOT TEMPLATE"
#define BLYNK_AUTH_TOKEN "5YIFRBHjduRgj03jUZjm94APWi7rIPRs"

// IMPORTANT: Do NOT define BLYNK_PRINT Serial. 
// Serial is now dedicated strictly to Blynk communication.

#define HAS_DHT_SENSOR
#define DHT_TYPE  DHT11   // hoặc DHT22
 
#include <Arduino.h>
#include <BlynkSimpleStream.h> // Changed from BlynkSimpleEsp8266.h
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#ifdef HAS_DHT_SENSOR
  #include <DHT.h>
#endif

#define OLED_SDA    D2
#define OLED_SCL    D1
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C
 
#define DHT_PIN     D4
#define LED_PIN     D5
#define MQ2_PIN     A0

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
 
#ifdef HAS_DHT_SENSOR
  DHT dht(DHT_PIN, DHT_TYPE);
#endif
 
BlynkTimer timer;

bool    ledState       = false;
float   temperature    = 0.0f;
float   humidity       = 0.0f;
int     gasValue       = 0;  
unsigned long uptimeSeconds = 0;
 
uint8_t oledPage = 0;
#define OLED_PAGES 4
 
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void readDHT() {
#ifdef HAS_DHT_SENSOR
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;
#else
  temperature = 25.0f + (float)(random(-30, 60)) / 10.0f;
  humidity    = 60.0f + (float)(random(-150, 150)) / 10.0f;
#endif
}

void readGas() {
#ifdef HAS_MQ2_SENSOR
  int raw  = analogRead(MQ2_PIN);
  gasValue = map(raw, 0, 1023, 0, 1000);
#else
  gasValue = random(50, 301);
#endif
}

void updateUptime() {
  uptimeSeconds++;
}

void sendToBlynk() {
  unsigned long h = uptimeSeconds / 3600;
  unsigned long m = (uptimeSeconds % 3600) / 60;
  unsigned long s = uptimeSeconds % 60;
  char uptimeStr[16];
  snprintf(uptimeStr, sizeof(uptimeStr), "%02lu:%02lu:%02lu", h, m, s);
 
  Blynk.virtualWrite(V1, uptimeStr);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, gasValue);
}

void drawHeader(const char* title) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 9, OLED_WIDTH - 1, 9, SSD1306_WHITE);
}

void oledPageUptime() {
  drawHeader("=== STATUS ===");
 
  unsigned long h = uptimeSeconds / 3600;
  unsigned long m = (uptimeSeconds % 3600) / 60;
  unsigned long s = uptimeSeconds % 60;
 
  display.setTextSize(1);
  display.setCursor(0, 13);
  display.print("Uptime:");
  display.setTextSize(2);
  display.setCursor(0, 22);
  char buf[10];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);
  display.print(buf);
 
  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("LED: ");
  display.setTextSize(2);
  display.setCursor(30, 39);
  display.print(ledState ? "ON " : "OFF");
 
  // Changed indicator to USB
  display.setTextSize(1);
  display.setCursor(98, 0);
  display.print("[USB]");
}

void oledPageTempHum() {
  drawHeader("=== CLIMATE ===");
 
  display.setTextSize(1);
  display.setCursor(0, 13);
  display.print("Nhiet do:");
  display.setTextSize(2);
  display.setCursor(0, 22);
  display.print(temperature, 1);
  display.print(" C");
 
  display.setTextSize(1);
  display.setCursor(0, 42);
  display.print("Do am:");
  display.setTextSize(2);
  display.setCursor(0, 51);
  display.print(humidity, 1);
  display.print(" %");
}

void oledPageGas() {
  drawHeader("=== GAS MQ2 ===");
 
  display.setTextSize(1);
  display.setCursor(0, 13);
  display.print("Gas Level:");
 
  display.setTextSize(3);
  display.setCursor(0, 24);
  display.print(gasValue);
 
  display.setTextSize(1);
  display.setCursor(75, 36);
  display.print("ppm");
 
  display.setTextSize(1);
  display.setCursor(0, 54);
  if (gasValue > 400)      display.print("! CANH BAO NGUY HIEM !");
  else if (gasValue > 200) display.print("  ~ Muc trung binh ~");
  else                     display.print("     An toan         ");
}

void oledPageTeam() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("---- Team 3 ----");
  display.drawLine(0, 9, OLED_WIDTH - 1, 9, SSD1306_WHITE);
 
  display.setCursor(0, 12);
  display.println("1. Ng. Dinh Tuan");
  display.println("2. Ng. Dang Hung");
  display.println("3. Ha Huy Vi");
  display.println("4. Tran Thang");
}

void updateOLED() {
  display.clearDisplay();
 
  switch (oledPage) {
    case 0: oledPageUptime();  break;
    case 1: oledPageTempHum(); break;
    case 2: oledPageGas();     break;
    case 3: oledPageTeam();    break;
  }
 
  for (uint8_t i = 0; i < OLED_PAGES; i++) {
    if (i == oledPage)
      display.fillCircle(56 + i * 6, 62, 2, SSD1306_WHITE);
    else
      display.drawCircle(56 + i * 6, 62, 2, SSD1306_WHITE);
  }
 
  display.display();
  oledPage = (oledPage + 1) % OLED_PAGES;
}

void setup() {
  // Serial is now ONLY for Blynk. Do not use Serial.print.
  Serial.begin(115200);
  
  randomSeed(analogRead(A0) ^ millis());
 
  // GPIO
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
 
  // I2C + OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    display.println("Team 3 - ESP8266");
    display.setCursor(15, 35);
    display.println("Chay che do USB...");
    display.display();
  }
 
  // DHT
#ifdef HAS_DHT_SENSOR
  dht.begin();
#endif
 
  // Read sensors once before loop
  readDHT();
  readGas();
 
  // Start Blynk via USB Stream
  Blynk.begin(Serial, BLYNK_AUTH_TOKEN);
 
  delay(2000);
 
  // ── Timer ─────────────────────────────────────────────────
  timer.setInterval(1000L,  updateUptime);   
  timer.setInterval(2000L,  sendToBlynk);    
  timer.setInterval(2500L,  readDHT);        
  timer.setInterval(2600L,  readGas);        
  timer.setInterval(3000L,  updateOLED);     
}

void loop() {
  Blynk.run();
  timer.run();
}