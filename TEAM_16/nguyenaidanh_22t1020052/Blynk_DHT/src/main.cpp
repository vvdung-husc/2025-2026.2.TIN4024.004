
// THÔNG TIN BLYNK
#define BLYNK_TEMPLATE_ID "TMPL6fTxYyaB6"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "eKMpmMiS3pmDDrBU3TOoaMeVOH1DDEbh"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <TM1637Display.h>

// --- THÔNG TIN WIFI CỦA WOKWI ---
char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";            

// --- KHAI BÁO CHÂN DỰA TRÊN DIAGRAM.JSON ---
#define DHTPIN 16      // Chân SDA của DHT nối với 16
#define DHTTYPE DHT22  
#define CLK_PIN 18     // Chân CLK của TM1637 nối 18
#define DIO_PIN 19     // Chân DIO của TM1637 nối 19
#define LED_PIN 21     // Chân Đèn LED nối 21
#define BUTTON_PIN 23  // Chân Nút nhấn nối 23

// --- KHỞI TẠO ĐỐI TƯỢNG ---
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

// --- BIẾN TRẠNG THÁI ---
bool isCountdownMode = false;
int countdownValue = 0;
int uptimeSeconds = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;

// Hàm gửi dữ liệu Cảm biến
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
}

// Hàm xử lý màn hình và logic
void logicAndDisplay() {
  uptimeSeconds++; 
  
  if (isCountdownMode) {
    display.showNumberDec(countdownValue, false); 
    countdownValue--;
    
    if (countdownValue < 0) {
      isCountdownMode = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V3, 0); // Báo về Blynk tắt công tắc
    }
  } else {
    display.showNumberDec(uptimeSeconds, false);
    Blynk.virtualWrite(V2, uptimeSeconds);
  }
}

// Hàm kiểm tra nút nhấn ảo
void checkPhysicalButton() {
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) { 
    isCountdownMode = !isCountdownMode;
    if (isCountdownMode) {
      digitalWrite(LED_PIN, HIGH);
      countdownValue = 15; 
      Blynk.virtualWrite(V3, 1); 
    } else {
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V3, 0); 
    }
  }
  lastButtonState = buttonState;
}

// Hàm nhận lệnh từ công tắc trên Blynk
BLYNK_WRITE(V3) {
  int switchState = param.asInt(); 
  
  if (switchState == 1) {
    digitalWrite(LED_PIN, HIGH);
    isCountdownMode = true;
    countdownValue = 15; 
  } else {
    digitalWrite(LED_PIN, LOW);
    isCountdownMode = false;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  display.setBrightness(0x0f); 
  display.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData); 
  timer.setInterval(1000L, logicAndDisplay); 
  timer.setInterval(100L, checkPhysicalButton); 
}

void loop() {
  Blynk.run();
  timer.run();
}
