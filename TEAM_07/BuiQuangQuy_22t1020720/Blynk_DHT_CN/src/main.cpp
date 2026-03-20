#define BLYNK_TEMPLATE_ID "TMPL6fs3cZUk8"
#define BLYNK_TEMPLATE_NAME "BLYNK ESP32 TM1637"
#define BLYNK_AUTH_TOKEN "SNVvqqGsWmbRXfacfo_ypDirFv_UQzPk"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BTN_BLED    23    
#define PIN_BLED    21    
#define CLK         18    
#define DIO         19    
#define DHTPIN      16    
#define DHTTYPE     DHT22 

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
bool blueLED_ON = true;

// Hàm gửi dữ liệu DHT22
void sendSensorData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
    Serial.printf("Temp: %.1f C | Hum: %.1f %%\n", t, h);
  }
}

void sendUptime() {
  static unsigned long startTime = millis();
  unsigned long uptimeSeconds = (millis() - startTime) / 1000;
  Blynk.virtualWrite(V0, uptimeSeconds);
  if (blueLED_ON) display.showNumberDec(uptimeSeconds, false);
}

BLYNK_WRITE(V1) {
  blueLED_ON = param.asInt();
  digitalWrite(PIN_BLED, blueLED_ON ? HIGH : LOW);
  if (!blueLED_ON) display.clear();
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V4, "Hồ Thị Thanh Bình");
  Blynk.syncVirtual(V1);
  Serial.println(">>> ĐÃ KẾT NỐI BLYNK!");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BLED, OUTPUT);
  pinMode(BTN_BLED, INPUT_PULLUP);
  digitalWrite(PIN_BLED, blueLED_ON ? HIGH : LOW);
  display.setBrightness(0x0f);
  dht.begin();

  
  Serial.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, pass);
  

  IPAddress dns(8, 8, 8, 8); 
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);


  Blynk.config(BLYNK_AUTH_TOKEN, IPAddress(128, 199, 144, 129), 80);
  
  timer.setInterval(1000L, sendUptime);
  timer.setInterval(5000L, sendSensorData);
  Serial.println("\nCau hinh hoan tat!");
}

void loop() {
  Blynk.run();
  timer.run();
  
  
  static int lastState = HIGH;
  int btn = digitalRead(BTN_BLED);
  if (btn != lastState && btn == HIGH) {
    blueLED_ON = !blueLED_ON;
    digitalWrite(PIN_BLED, blueLED_ON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueLED_ON);
    if (!blueLED_ON) display.clear();
    delay(50); 
  }
  lastState = btn;
}