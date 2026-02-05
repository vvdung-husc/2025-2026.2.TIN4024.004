/*
THÔNG TIN NHÓM
1. Phan Hữu Tuấn Kiệt
2. Nguyễn Đại Quý
3.
4.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- 1. KHAI BÁO CHÂN (PIN DEFINITIONS) ---
#define DHTPIN 27       // Chân DHT (Kết nối vào chân 27)
#define LED_GREEN 18    // Đèn Xanh
#define LED_YELLOW 19   // Đèn Vàng
#define LED_RED 5       // Đèn Đỏ

// --- 2. CẤU HÌNH THIẾT BỊ ---
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- 3. BIẾN QUẢN LÝ THỜI GIAN (NON-BLOCKING) ---
unsigned long previousSensorMillis = 0; 
const long sensorInterval = 2000;       // Đọc mỗi 2 giây

unsigned long previousBlinkMillis = 0;  
const long blinkInterval = 500;         // Nháy mỗi 0.5 giây

// --- 4. BIẾN TRẠNG THÁI ---
bool ledState = LOW;    
int activeLED = -1;     
String statusMsg = "";  
float temp = 0;
float humi = 0;

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  delay(100); 
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED Error"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Khoi tao...");
  display.display();
}

void loop() {
  unsigned long currentMillis = millis(); 

  // ============================================================
  // TÁC VỤ 1: ĐỌC CẢM BIẾN & CẬP NHẬT MÀN HÌNH
  // ============================================================
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis; 

    humi = dht.readHumidity();
    temp = dht.readTemperature();

    if (isnan(humi) || isnan(temp)) {
      statusMsg = "ERR";
      activeLED = -1;
    } else {
      // Logic phân loại nhiệt độ theo yêu cầu
      if (temp < 13) {
        statusMsg = "TOO COLD";
        activeLED = LED_GREEN;
      } else if (temp >= 13 && temp < 20) {
        statusMsg = "COLD";
        activeLED = LED_GREEN;
      } else if (temp >= 20 && temp < 25) {
        statusMsg = "COOL";
        activeLED = LED_YELLOW;
      } else if (temp >= 25 && temp < 30) {
        statusMsg = "WARM";
        activeLED = LED_YELLOW;
      } else if (temp >= 30 && temp <= 35) {
        statusMsg = "HOT";
        activeLED = LED_RED;
      } else {
        statusMsg = "TOO HOT";
        activeLED = LED_RED;
      }
    }

    // --- CẬP NHẬT GIAO DIỆN GIỐNG HÌNH MẪU ---
    display.clearDisplay();
    
    // 1. Dòng tiêu đề nhiệt độ + Trạng thái (VD: Temperature: HOT)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Temp: "); 
    display.println(statusMsg); 

    // 2. Giá trị Nhiệt độ (Chữ to)
    display.setTextSize(2);
    display.setCursor(0, 12); // Cách lề trên 12px
    display.print(temp, 1);   // Lấy 1 số thập phân (VD: 53.7)
    display.cp437(true); 
    display.write(167);       // Ký tự độ (°)
    display.println("C");

    // 3. Dòng tiêu đề độ ẩm
    display.setTextSize(1);
    display.setCursor(0, 35); // Cách lề trên 35px để tạo khoảng cách
    display.println("Humidity:");

    // 4. Giá trị Độ ẩm (Chữ to)
    display.setTextSize(2);
    display.setCursor(0, 47); // Cách lề trên 47px
    display.print(humi, 1);
    display.println(" %");

    display.display();
  }

  // ============================================================
  // TÁC VỤ 2: ĐÈN NHẤP NHÁY
  // ============================================================
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis; 
    
    ledState = !ledState;
    
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);

    if (activeLED != -1) {
      digitalWrite(activeLED, ledState);
    }
  }
}
