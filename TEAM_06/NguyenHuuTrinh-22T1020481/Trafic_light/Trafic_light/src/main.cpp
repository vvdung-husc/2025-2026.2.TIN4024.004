#include <Arduino.h>

// --- 1. CẤU HÌNH CHÂN ---
#define RED_LED 23
#define YELLOW_LED 22
#define GREEN_LED 21
#define STREET_LED 5    
#define BTN_PIN 27      
#define LDR_PIN 34      

#define CLK 18
#define DIO 19

// --- 2. CÁC BIẾN QUẢN LÝ ---
enum TrafficState { STATE_RED, STATE_GREEN, STATE_YELLOW };
TrafficState currentState = STATE_RED;

const long TIME_RED = 5000;
const long TIME_GREEN = 5000;
const long TIME_YELLOW = 3000;

unsigned long previousMillis = 0; 
long currentInterval = TIME_RED;  

bool isDisplayOn = true;         
bool lastBtnState = HIGH;       
bool lastStreetLightState = false;
int lastShownSecond = -1;

// --- 3. PHẦN TỰ VIẾT ĐỂ THAY THẾ THƯ VIỆN TM1637 ---

// Mã Hex hiển thị số 0-9 trên LED 7 thanh
const uint8_t SEG_MAP[] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F  // 9
};

// Hàm gửi tín hiệu Start
void tm1637Start() {
  digitalWrite(DIO, LOW);
  delayMicroseconds(2);
  digitalWrite(CLK, LOW);
}

// Hàm gửi tín hiệu Stop
void tm1637Stop() {
  digitalWrite(DIO, LOW);
  digitalWrite(CLK, LOW);
  delayMicroseconds(2);
  digitalWrite(CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(DIO, HIGH);
}

// Hàm gửi 1 byte dữ liệu
void tm1637WriteByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    if (b & 0x01) digitalWrite(DIO, HIGH);
    else digitalWrite(DIO, LOW);
    delayMicroseconds(3);
    b >>= 1;
    digitalWrite(CLK, HIGH);
    delayMicroseconds(3);
  }
  
  // Đợi phản hồi ACK (Clock thứ 9)
  digitalWrite(CLK, LOW);
  digitalWrite(DIO, LOW); // Kéo DIO xuống để chờ ACK
  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
}

// Hàm hiển thị số (Thay thế display.showNumberDec)
// Chỉ hiển thị số ở hàng đơn vị và chục (vì đèn giao thông < 99s)
void showNumberManual(int num) {
  // 1. Gửi lệnh cài đặt dữ liệu (Data Command)
  tm1637Start();
  tm1637WriteByte(0x40); // 0x40: Chế độ ghi dữ liệu
  tm1637Stop();

  // 2. Gửi dữ liệu hiển thị vào địa chỉ C0 (LED đầu tiên)
  tm1637Start();
  tm1637WriteByte(0xC0); // 0xC0: Địa chỉ bắt đầu

  // Xử lý hiển thị (để giống thư viện, ta tắt 2 led đầu, hiện số ở 2 led cuối)
  tm1637WriteByte(0x00); // LED 1: Tắt
  tm1637WriteByte(0x00); // LED 2: Tắt
  
  if (num > 9) {
    tm1637WriteByte(SEG_MAP[num / 10]); // LED 3: Hàng chục
  } else {
    tm1637WriteByte(0x00); // Tắt nếu không có hàng chục
  }
  
  tm1637WriteByte(SEG_MAP[num % 10]); // LED 4: Hàng đơn vị
  tm1637Stop();

  // 3. Gửi lệnh điều khiển hiển thị (Display Control) - Bật màn hình + Độ sáng
  tm1637Start();
  tm1637WriteByte(0x88 + 7); // 0x88 là bật, +7 là độ sáng max
  tm1637Stop();
}

// Hàm xóa màn hình (Thay thế display.clear)
void clearDisplayManual() {
  tm1637Start();
  tm1637WriteByte(0x40);
  tm1637Stop();

  tm1637Start();
  tm1637WriteByte(0xC0);
  for(int i=0; i<4; i++) tm1637WriteByte(0x00); // Gửi 0x00 để tắt hết
  tm1637Stop();

  tm1637Start();
  tm1637WriteByte(0x88 + 7); 
  tm1637Stop();
}

// --- 4. SETUP & LOOP ---

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(STREET_LED, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  // Cấu hình chân cho màn hình (Quan trọng khi không dùng thư viện)
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);

  Serial.println(">>> DA FIX LOI CHAM + KHONG THU VIEN <<<");
  Serial.println("🔴 DEN DO: DUNG LAI (5s)");
  digitalWrite(RED_LED, HIGH);
  
  // Hiển thị số ban đầu
  showNumberManual(TIME_RED/1000);
}

void turnOffTrafficLeds() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- NHIỆM VỤ A: ĐÈN ĐƯỜNG ---
  static unsigned long lastLdrCheck = 0;
  if (currentMillis - lastLdrCheck > 200) { 
    lastLdrCheck = currentMillis;
    int lightValue = analogRead(LDR_PIN);
    bool isDark = (lightValue > 2000); 

    if (isDark != lastStreetLightState) {
      if (isDark) {
        digitalWrite(STREET_LED, HIGH);
        Serial.println("🌌 TROI TOI: Bat den duong");
      } else {
        digitalWrite(STREET_LED, LOW);
        Serial.println("☀️ TROI SANG: Tat den duong");
      }
      lastStreetLightState = isDark;
    }
  }

  // --- NHIỆM VỤ B: NÚT BẤM ---
  int btnState = digitalRead(BTN_PIN);
  if (btnState == LOW && lastBtnState == HIGH) {
    isDisplayOn = !isDisplayOn; 
    
    if (isDisplayOn) {
      Serial.println("📺 MAN HINH: ON");
      lastShownSecond = -1; 
    } else {
      clearDisplayManual(); // Dùng hàm tự viết
      Serial.println("📴 MAN HINH: OFF");
    }
    delay(50); 
  }
  lastBtnState = btnState;

  // --- NHIỆM VỤ C: ĐÈN GIAO THÔNG ---
  unsigned long timePassed = currentMillis - previousMillis;
  long remainingSeconds = (currentInterval - timePassed) / 1000;
  if (remainingSeconds < 0) remainingSeconds = 0;
  
  if (isDisplayOn) {
    if (remainingSeconds != lastShownSecond) {
      // Gọi hàm tự viết thay vì thư viện
      showNumberManual(remainingSeconds);
      lastShownSecond = remainingSeconds; 
    }
  }

  // Kiểm tra chuyển đèn
  if (timePassed >= currentInterval) {
    previousMillis = currentMillis;
    turnOffTrafficLeds();
    lastShownSecond = -1; 

    switch (currentState) {
      case STATE_RED:
        currentState = STATE_GREEN;
        digitalWrite(GREEN_LED, HIGH);
        currentInterval = TIME_GREEN;
        Serial.println("🟢 DEN XANH (5s)");
        break;

      case STATE_GREEN:
        currentState = STATE_YELLOW;
        digitalWrite(YELLOW_LED, HIGH);
        currentInterval = TIME_YELLOW;
        Serial.println("🟡 DEN VANG (3s)");
        break;

      case STATE_YELLOW:
        currentState = STATE_RED;
        digitalWrite(RED_LED, HIGH);
        currentInterval = TIME_RED;
        Serial.println("🔴 DEN DO (5s)");
        break;
    }
  }
}