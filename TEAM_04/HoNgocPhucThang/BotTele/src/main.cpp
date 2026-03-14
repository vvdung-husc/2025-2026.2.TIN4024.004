#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 1. Thông tin mạng (Giữ nguyên cho Wokwi)
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// 2. Thông tin Telegram của bạn
#define BOTtoken "8161835889:AAEfRs92rg80jkTCOpDUR7VdEpOIRNs9spQ"
#define GROUP_ID "-5275861628"

// 3. Khai báo chân Pin (Theo hình ảnh bạn gửi)
const int motionSensor = 27; // Cảm biến PIR chân 27
const int ledPin = 23;       // Đèn LED chân 23

bool motionDetected = false;
unsigned long lastTimeBotRan;
int delayBetweenChecks = 1000; // 1 giây kiểm tra tin nhắn mới 1 lần

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Hàm định dạng chuỗi
String StringFormat(const char *fmt, ...)
{
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

// Hàm ngắt khi phát hiện chuyển động
void IRAM_ATTR detectsMovement()
{
  motionDetected = true;
}

// Hàm xử lý tin nhắn từ Telegram gửi đến
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID)
      continue; // Chỉ nhận lệnh từ nhóm chỉ định

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start")
    {
      String welcome = "Xin chào, " + from_name + ".\n";
      welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
      welcome += "Gửi /led_on để bật đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để xem trạng thái hiện tại";
      bot.sendMessage(GROUP_ID, welcome, "");
    }

    if (text == "/led_on")
    {
      digitalWrite(ledPin, HIGH);
      bot.sendMessage(GROUP_ID, "LED đã bật sáng", "");
    }

    if (text == "/led_off")
    {
      digitalWrite(ledPin, LOW);
      bot.sendMessage(GROUP_ID, "LED đã tắt", "");
    }

    if (text == "/get_state")
    {
      if (digitalRead(ledPin))
      {
        bot.sendMessage(GROUP_ID, "LED is ON", "");
      }
      else
      {
        bot.sendMessage(GROUP_ID, "LED is OFF", "");
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Mặc định tắt LED

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nWiFi connected");
  bot.sendMessage(GROUP_ID, "Hệ thống IoT đã sẵn sàng!", "");
}

void loop()
{
  static uint count_ = 0;

  // 1. Xử lý khi phát hiện chuyển động (Tự động gửi tin)
  if (motionDetected)
  {
    ++count_;
    Serial.println("MOTION DETECTED!");
    String msg = StringFormat("%u => Phát hiện có người chuyển động!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str(), "");
    motionDetected = false;
  }

  // 2. Kiểm tra tin nhắn mới từ Telegram (Điều khiển LED)
  if (millis() > lastTimeBotRan + delayBetweenChecks)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}