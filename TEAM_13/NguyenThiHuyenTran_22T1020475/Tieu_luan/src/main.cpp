/************ BLYNK ************/
#define BLYNK_TEMPLATE_ID "TMPL6AQ-JzTuo"
#define BLYNK_TEMPLATE_NAME "Tưới cây tự động"
#define BLYNK_AUTH_TOKEN "0rS6iYsscKBunkLhuLds8LQU23gV9Zqe"

/************ THƯ VIỆN ************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

/************ WIFI ************/
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/************ DATASTREAM ************/
#define SOIL_VPIN V0  // độ ẩm đất
#define TEMP_VPIN V1  // nhiệt độ
#define HUM_VPIN  V2  // độ ẩm không khí
#define RELAY_VPIN V3 //  nút điều khiển
#define LED_VPIN  V4   // LED trạng thái bơm

/************ CHÂN ************/
#define DHTPIN 4
#define DHTTYPE DHT22

#define SOIL_PIN 34
#define RELAY_PIN 26

/************ KHỞI TẠO ************/
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

bool relay_state = false;
//bool daCanhBao = false; // chống spam cảnh báo
int lastMoisture = -1;

void checkConnection() {
  Serial.println("\n🌐 ===== TRẠNG THÁI KẾT NỐI =====");

  Serial.print(" 📶 WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Đã kết nối" : "Mất kết nối");

  Serial.print(" ☁️ Blynk: ");
  Serial.println(Blynk.connected() ? "Đã kết nối" : "Mất kết nối");

  Serial.println("=================================");
}

/************ HÀM GỬI DỮ LIỆU ************/
void sendData() {
  int sensor_analog = analogRead(SOIL_PIN);

  // ===== TÍNH % ĐỘ ẨM =====
  int moisture = map(sensor_analog, 0, 4095, 0, 100);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // ===== SERIAL =====
  Serial.println("\n===================================");
  Serial.println("   🌱 HỆ THỐNG TƯỚI CÂY TỰ ĐỘNG 🌱   ");
  Serial.println("-----------------------------------");

  Serial.print(" 🪴 Độ ẩm đất    : "); 
  Serial.print(moisture); 
  Serial.println(" %");
  
  Serial.print(" 🌡️ Nhiệt độ không khí  : "); 
  Serial.print(temp); 
  Serial.println(" °C");
  
  Serial.print(" ☁️ Độ ẩm không khí     : "); 
  Serial.print(hum);
  Serial.println(" %");
  Serial.println("===================================");
  // ===== GỬI LÊN BLYNK =====
  Blynk.virtualWrite(SOIL_VPIN, moisture);
  Blynk.virtualWrite(TEMP_VPIN, temp);
  Blynk.virtualWrite(HUM_VPIN, hum);

  // ===== ĐIỀU KHIỂN TỰ ĐỘNG =====
  if (moisture < 30 && relay_state == false) {
    digitalWrite(RELAY_PIN, HIGH);
    relay_state = true;

    Serial.println(" ⚠️ CẢNH BÁO: Đất quá khô!");
    Serial.println(" 💦 TRẠNG THÁI: Đang bật máy bơm...");

    // 🔔 gửi cảnh báo 1 lần
    if (moisture < 30 && moisture != lastMoisture) {
      Blynk.logEvent("warning", "⚠️ WARNING: Đất khô! Hệ thống đang tưới...");
      //daCanhBao = true;
       lastMoisture = moisture;
    }
  }

  else if (moisture >= 60 && relay_state == true) {
    digitalWrite(RELAY_PIN, LOW);
    relay_state = false;

    Serial.println(" ✅ THÔNG BÁO: Đất đã đủ độ ẩm.");
    Serial.println(" 🛑 TRẠNG THÁI: Đã ngắt máy bơm.");

    //daCanhBao = false; // reset cảnh báo
  }

  // ===== ĐỒNG BỘ APP =====
  Blynk.virtualWrite(RELAY_VPIN, relay_state); // switch
  Blynk.virtualWrite(LED_VPIN, relay_state);   // LED
}

/************ ĐIỀU KHIỂN TỪ APP ************/
BLYNK_WRITE(RELAY_VPIN) {
  int value = param.asInt();

  relay_state = value;

  if (relay_state) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("\n 📱 APP: Người dùng BẬT bơm!");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("\n 📱 APP: Người dùng TẮT bơm!");
  }

  // đồng bộ LED
  Blynk.virtualWrite(LED_VPIN, relay_state);
}

/************ SETUP ************/
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendData);
  checkConnection();
}

/************ LOOP ************/
void loop() {
  Blynk.run();
  timer.run();
}