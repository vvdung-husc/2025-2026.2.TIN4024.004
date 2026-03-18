#include <Arduino.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "8236857374:AAGo69-uEtpdJ2fHCrYV5K_7Vk8NmtkCHrA"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-4986195783" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// PIR Motion Sensor
const int motionSensor = 27; 
bool motionDetected = false;

// LED
const int ledPin = 23;
bool ledState = false;

// Telegram check message interval
unsigned long lastTimeBotRan;
int botRequestDelay = 1000;

//Định dạng chuỗi %s,%d,...
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}

// xử lý lệnh Telegram
void handleNewMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {
      String welcome = "Xin chào,\n";
      welcome += "Sử dụng các lệnh sau để điều khiển LED \n";
      welcome += "/led_on  - Bật đèn\n";
      welcome += "/led_off - Tắt đèn\n";
      welcome += "/status  - Kiểm tra trạng thái đèn\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      Serial.println("LED đã được BẬT");
      bot.sendMessage(chat_id, "Đèn đã được BẬT", "");
    }

    if (text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      Serial.println("LED đã được TẮT");
      bot.sendMessage(chat_id, "Đèn đã được TẮT", "");
    }

    if (text == "/status") {
    if (ledState) {
    bot.sendMessage(chat_id, "Trạng thái đèn: ON", "");
    Serial.println("Kiểm tra trạng thái: LED ĐANG BẬT");
  } 
  else {
    bot.sendMessage(chat_id, "Trạng thái đèn: OFF", "");
    Serial.println("Kiểm tra trạng thái: LED ĐANG TẮT");
  }
}
  }
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  
  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}


void loop() {
  static uint count_ = 0;

  if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    motionDetected = false;
  }

  // đọc tin nhắn telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }

}