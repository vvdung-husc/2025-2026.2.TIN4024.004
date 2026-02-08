/* DANH SACH NHOM
1.Zoram Nho
2.Trương Đức Mai Linh
3.Phạm Hữu Anh Kiệt

*/
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Wire.h>

// --- CẤU HÌNH PHẦN CỨNG ---

// Cấu hình màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
// Định nghĩa chân I2C theo diagram của bạn
#define OLED_SDA 13
#define OLED_SCL 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Cấu hình DHT22
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Định nghĩa chân LED
const int LED_GREEN = 15; // Led Cyan trong diagram đóng vai trò là Green
const int LED_YELLOW = 2;
const int LED_RED = 4;

void setup() {
  Serial.begin(115200);

  // Khởi tạo I2C với chân Custom (SDA=13, SCL=12)
  Wire.begin(OLED_SDA, OLED_SCL);

  // Khởi tạo màn hình OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Dừng chương trình nếu lỗi màn hình
  }

  // Cấu hình hiển thị mặc định
  display.setTextColor(WHITE);
  display.clearDisplay();

  // Khởi tạo cảm biến và LED
  dht.begin();
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // 1. Đọc dữ liệu từ cảm biến
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Kiểm tra lỗi đọc cảm biến
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Sensor Error");
    display.display();
    return;
  }

  // 2. Xử lý Logic theo bảng yêu cầu
  String statusMsg = "";
  int activeLed = -1; // -1 nghĩa là không bật đèn nào

  if (t < 13) {
    statusMsg = "TOO COLD";
    activeLed = LED_GREEN;
  } 
  else if (t >= 13 && t < 20) {
    statusMsg = "COLD";
    activeLed = LED_GREEN;
  }
  else if (t >= 20 && t < 25) {
    statusMsg = "COOL";
    activeLed = LED_YELLOW;
  }
  else if (t >= 25 && t < 30) {
    statusMsg = "WARM";
    activeLed = LED_YELLOW;
  }
  else if (t >= 30 && t < 35) {
    statusMsg = "HOT";
    activeLed = LED_RED;
  }
  else { // t >= 35
    statusMsg = "TOO HOT";
    activeLed = LED_RED;
  }

  // 3. Hiển thị thông tin lên OLED
  display.clearDisplay();

  // Dòng 1: Nhiệt độ & Độ ẩm
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(t, 1); // Lấy 1 số lẻ
  display.print(" C");
  
  display.setCursor(0, 12);
  display.print("Humi: ");
  display.print(h, 1);
  display.print(" %");

  // Dòng 2: Trạng thái (được phóng to chữ)
  display.setTextSize(2);
  
  // Căn giữa chữ (tương đối) dựa trên độ dài
  int xPos = (128 - (statusMsg.length() * 12)) / 2; 
  if(xPos < 0) xPos = 0;
  
  display.setCursor(xPos, 35);
  display.print(statusMsg);
  
  display.display();

  // 4. Điều khiển LED nhấp nháy
  // Tắt tất cả trước
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // Nhấp nháy LED đang kích hoạt
  if (activeLed != -1) {
    digitalWrite(activeLed, HIGH);
    delay(500); // Sáng 0.5s
    digitalWrite(activeLed, LOW);
    delay(500); // Tắt 0.5s
  } else {
    delay(1000); // Nếu không có led nào (trường hợp dự phòng), đợi 1s
  }
}