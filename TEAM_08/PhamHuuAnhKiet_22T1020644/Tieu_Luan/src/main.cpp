// --- DÁN 3 DÒNG MÃ BLYNK CỦA BẠN VÀO ĐÂY ---
#define BLYNK_TEMPLATE_ID "TMPL69EbeMR1U"
#define BLYNK_TEMPLATE_NAME "Bao Dong Gas"
#define BLYNK_AUTH_TOKEN "Jscxx0E6CB7bsvB4QOj3NGrShwg4lWdS"

// ------------------------------------------

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Thông tin WiFi của môi trường Wokwi (Không cần đổi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa các chân kết nối (Khớp với file diagram.json)
#define BUZZER_PIN 32
#define LED_PIN 33
#define POT_GAS_PIN 34 // Chân đọc chiết áp mô phỏng khí Gas

// Ngưỡng báo động giả định (Từ 0 - 4095)
const int GAS_THRESHOLD = 2500; 

BlynkTimer timer;

// Hàm đọc dữ liệu gas mô phỏng và kiểm tra cảnh báo
void checkGasLevel() {
  // Đọc giá trị từ chiết áp (Thay thế cho cảm biến MQ-6)
  int gasValue = analogRead(POT_GAS_PIN);
  
  // Gửi dữ liệu lên Blynk (Datastream V0) để hiển thị lên Gauge/Web
  Blynk.virtualWrite(V0, gasValue);

  // In ra Serial Monitor để dễ debug trên VS Code
  Serial.print("Mức Gas hiện tại: ");
  Serial.println(gasValue);

  // Xử lý logic báo động
  if (gasValue > GAS_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH); // Bật LED đỏ
    tone(BUZZER_PIN, 1000);      // Bật còi với tần số 1000Hz
    
    Serial.println(">>> NGUY HIỂM: Rò rỉ khí Gas! <<<");
    
    // Gửi Event cảnh báo về điện thoại (Tên "gas_alert" phải khớp với trên Web)
    Blynk.logEvent("gas_alert", "CẢNH BÁO: Phát hiện rò rỉ khí Gas ở mức nguy hiểm!");
  } else {
    digitalWrite(LED_PIN, LOW);  // Tắt LED
    noTone(BUZZER_PIN);          // Tắt còi
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Đang khởi động hệ thống...");

  // Thiết lập chế độ cho các chân GPIO
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(POT_GAS_PIN, INPUT);

  // Kết nối với Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Hẹn giờ chạy hàm checkGasLevel mỗi 2 giây (Tránh gửi dữ liệu quá nhanh làm treo Blynk)
  timer.setInterval(2000L, checkGasLevel);
}

void loop() {
  Blynk.run();
  timer.run();
}