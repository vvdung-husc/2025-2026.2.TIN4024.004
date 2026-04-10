#define BLYNK_TEMPLATE_ID "TMPL6W7mhrWfO"
#define BLYNK_TEMPLATE_NAME "ESP32 MAIN"
#define BLYNK_AUTH_TOKEN "4AqNCPHAnbeJDDQa88XGD2ZM6q7SD1qg"


#include <WiFi.h>
#include <BlynkSimpleEsp32.h> 

// Chân điều khiển LED theo sơ đồ
const int LED_PIN = 15;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; 

// Hàm này chạy mỗi khi trạng thái nút nhấn trên Blynk Web/App thay đổi
BLYNK_WRITE(V1) { 
  int ledState = param.asInt(); // Nhận giá trị 0 hoặc 1 từ Widget
  digitalWrite(LED_PIN, ledState);
  
  Serial.print("Blynk nhận lệnh: ");
  Serial.println(ledState ? "BẬT ĐÈN" : "TẮT ĐÈN");
}

// Hàm đồng bộ trạng thái khi ESP32 vừa kết nối mạng
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  
  // Khởi tạo kết nối Blynk
  Blynk.begin(auth, ssid, pass);
  
  Serial.println("Đang đợi lệnh từ Blynk...");
}

void loop() {
  Blynk.run();
} 