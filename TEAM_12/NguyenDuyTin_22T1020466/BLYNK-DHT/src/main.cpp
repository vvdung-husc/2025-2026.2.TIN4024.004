// BẮT BUỘC ĐỂ 3 DÒNG NÀY LÊN TRÊN CÙNG
#define BLYNK_TEMPLATE_ID "TMPL6o04aN1A-"
#define BLYNK_TEMPLATE_NAME "Nguyễn Duy Tín"
#define BLYNK_AUTH_TOKEN "p04d2jqqWxtcAv9yLaHcyv9LvSVA4Iw2"

// Sau đó mới include thư viện
#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h> 

char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// Chân kết nối 
#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xanh
#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637

// Khai báo chân cho cảm biến DHT22
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// SỬA LỖI: Đổi ulong thành unsigned long
unsigned long currentMiliseconds = 0; 
bool blueButtonON = true;     

TM1637Display display(CLK, DIO);

bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin(); 
  
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 

  Serial.println();
  Serial.println("WiFi connected");
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); 
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton(){
  static unsigned long lastTime = 0; 
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }    
}

void uptimeBlynk(){
  static unsigned long lastTime = 0; 
  if (!IsReady(lastTime, 1000)) return; 
  
  unsigned long value = lastTime / 1000; 
  Blynk.virtualWrite(V0, value);  
  if (blueButtonON){
    display.showNumberDec(value);
  }

  // --- PHẦN BỔ SUNG: ĐỌC VÀ GỬI NHIỆT ĐỘ, ĐỘ ẨM ---
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    // V2: Nhiệt độ, V3: Độ ẩm 
    Blynk.virtualWrite(V2, t); 
    Blynk.virtualWrite(V3, h); 
  }
}

BLYNK_WRITE(V1) { 
  blueButtonON = param.asInt();  
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}