#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Thông tin Telegram chính chủ của Vinh
#define BOTtoken "8664422291:AAGPofPt-OnIPDKNdVIA0rL2T98imIgL4f8"
#define GROUP_ID "-4941391701"

// 3. Cấu hình chân Pin
const int motionSensor = 27; // Cảm biến PIR
const int ledPin = 23;        // Đèn LED

bool motionDetected = false;
unsigned long lastTimeBotRan;
int delayBetweenChecks = 1000; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Hàm định dạng chuỗi tin nhắn
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

// Hàm xử lý tin nhắn tới - Đã fix lỗi phản hồi loạn
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID) continue; 

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED:\n\n";
      welcome += "/led_on  : Bật đèn\n";
      welcome += "/led_off : Tắt đèn\n";
      welcome += "/get_state : Xem trạng thái hiện tại";
      bot.sendMessage(GROUP_ID, welcome, "");
    }
    else if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "LED đã bật sáng", "");
    }
    else if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "LED đã tắt", "");
    }
    else if (text == "/get_state") {
      String state = digitalRead(ledPin) ? "ON" : "OFF";
      bot.sendMessage(GROUP_ID, "Trạng thái LED hiện tại: " + state, "");
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
  
  // Thiết lập bảo mật cho Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  // Sửa lỗi DNS trên môi trường Wokwi
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns);

  Serial.println("\nWiFi connected");
  
  if(bot.sendMessage(GROUP_ID, "Hệ thống IoT của Vinh đã sẵn sàng!", "")) {
    Serial.println("Telegram sẵn sàng!");
  } else {
    Serial.println("Lỗi kết nối Telegram!");
  }
}

void loop() {
  static uint count_ = 0;

  // Xử lý khi có người chuyển động
  if (motionDetected) {
    ++count_;
    Serial.println("MOTION DETECTED!");
    String msg = StringFormat("%u => Phát hiện có người chuyển động!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str(), "");
    motionDetected = false;
  }

  // Kiểm tra tin nhắn mới từ Bot sau mỗi giây
  if (millis() > lastTimeBotRan + delayBetweenChecks) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}