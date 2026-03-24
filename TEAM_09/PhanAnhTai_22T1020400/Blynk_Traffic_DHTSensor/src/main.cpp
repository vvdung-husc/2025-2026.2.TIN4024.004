#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Thông tin kết nối Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6g8lwg_W0"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "oCYKQ9AR1TSwjSBz6bz1474GBZTTZ86b"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* Cấu hình chân cắm */
#define BTN_LED 23
#define PIN_LED 21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

unsigned long currentMiliseconds = 0;
unsigned long uptime = 0; 
bool blueButtonON = true;

/* Khai báo hàm */
bool IsReady(unsigned long &timer, uint32_t interval);
void updateButton();
void uptimeTask();
void readDHT();

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(BTN_LED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected!");

  digitalWrite(PIN_LED, blueButtonON ? HIGH : LOW);

  // Khởi tạo giá trị ban đầu lên Blynk đúng chân Datastream
  Blynk.virtualWrite(V1, blueButtonON); // V1: led
  Blynk.virtualWrite(V0, 0);            // V0: time
  Serial.println("=== HỆ THỐNG SẴN SÀNG ===");
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();

  updateButton();
  uptimeTask();
  readDHT();
}

/* Hàm kiểm tra thời gian không dùng delay */
bool IsReady(unsigned long &timer, uint32_t interval) {
  if (currentMiliseconds - timer < interval) return false;
  timer = currentMiliseconds;
  return true;
}

/* 1. Xử lý nút nhấn vật lý trên Wokwi */
void updateButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(BTN_LED);

  if (v == lastValue) return;
  lastValue = v;

  if (v == LOW) { // Khi nhấn nút
    blueButtonON = !blueButtonON;
    digitalWrite(PIN_LED, blueButtonON ? HIGH : LOW);
    
    // Cập nhật trạng thái nút nhấn lên Blynk (V1)
    Blynk.virtualWrite(V1, blueButtonON);

    if (!blueButtonON) {
      display.clear();
    }
    Serial.println(blueButtonON ? "LED ON (Nút vật lý)" : "LED OFF (Nút vật lý)");
  }
}

/* 2. Xử lý thời gian hoạt động (V0: time) */
void uptimeTask() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  if (blueButtonON) {
    uptime++;
    display.showNumberDec(uptime);
    // Gửi lên Blynk chân V0
    Blynk.virtualWrite(V0, uptime);
  }
}

/* 3. Đọc cảm biến DHT22 và gửi lên Blynk (V2: tmep, V3: humi) */
void readDHT() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc cảm biến DHT22!");
    return;
  }

  // Gửi đúng chân theo Datastreams của bạn
  Blynk.virtualWrite(V2, temperature); // V2 khớp với 'tmep'
  Blynk.virtualWrite(V3, humidity);    // V3 khớp với 'humi'

  Serial.print("Nhiệt độ: "); Serial.print(temperature);
  Serial.print(" - Độ ẩm: "); Serial.println(humidity);
}

/* 4. Nhận lệnh điều khiển từ nút nhấn trên Blynk (V1: led) */
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(PIN_LED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON) {
    display.clear();
  }
  Serial.println(blueButtonON ? "Blynk điều khiển -> ON" : "Blynk điều khiển -> OFF");
}