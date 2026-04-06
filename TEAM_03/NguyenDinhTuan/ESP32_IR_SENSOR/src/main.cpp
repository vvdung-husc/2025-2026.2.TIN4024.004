#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

#define BOT_TOKEN "8606889770:AAFEyWTRWIDLCWFmKKjt3C34ylZiZMiGJ4k" // Thay Token Bot của bạn vào đây
#define CHAT_ID "-1003769844639"     // Thay Chat ID của bạn vào đây

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// --- THÔNG SỐ OLED ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- THÔNG SỐ CHÂN CẮM ---
#define PIR_IN_PIN 13   
#define PIR_OUT_PIN 14  
#define LED_PIN 12

// --- BIẾN TOÀN CỤC ---
int countIn = 0;
int countOut = 0;
int lastPirInState = LOW;
int lastPirOutState = LOW;

// ==========================================
// KHAI BÁO HÀM (FUNCTION PROTOTYPES) Ở ĐÂY
// Để PlatformIO biết các hàm này tồn tại
// ==========================================
void flashLed();
void updateOLED(String message, int in, int out);
void sendTelegram(bool isIn, int total);

void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_IN_PIN, INPUT);
  pinMode(PIR_OUT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // 1. Khởi tạo OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Khởi tạo SSD1306 thất bại!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();

  // 2. Kết nối WiFi
  WiFi.begin(ssid, password);
  secured_client.setInsecure(); 
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  
  // Thông báo khởi động thành công
  updateOLED("He thong san sang!", 0, 0);
  bot.sendMessage(CHAT_ID, "Hệ thống giám sát vào/ra đã khởi động!", "");
}

void loop() {
  int currentPirIn = digitalRead(PIR_IN_PIN);
  int currentPirOut = digitalRead(PIR_OUT_PIN);

  // XỬ LÝ SỰ KIỆN: NGƯỜI ĐI VÀO
  if (currentPirIn == HIGH && lastPirInState == LOW) {
    countIn++;
    String consoleMsg = "Phat hien co nguoi vao!";
    Serial.println(consoleMsg + " Tong Vao: " + String(countIn));
    
    flashLed();
    updateOLED(consoleMsg, countIn, countOut);
    sendTelegram(true, countIn); 
  }

  // XỬ LÝ SỰ KIỆN: NGƯỜI ĐI RA
  if (currentPirOut == HIGH && lastPirOutState == LOW) {
    countOut++;
    String consoleMsg = "Phat hien co nguoi ra!";
    Serial.println(consoleMsg + " Tong Ra: " + String(countOut));
    
    flashLed();
    updateOLED(consoleMsg, countIn, countOut);
    sendTelegram(false, countOut); 
  }

  lastPirInState = currentPirIn;
  lastPirOutState = currentPirOut;
  
  delay(50); 
}

// --- CÁC HÀM PHỤ TRỢ ---

void flashLed() {
  digitalWrite(LED_PIN, HIGH);
  delay(300); 
  digitalWrite(LED_PIN, LOW);
}

void updateOLED(String message, int in, int out) {
  display.clearDisplay(); 
  
  // Dòng trạng thái (Vào/Ra)
  display.setTextSize(1);     
  display.setCursor(0, 5);
  display.print(message);
  
  // Cột đếm số người VÀO
  display.setCursor(0, 25);
  display.print("Vao:");
  display.setTextSize(2);
  display.setCursor(0, 40);
  display.print(in);
  
  // Cột đếm số người RA
  display.setTextSize(1);
  display.setCursor(70, 25);
  display.print("Ra:");
  display.setTextSize(2);
  display.setCursor(70, 40);
  display.print(out);
  
  display.display(); 
}

void sendTelegram(bool isIn, int total) {
  String message = "";
  if (isIn) {
    message += "Phát hiện có người đi vào!\n";
    message += "Tổng người đã đi vào: ";
  } else {
    message += "Phát hiện có người đi ra!\n";
    message += "Tổng người đã đi ra: ";
  }
  message += String(total);
  
  bot.sendMessage(CHAT_ID, message, "");
}