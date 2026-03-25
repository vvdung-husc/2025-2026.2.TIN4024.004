/*
THÔNG TIN NHÓM 05
1. Trương Đức Long -Telegram: longdev0505
2.Trần Xuân Tấn Vinh
3.Võ Hoàng Minh Phước
4.Phan Nguyễn Anh Nhật
*/

#define BLYNK_TEMPLATE_ID "TMPL60DAN-nlC"
#define BLYNK_TEMPLATE_NAME "groupExcercise"
#define BLYNK_AUTH_TOKEN "CerzzE6PrWHL_mJIjoLTYAUEMTAPxt5v"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";


#define BOT_TOKEN "8496700299:AAGMV8Wn213OvEjJS02iEYPkpwT44yFdZns"
#define CHAT_ID "-5158052780"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define DHTPIN D3
#define DHTTYPE DHT11
#define MQ2_PIN A0
#define LED_PIN D6

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);

unsigned long previousSensor = 0;
unsigned long bot_lasttime = 0;

float temp = 0;
float hum = 0;
int gas = 0;

bool ledState = false;


BLYNK_WRITE(V0)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

void handleTelegram(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String text = bot.messages[i].text;

    if (text == "/led_on")
    {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(CHAT_ID, "LED ON", "");
    }

    if (text == "/led_off")
    {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(CHAT_ID, "LED OFF", "");
    }

    if (text == "/led_status")
    {
      bot.sendMessage(CHAT_ID, ledState ? "LED DANG BAT" : "LED DANG TAT", "");
    }

    if (text == "/get_weather")
    {
      String msg = "Nhiet do: " + String(temp) + " C\n";
      msg += "Do am: " + String(hum) + " %";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}


void setup()
{
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();
  display.begin();

 
  WiFi.begin(ssid, pass);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");


  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}


void loop()
{
  Blynk.run();

  unsigned long currentMillis = millis();

  if (currentMillis - previousSensor >= 2000)
  {
    previousSensor = currentMillis;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp) && !isnan(newHum))
    {
      
      if (newTemp != temp || newHum != hum)
      {
        String msg = "Thay doi!\n";
        msg += "Temp: " + String(newTemp) + " C\n";
        msg += "Hum: " + String(newHum) + " %";
        bot.sendMessage(CHAT_ID, msg, "");
      }

      temp = newTemp;
      hum = newHum;
    }

  
    gas = analogRead(MQ2_PIN);


    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);

    display.setCursor(0, 12);
    display.print("Temp: ");
    display.print(temp);

    display.setCursor(0, 28);
    display.print("Hum: ");
    display.print(hum);

    display.setCursor(0, 44);
    display.print("Gas: ");
    display.print(gas);

    display.setCursor(0, 60);
    display.print("Up: ");
    display.print(millis() / 1000);
    display.print("s");

    display.setCursor(80, 60);
    display.print("Team X");

    display.sendBuffer();

 
    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V2, hum);
    Blynk.virtualWrite(V3, gas);
    Blynk.virtualWrite(V4, millis() / 1000);
  }

  
  if (millis() - bot_lasttime > 1000)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      handleTelegram(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}