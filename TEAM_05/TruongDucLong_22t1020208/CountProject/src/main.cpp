// ==================== QUAN TRỌNG: ĐỊNH NGHĨA BLYNK TRƯỚC KHI INCLUDE ====================
#define BLYNK_TEMPLATE_ID "TMPL6YF6AHar-"
#define BLYNK_TEMPLATE_NAME "esp32"
#define BLYNK_AUTH_TOKEN "wnSpEDjsLUmPSZHOfcL2PTDdyMznL6Sw"

// ==================== INCLUDE THƯ VIỆN ====================
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <BlynkSimpleEsp32.h>

// ==================== CẤU HÌNH MÀN HÌNH OLED ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==================== CẤU HÌNH WIFI ====================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ==================== CẤU HÌNH TELEGRAM ====================
#define BOT_TOKEN "8074033429:AAGCv4_uo4y8rwiYlqg1MdYv5gA_7qCKLO0"
#define CHAT_ID "2024544647"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ==================== ĐỊNH NGHĨA CHÂN ====================
#define LED_PIN 2

// ==================== BIẾN TOÀN CỤC ====================
int peopleCount = 0;
bool systemEnabled = true;
unsigned long lastAutoIncrementTime = 0;
unsigned long lastSendTime = 0;
unsigned long lastBlynkTimeUpdate = 0;
unsigned long lastTelegramCheckTime = 0;
unsigned long startTime = 0;

const unsigned long autoInterval = 1000;           // Mỗi 1 giây tự động tăng 1 người
const unsigned long sendInterval = 5000;           // Mỗi 5 giây gửi Telegram báo cáo
const unsigned long blynkTimeInterval = 1000;      // Mỗi 1 giây cập nhật thời gian lên Blynk
const unsigned long telegramCheckInterval = 2000;  // Mỗi 2 giây kiểm tra lệnh từ Telegram

// ==================== CẬP NHẬT MÀN HÌNH OLED ====================
void updateDisplay() {
  display.clearDisplay();
  
  // Vẽ khung viền
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // Tiêu đề
  display.setTextSize(1);
  display.setCursor(10, 5);
  display.println("SO NGUOI DEM:");
  
  // Số lượng người (cỡ lớn)
  display.setTextSize(3);
  display.setCursor(40, 25);
  if (peopleCount < 10) display.print(" ");
  display.println(peopleCount);
  
  // Trạng thái hệ thống
  display.setTextSize(1);
  display.setCursor(10, 55);
  if (systemEnabled) {
    display.println("Trang thai: HOAT DONG");
  } else {
    display.println("Trang thai: DA TAT");
  }
  
  display.display();
}

// ==================== NHẬN LỆNH BẬT/TẮT TỪ BLYNK (SWITCH V0) ====================
BLYNK_WRITE(V0) {
  systemEnabled = param.asInt();
  if (systemEnabled) {
    Serial.println("🔛 [BLYNK] He thong duoc BAT tu xa");
    bot.sendMessage(CHAT_ID, "🔛 He thong da duoc BAT tu Blynk", "");
  } else {
    Serial.println("🔴 [BLYNK] He thong duoc TAT tu xa");
    bot.sendMessage(CHAT_ID, "🔴 He thong da duoc TAT tu Blynk", "");
  }
  updateDisplay();
}

