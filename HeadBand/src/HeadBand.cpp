#include <PollingTimer.h>
#include "BleNeoPixelCentral.h"
#include "PostureSensor.h"

// ピン番号
#define PIN_HR    A0  // 心拍センサー
#define PIN_BTN   D1  // ボタン 
#define PIN_VBUS  D2  // USB接続検出用
#define PIN_SS    D7  // SPIセレクト (BMI160)

// 動作モード
#define MODE_POSTURE      0 // 姿勢モード
#define MODE_HEART_RATE   1 // 心拍モード

// BLE NeoPixel セントラル
BleNeoPixel bleNeoPixelCentral;
// 姿勢センサ
PostureSensor postureSensor;
// 周期タイマ
IntervalTimer interval1;

static int mode = MODE_POSTURE;   // 動作モード
static bool isConnected = false;  // BLE接続中か？
static int btn_prev = HIGH;       // ボタンの前回状態

// LEDの初期化
void led_init()
{
    pinMode(LED_RED,   OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE,  OUTPUT);
    digitalWrite(LED_RED,   HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE,  HIGH);
}

// LEDの表示
void led_show()
{
    // 接続中
    if(isConnected){
        // 姿勢モード：青
        if(mode == MODE_POSTURE){
            digitalWrite(LED_RED,   HIGH);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE,  LOW);
        }
        // 心拍モード：緑
        else{
            digitalWrite(LED_RED,   HIGH);
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_BLUE,  HIGH);
        }
    }
    // 切断時：赤
    else{
        digitalWrite(LED_RED,   LOW);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_BLUE,  HIGH);
    }
}

// 初期化
void setup()
{
    // LEDとボタンの初期化
    led_init();
    pinMode(PIN_BTN,   INPUT_PULLUP);

    // Vbus検出 (USB接続時はシリアルポートを開くまで待つ)
    pinMode(PIN_VBUS, INPUT);
    Serial.begin(115200);
    delay(100);
    if(digitalRead(PIN_VBUS) == HIGH){
      while(!Serial){;}
    }
    Serial.println("Hello!");
    led_show();
    
    // BLE NeoPixel セントラルを開始
    bleNeoPixelCentral.begin();
    // 姿勢センサを開始
    postureSensor.begin(PIN_SS);
    // 周期タイマ開始
    interval1.set(100);
}

// メインループ
void loop()
{
    // BLE NeoPixel セントラルのタスク
    bleNeoPixelCentral.task();
    // 姿勢センサのタスク
    postureSensor.task();

    // 周期処理
    if(interval1.elapsed()){
        // 接続時
        if(!isConnected && bleNeoPixelCentral.isConnected()){
            isConnected = true;
            Serial.println("BLE connected");
            led_show();
        }
        // 切断時
        else if(isConnected && !bleNeoPixelCentral.isConnected()){
            isConnected = false;
            btn_prev = HIGH;
            Serial.println("BLE disconnected");
            led_show();
        }
        // 接続中
        if(isConnected){
            // ボタンを押したらモード切替
            int btn = digitalRead(PIN_BTN);
            if(btn == LOW && btn_prev == HIGH){
                Serial.println("Button pressed");
                mode = (mode + 1) % 2; // MODE_POSTURE <-> MODE_HEART_RATE
                led_show();
            }
            btn_prev = btn;
        }
    }
    
    // デバッグ用
    if(Serial.available() > 0)
    {
        char c = Serial.read();
        switch(c){
            case '0': bleNeoPixelCentral.setPattern(PTN_OFF);         break;
            case '1': bleNeoPixelCentral.setPattern(PTN_ONE_COLOR);   break;
            case '2': bleNeoPixelCentral.setPattern(PTN_TWO_COLOR);   break;
            case '3': bleNeoPixelCentral.setPattern(PTN_FADE);        break;
            case '4': bleNeoPixelCentral.setPattern(PTN_ROUND);       break;
            case '5': bleNeoPixelCentral.setPattern(PTN_FLUCTUATION); break;
        }
    }
}
