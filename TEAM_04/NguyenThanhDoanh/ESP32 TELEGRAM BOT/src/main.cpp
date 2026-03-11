#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Thông tin Telegram đã lấy từ ảnh của bạn
#define BOTtoken "8671771731:AAG_WRAC0ATmcYQaMyPPZC6TiWBeHVOgFLc"
#define GROUP_ID "-5198049087"

// 3. Khai báo chân Pin (Theo hình ảnh bạn gửi)
const int motionSensor = 27; // Cảm biến PIR chân 27
const int ledPin = 23;       // Đèn LED chân 23

bool motionDetected = false;
unsigned long lastTimeBotRan;
int delayBetweenChecks = 1000; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Hàm định dạng chuỗi
String StringFormat(const char *fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char *buff = (char *)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; 

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on để bật đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để xem trạng thái hiện tại";
      bot.sendMessage(GROUP_ID, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "LED đã bật sáng", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "LED đã tắt", "");
    }

    if (text == "/get_state") {
      String state = digitalRead(ledPin) ? "ON" : "OFF";
      bot.sendMessage(GROUP_ID, "LED is " + state, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Cài đặt chứng chỉ cho Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  // --- PHẦN SỬA LỖI DNS ---
  // Thiết lập Google DNS để ESP32 tìm được api.telegram.org
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns);
  // -------------------------

  Serial.println("\nWiFi connected");
  
  // Thử gửi tin nhắn chào mừng, nếu DNS lỗi lệnh này sẽ thất bại
  if(bot.sendMessage(GROUP_ID, "Hệ thống IoT đã sẵn sàng!", "")) {
    Serial.println("Telegram sẵn sàng!");
  } else {
    Serial.println("Lỗi gửi tin nhắn khởi động - Kiểm tra lại DNS/Token");
  }
}

void loop() {
  static uint count_ = 0;

  if (motionDetected) {
    ++count_;
    Serial.println("MOTION DETECTED!");
    String msg = StringFormat("%u => Phát hiện có người chuyển động!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str(), "");
    motionDetected = false;
  }

  if (millis() > lastTimeBotRan + delayBetweenChecks) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}