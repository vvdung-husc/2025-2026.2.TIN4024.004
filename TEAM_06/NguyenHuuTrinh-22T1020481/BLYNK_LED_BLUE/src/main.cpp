#include "main.h"

// --- Cấu hình WiFi cho Wokwi ---
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// --- Định nghĩa chân theo Diagram ---
#define PIN_LED      21
#define PIN_BTN      23
#define PIN_TM_CLK   18
#define PIN_TM_DIO   19
#define PIN_DHT      16

#define DHT_TYPE     DHT22

// --- Khởi tạo đối tượng toàn cục ---
MyLED ledBlue;
MyButton btnToggle;
MyDHT dhtSensor;
MyDisplay tmDisplay;
SystemController mySystem;

// --- Hàm xử lý tín hiệu từ App Blynk (Nút nhấn V1) ---
BLYNK_WRITE(V1) {
    int pinValue = param.asInt(); 
    bool state = false;
    
    if (pinValue == 1) {
        state = true;
    }
    
    // Gửi tín hiệu vào hàm setLedState của hệ thống
    mySystem.setLedState(state, ledBlue);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Bat dau khoi dong...");

    // Setup phần cứng
    ledBlue.setup(PIN_LED);
    btnToggle.setup(PIN_BTN);
    dhtSensor.setup(PIN_DHT, DHT_TYPE);
    tmDisplay.setup(PIN_TM_CLK, PIN_TM_DIO);

    // Kết nối Blynk và WiFi (Sử dụng mảng char đã khai báo ở trên)
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
    Blynk.run(); // Bắt buộc phải có để giữ kết nối Blynk
    
    // Chạy logic chính của toàn bộ mạch
    mySystem.run(ledBlue, btnToggle, dhtSensor, tmDisplay);
}