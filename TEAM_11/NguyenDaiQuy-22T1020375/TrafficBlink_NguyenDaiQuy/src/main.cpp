#include <Arduino.h>

// --- 1. CẤU HÌNH CHÂN ---
#define RED_LED 23
#define YELLOW_LED 22
#define GREEN_LED 21
#define STREET_LED 5     // Đèn đường (Blue)
#define BTN_PIN 27       // Nút bấm
#define LDR_PIN 34       // Cảm biến ánh sáng

#define CLK 18
#define DIO 19

// --- 2. CÁC THAM SỐ THỜI GIAN ---
const long TIME_RED = 5000;
const long TIME_GREEN = 5000;
const long TIME_YELLOW = 3000;

// --- 3. BIẾN TOÀN CỤC ---
enum TrafficState { STATE_RED, STATE_GREEN, STATE_YELLOW };
TrafficState currentState = STATE_RED;

unsigned long previousMillis = 0; 
long currentInterval = TIME_RED;
bool isDayMode = true;          // Trạng thái Ngày/Đêm
bool lastStreetLightState = false;

// Biến cho chế độ đèn vàng nhấp nháy (Ban đêm)
unsigned long blinkMillis = 0;
bool yellowState = false;

// --- 4. HÀM ĐIỀU KHIỂN LED 7 THANH (KHÔNG DÙNG THƯ VIỆN) ---
const uint8_t SEG_MAP[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };

void tm1637Start() {
  digitalWrite(DIO, LOW); delayMicroseconds(2);
  digitalWrite(CLK, LOW);
}

void tm1637Stop() {
  digitalWrite(DIO, LOW); digitalWrite(CLK, LOW); delayMicroseconds(2);
  digitalWrite(CLK, HIGH); delayMicroseconds(2);
  digitalWrite(DIO, HIGH);
}

void tm1637WriteByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIO, (b & 0x01) ? HIGH : LOW);
    delayMicroseconds(3);
    b >>= 1;
    digitalWrite(CLK, HIGH); delayMicroseconds(3);
  }
  digitalWrite(CLK, LOW); digitalWrite(DIO, LOW); digitalWrite(CLK, HIGH); digitalWrite(CLK, LOW);
}

void showNumberManual(int num) {
  tm1637Start(); tm1637WriteByte(0x40); tm1637Stop();
  tm1637Start(); tm1637WriteByte(0xC0);
  tm1637WriteByte(0x00); tm1637WriteByte(0x00); // Tắt 2 led đầu
  tm1637WriteByte((num > 9) ? SEG_MAP[num / 10] : 0x00);
  tm1637WriteByte(SEG_MAP[num % 10]);
  tm1637Stop();
  tm1637Start(); tm1637WriteByte(0x88 + 7); tm1637Stop();
}

// Hàm hiển thị dấu gạch ngang (cho chế độ chờ/ban đêm)
void showDashes() {
  tm1637Start(); tm1637WriteByte(0x40); tm1637Stop();
  tm1637Start(); tm1637WriteByte(0xC0);
  for(int i=0; i<4; i++) tm1637WriteByte(0x40); // 0x40 là dấu gạch ngang (-)
  tm1637Stop();
  tm1637Start(); tm1637WriteByte(0x88 + 7); tm1637Stop();
}

void clearDisplay() {
  tm1637Start(); tm1637WriteByte(0x40); tm1637Stop();
  tm1637Start(); tm1637WriteByte(0xC0);
  for(int i=0; i<4; i++) tm1637WriteByte(0x00);
  tm1637Stop();
  tm1637Start(); tm1637WriteByte(0x88 + 7); tm1637Stop();
}

// --- 5. LOGIC CHÍNH ---

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT); pinMode(YELLOW_LED, OUTPUT); pinMode(GREEN_LED, OUTPUT);
  pinMode(STREET_LED, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP); pinMode(LDR_PIN, INPUT);
  
  pinMode(CLK, OUTPUT); pinMode(DIO, OUTPUT);
  digitalWrite(CLK, HIGH); digitalWrite(DIO, HIGH);

  Serial.println(">>> HE THONG GIAO THONG THONG MINH (DAY/NIGHT) <<<");
}

void turnOffAllTrafficLeds() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. ĐỌC CẢM BIẾN LDR
  int lightValue = analogRead(LDR_PIN);
  bool isDark = (lightValue > 2000); // Ngưỡng trời tối

  // Phát hiện chuyển đổi trạng thái Ngày/Đêm
  if (isDark != !isDayMode) {
    isDayMode = !isDark;
    if (isDayMode) {
      Serial.println("☀️ CHUYEN SANG CHE DO NGAY: Den hoat dong binh thuong");
      digitalWrite(STREET_LED, LOW); // Tắt đèn đường
      // Reset về đèn đỏ khi bắt đầu ngày mới
      currentState = STATE_RED;
      previousMillis = currentMillis;
      currentInterval = TIME_RED;
      digitalWrite(RED_LED, HIGH);
    } else {
      Serial.println("🌙 CHUYEN SANG CHE DO DEM: Den vang nhap nhay");
      digitalWrite(STREET_LED, HIGH); // Bật đèn đường (Blue LED)
      turnOffAllTrafficLeds(); // Tắt hết đèn giao thông để bắt đầu nhấp nháy
      showDashes(); // Hiển thị gạch ngang trên màn hình
    }
    delay(500); // Debounce nhẹ cho cảm biến đỡ nhảy qua lạig
  }

  // 2. XỬ LÝ LOGIC THEO CHẾ ĐỘ
  if (isDayMode) {
    // --- CHẾ ĐỘ NGÀY: CHẠY BÌNH THƯỜNG ---
    unsigned long timePassed = currentMillis - previousMillis;
    long remainingSeconds = (currentInterval - timePassed) / 1000;
    
    // Hiển thị số
    static int lastSec = -1;
    if (remainingSeconds != lastSec && remainingSeconds >= 0) {
      showNumberManual(remainingSeconds);
      lastSec = remainingSeconds;
    }

    // Chuyển đèn
    if (timePassed >= currentInterval) {
      previousMillis = currentMillis;
      turnOffAllTrafficLeds();
      lastSec = -1; 

      switch (currentState) {
        case STATE_RED:
          currentState = STATE_GREEN;
          digitalWrite(GREEN_LED, HIGH);
          currentInterval = TIME_GREEN;
          Serial.println("🟢 DEN XANH");
          break;
        case STATE_GREEN:
          currentState = STATE_YELLOW;
          digitalWrite(YELLOW_LED, HIGH);
          currentInterval = TIME_YELLOW;
          Serial.println("🟡 DEN VANG");
          break;
        case STATE_YELLOW:
          currentState = STATE_RED;
          digitalWrite(RED_LED, HIGH);
          currentInterval = TIME_RED;
          Serial.println("🔴 DEN DO");
          break;
      }
    }

  } else {
    // --- CHẾ ĐỘ ĐÊM: ĐÈN VÀNG NHẤP NHÁY ---
    // Nhấp nháy mỗi 1 giây (1000ms)
    if (currentMillis - blinkMillis >= 1000) {
      blinkMillis = currentMillis;
      yellowState = !yellowState;
      digitalWrite(YELLOW_LED, yellowState ? HIGH : LOW);
      
      // Đảm bảo Đỏ và Xanh luôn tắt
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
    }
  }
}