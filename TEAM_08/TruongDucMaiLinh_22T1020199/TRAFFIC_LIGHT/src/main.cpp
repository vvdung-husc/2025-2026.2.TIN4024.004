#include <Arduino.h>
#include <TM1637Display.h>

// --- 1. CẤU HÌNH CHÂN (Cập nhật theo diagram.json mới nhất) ---
#define LED_RED     27   // Đèn Đỏ nối chân 27
#define LED_YELLOW  26   // Đèn Vàng nối chân 26
#define LED_GREEN   25   // Đèn Xanh nối chân 25

#define LED_BLUE    21   // Đèn báo hiệu nối chân 21
#define BUTTON_PIN  23   // Nút bấm nối chân 23
#define LDR_PIN     13   // Cảm biến ánh sáng nối chân 13

// Cấu hình Màn hình 4 số
#define CLK_PIN     18 
#define DIO_PIN     19

// Khởi tạo đối tượng màn hình
TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  Serial.begin(115200);

  // Cấu hình các chân đèn là Output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Cấu hình nút bấm (Quan trọng: Phải dùng INPUT_PULLUP vì sơ đồ nối xuống GND)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Cấu hình màn hình
  display.setBrightness(7); // Độ sáng cao nhất
  display.showNumberDec(8888); // Test màn hình lúc khởi động
  delay(1000);
  display.clear();
}

// Hàm đếm ngược tích hợp kiểm tra nút bấm và cảm biến
void trafficDelay(int seconds, int currentLedPin) {
  for (int i = seconds; i > 0; i--) {
    // 1. Hiển thị số giây còn lại lên màn hình LED 7 thanh
    display.showNumberDec(i);
    
    // 2. In thông báo ra Serial Monitor
    Serial.print("LED Pin ["); Serial.print(currentLedPin);
    Serial.print("] ON - Time left: "); Serial.println(i);

    // 3. Kiểm tra nút bấm
    // Vì dùng INPUT_PULLUP, khi bấm nút giá trị sẽ là LOW (0)
    if (digitalRead(BUTTON_PIN) == LOW) { 
        digitalWrite(LED_BLUE, HIGH); // Bật đèn xanh dương cảnh báo
        Serial.println("Warning: Button is pressed!");
    } else {
        digitalWrite(LED_BLUE, LOW);  // Tắt đèn xanh dương
    }

    // 4. Kiểm tra cảm biến ánh sáng
    int lightValue = analogRead(LDR_PIN);
    // Serial.print("Light Sensor: "); Serial.println(lightValue); // Bỏ comment nếu muốn xem giá trị

    // 5. Chờ 1 giây (Đây chính là delay thầy yêu cầu)
    delay(1000); 
  }
  // Xóa màn hình khi hết giờ
  display.clear();
}

void loop() {
  // --- PHA 1: ĐÈN ĐỎ (5 Giây) ---
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  trafficDelay(5, LED_RED);

  // --- PHA 2: ĐÈN VÀNG (3 Giây) ---
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, LOW);
  trafficDelay(3, LED_YELLOW);

  // --- PHA 3: ĐÈN XANH (7 Giây) ---
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  trafficDelay(7, LED_GREEN);
}