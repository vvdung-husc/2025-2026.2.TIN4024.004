#define BLYNK_TEMPLATE_ID "TMPL6nbN7KgOW"
#define BLYNK_TEMPLATE_NAME "BME280"
#define BLYNK_AUTH_TOKEN "5hXUF9fQfJUWPFkBQapuOOhABrnBA8ob"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

// Cấu hình chân phần cứng theo diagram.json của bạn
#define BUTTON_PIN 13  // Nút nhấn vật lý
#define LED_PIN    2   // Đèn LED trên mạch (Magenta)
#define DHTPIN     15  // DHT22
#define DHTTYPE    DHT22

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool oledState = true;

// 1. HÀM ĐIỀU KHIỂN TỪ NÚT NHẤN TRÊN WEB (V5)
BLYNK_WRITE(V5) {
  oledState = param.asInt();
  Serial.print("Web Dashboard: ");
  Serial.println(oledState ? "BAT He thong" : "TAT He thong");
}

// Hàm ngắt nút bấm vật lý (GPIO 13)
void IRAM_ATTR toggleOLED() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 250) { 
    oledState = !oledState;
  }
  lastTime = millis();
}

void updateSystem() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure() / 100.0F;

  if (isnan(h) || isnan(t)) {
    Serial.println("Loi: Khong doc du được cam bien!");
    return;
  }

  // 2. IN THÔNG BÁO RA TERMINAL (Để phần bên dưới nhảy chữ)
  Serial.printf("Data -> T: %.1f C | H: %.1f %% | P: %.1f hPa | Status: %s\n", 
                t, h, p, oledState ? "ON" : "OFF");

  // 3. ĐẨY DỮ LIỆU CẢM BIẾN LÊN BLYNK
  Blynk.virtualWrite(V1, t); 
  Blynk.virtualWrite(V2, h); 
  Blynk.virtualWrite(V3, p); 

  // 4. ĐỒNG BỘ TRẠNG THÁI (Màn hình, LED vật lý, LED Web, Nút Web)
  if (oledState) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("TIN'S STATION");
    display.printf("Nhiet: %.1f C\n", t);
    display.printf("Do am: %.1f %%\n", h);
    display.printf("Ap suat: %.1f hPa", p);
    display.display();
    
    digitalWrite(LED_PIN, HIGH); 
    Blynk.virtualWrite(V4, 255); // Sáng LED trên Web
    Blynk.virtualWrite(V5, 1);   // Gạt nút trên Web sang ON
  } else {
    display.clearDisplay();
    display.display(); // Tắt màn hình
    
    digitalWrite(LED_PIN, LOW); 
    Blynk.virtualWrite(V4, 0);   // Tắt LED trên Web
    Blynk.virtualWrite(V5, 0);   // Gạt nút trên Web sang OFF
  }
}

void setup() {
  // 5. KHỞI TẠO SERIAL ĐỂ HIỆN THÔNG BÁO TERMINAL
  Serial.begin(115200);
  Serial.println("--- DANG KHOI DONG ---");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(BUTTON_PIN, toggleOLED, FALLING);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Loi OLED!");
  }
  dht.begin();
  if(!bmp.begin()) {
    Serial.println("Loi BMP180!");
  }

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  
  timer.setInterval(2000L, updateSystem);
  Serial.println("--- DA SAN SANG ---");
}

void loop() {
  Blynk.run();
  timer.run();
}