// ==================== XỬ LÝ TIN NHẮN TELEGRAM ====================
void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
  while (numNewMessages) {
    Serial.println("📩 Nhan tin nhan moi tu Telegram!");
    
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      text.toLowerCase(); // Chuyển thành chữ thường để dễ xử lý
      
      Serial.print("Tin nhan: ");
      Serial.println(text);
      
      // Kiểm tra lệnh
      if (text == "/tat" || text == "tat" || text == "off") {
        systemEnabled = false;
        Blynk.virtualWrite(V0, 0);  // Đồng bộ với Blynk
        bot.sendMessage(chat_id, "🔴 He thong da duoc TAT qua Telegram!", "");
        updateDisplay();
        Serial.println("🔴 [TELEGRAM] He thong da TAT");
      }
      else if (text == "/bat" || text == "bat" || text == "on") {
        systemEnabled = true;
        Blynk.virtualWrite(V0, 1);  // Đồng bộ với Blynk
        bot.sendMessage(chat_id, "🔛 He thong da duoc BAT qua Telegram!", "");
        updateDisplay();
        Serial.println("🔛 [TELEGRAM] He thong da BAT");
      }
      else if (text == "/trangthai" || text == "trangthai" || text == "status") {
        String statusMsg = "📊 TRANG THAI HE THONG:\n";
        statusMsg += "👥 So nguoi da dem: " + String(peopleCount) + "\n";
        statusMsg += systemEnabled ? "🔛 Trang thai: HOAT DONG" : "🔴 Trang thai: DA TAT";
        bot.sendMessage(chat_id, statusMsg, "");
        Serial.println("📊 [TELEGRAM] Da gui trang thai he thong");
      }
      else if (text == "/reset" || text == "reset") {
        peopleCount = 0;
        Blynk.virtualWrite(V2, 0);
        updateDisplay();
        bot.sendMessage(chat_id, "🔄 Da reset bo dem ve 0!", "");
        Serial.println("🔄 [TELEGRAM] Da reset bo dem");
      }
      else if (text == "/help" || text == "help") {
        String helpMsg = "🤖 LENH TELEGRAM:\n";
        helpMsg += "/bat hoac on - Bat he thong\n";
        helpMsg += "/tat hoac off - Tat he thong\n";
        helpMsg += "/trangthai hoac status - Xem trang thai\n";
        helpMsg += "/reset - Reset bo dem ve 0\n";
        helpMsg += "/help - Hien thi tro giup";
        bot.sendMessage(chat_id, helpMsg, "");
      }
    }
    
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ==================== HÀM KHỞI TẠO ====================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Khởi tạo màn hình OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Loi: Khong the khoi tao man hinh OLED");
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Khoi tao he thong...");
  display.display();
  delay(1000);
  
  // Kết nối WiFi
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Dang ket noi WiFi...");
  display.display();
  
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi da ket noi!");
    display.println("\nWiFi OK!");
  } else {
    Serial.println("\n❌ WiFi ket noi that bai!");
    display.println("\nWiFi FAIL!");
  }
  display.display();
  delay(1000);
  
  // Cấu hình Telegram client
  client.setInsecure();
  
  // Kết nối Blynk
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Dang ket noi Blynk...");
  display.display();
  
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  delay(2000);
  
  // Gửi tin nhắn khởi động Telegram
  bot.sendMessage(CHAT_ID, "🚀 He thong dem nguoi (MO PHONG) da khoi dong!\n\nGui /help de xem cac lenh!", "");
  
  startTime = millis();
  updateDisplay();
  
  Serial.println("🎯 He thong san sang!");
  Serial.println("======================================");
  Serial.println("⚠️ CHE DO MO PHONG: Moi giay tu dong tang 1 nguoi");
  Serial.println("📱 Blynk: V0 = Switch (BAT/TAT), V1 = Thoi gian, V2 = So nguoi");
  Serial.println("🤖 Telegram: Gui /bat, /tat, /trangthai, /reset, /help");
  Serial.println("======================================");
}

// ==================== VÒNG LẶP CHÍNH ====================
void loop() {
  Blynk.run();
  
  unsigned long currentMillis = millis();
  
  // ========== 1. KIỂM TRA TIN NHẮN TELEGRAM ==========
  if (currentMillis - lastTelegramCheckTime >= telegramCheckInterval) {
    lastTelegramCheckTime = currentMillis;
    handleTelegramMessages();
  }
  
  // ========== 2. CẬP NHẬT THỜI GIAN LÊN BLYNK (V1) ==========
  if (currentMillis - lastBlynkTimeUpdate >= blynkTimeInterval) {
    lastBlynkTimeUpdate = currentMillis;
    
    unsigned long elapsedSeconds = (currentMillis - startTime) / 1000;
    Blynk.virtualWrite(V1, elapsedSeconds);
  }
  
  // Nếu hệ thống bị tắt, không đếm người
  if (!systemEnabled) {
    delay(100);
    return;
  }
  
  // ========== 3. TỰ ĐỘNG TĂNG SỐ NGƯỜI MỖI GIÂY ==========
  if (currentMillis - lastAutoIncrementTime >= autoInterval) {
    lastAutoIncrementTime = currentMillis;
    peopleCount++;
    
    // Cập nhật hiển thị OLED
    updateDisplay();
    
    // Gửi số người lên Blynk (V2)
    Blynk.virtualWrite(V2, peopleCount);
    
    // Log ra Serial Monitor
    Serial.print("👣 [MO PHONG] Co nguoi di ngang! ");
    Serial.print("Thoi gian: ");
    Serial.print((currentMillis - startTime) / 1000);
    Serial.print("s | Tong so: ");
    Serial.println(peopleCount);
    
    // Đèn LED nhấp nháy
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  }
  
  // ========== 4. GỬI TIN NHẮN TELEGRAM BÁO CÁO ĐỊNH KỲ ==========
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;
    
    unsigned long elapsedSeconds = (currentMillis - startTime) / 1000;
    int hours = elapsedSeconds / 3600;
    int minutes = (elapsedSeconds % 3600) / 60;
    int secs = elapsedSeconds % 60;
    
    String msg = "📊 BAO CAO DINH KY:\n";
    msg += "👥 Tong luot nguoi: " + String(peopleCount) + "\n";
    msg += "⏱️ Thoi gian hoat dong: " + String(hours) + "h" + String(minutes) + "m" + String(secs) + "s\n";
    msg += systemEnabled ? "🔛 Trang thai: HOAT DONG" : "🔴 Trang thai: DA TAT";
    bot.sendMessage(CHAT_ID, msg, "");
    
    Serial.println("📤 Da gui bao cao dinh ky qua Telegram");
  }
  
  delay(100);
}