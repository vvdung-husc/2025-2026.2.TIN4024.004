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
#define BOTtoken "8613471187:AAGgWAqQV_o60HA4TfVsk9eWzAeOL-wsN-U"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-5206225127" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
bool motionDetected = false;

//LED
const int ledPin = 23;
bool ledState = false;

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

// ====== XỬ LÝ LỆNH TELEGRAM ======
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {

      String welcome = "Xin chào.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển LED.\n\n";
      welcome += "Gửi /led_on để bật đèn\n";
      welcome += "Gửi /led_off để tắt đèn\n";
      welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {

      digitalWrite(ledPin, HIGH);
      ledState = true;

      bot.sendMessage(chat_id, "LED bật sáng", "");
      Serial.print("LED is ON\n");
    }

    if (text == "/led_off") {

      digitalWrite(ledPin, LOW);
      ledState = false;

      bot.sendMessage(chat_id, "LED đã tắt", "");
      Serial.print("LED is OFF\n");
    }

    if (text == "/get_state") {

      if (ledState){
        bot.sendMessage(chat_id, "LED is ON", "");
        Serial.print("State of LED is ON\n");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
        Serial.print("State of LED is OFF\n");
      }
    }

  }
}

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // ====== LED ======
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
   // ====== ĐỌC LỆNH TELEGRAM ======
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while(numNewMessages) {

    handleNewMessages(numNewMessages);

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  static uint count_ = 0;
 if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    motionDetected = false;
  }

  delay(1000);
}