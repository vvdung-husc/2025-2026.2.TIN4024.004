#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Định nghĩa kích thước màn hình OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Định nghĩa chân cắm theo diagram.json
#define PIR_PIN 13
#define LED_PIN 12

int count = 0;              // Biến đếm số người
int lastPirState = LOW;     // Trạng thái trước đó của cảm biến

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo chân I/O
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Khởi tạo màn hình OLED với địa chỉ I2C mặc định là 0x3C
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Khởi tạo SSD1306 thất bại!"));
    for(;;); // Dừng chương trình nếu lỗi OLED
  }

  // Hiển thị trạng thái ban đầu (chưa có ai)
  updateOLED();
}

void loop() {
  // Đọc trạng thái hiện tại của cảm biến chuyển động
  int currentPirState = digitalRead(PIR_PIN);

  // Kiểm tra: Nếu lúc trước không có người (LOW) mà bây giờ có người (HIGH)
  if (currentPirState == HIGH && lastPirState == LOW) {
    count++; // Tăng biến đếm lên 1
    
    // Bật đèn LED 1 phát
    digitalWrite(LED_PIN, HIGH);
    
    // Cập nhật màn hình OLED với số đếm mới
    updateOLED();
    
    // Giữ đèn sáng trong 0.5 giây rồi tắt để tạo hiệu ứng "nháy"
    delay(500); 
    digitalWrite(LED_PIN, LOW);
  }

  // Lưu lại trạng thái hiện tại để so sánh cho vòng lặp tiếp theo
  lastPirState = currentPirState;
  
  delay(50); // Delay nhỏ để ổn định vòng lặp
}

// Hàm phụ trợ để cập nhật màn hình OLED cho gọn code
void updateOLED() {
  display.clearDisplay(); // Xóa màn hình cũ
  
  display.setTextSize(1);      // Kích cỡ chữ
  display.setTextColor(WHITE); // Màu chữ (trắng trên nền đen)
  
  // Dòng 1: Phat hien X nguoi!
  display.setCursor(0, 15);
  display.print("Phat hien ");
  display.print(count);
  display.print(" nguoi!");
  
  // Dòng 2: Count: X
  display.setTextSize(2); // In to hơn một chút cho dòng Count
  display.setCursor(0, 35);
  display.print("Count: ");
  display.print(count);
  
  display.display(); // Lệnh này bắt buộc phải có để đẩy dữ liệu ra màn hình
}