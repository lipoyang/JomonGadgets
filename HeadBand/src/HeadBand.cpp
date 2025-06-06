#include <PollingTimer.h>
#include <Button.h>
#include "BleNeoPixelCentral.h"
#include "PostureSensor.h"
#include "HeartSensor.h"
#include "HeadBand.h"

//#define _DEBUG

// ピン番号
#define PIN_HR    A0  // 心拍センサー
#define PIN_BTN   D1  // ボタン 
#define PIN_VBUS  D2  // USB接続検出用
#define PIN_BUZZ  D3  // ブザー
#define PIN_SS    D7  // SPIセレクト (BMI160)

// 動作モード
#define MODE_POSTURE      0 // 姿勢モード
#define MODE_HEART_RATE   1 // 心拍モード

// BLE NeoPixel セントラル
BleNeoPixel bleNeoPixelCentral;
// 姿勢センサ
PostureSensor postureSensor;
// 心拍センサ
HeartSensor heartSensor;
// 周期タイマ
IntervalTimer interval1;
// ボタン
Button button;

static int mode = MODE_POSTURE;   // 動作モード
static bool isConnected = false;  // BLE接続中か？

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

// ブザーの音を鳴らす
void buzz_out(int pattern)
{
    switch(pattern){
        case BUZZ_POWER_ON:    // 電源ON
            tone(PIN_BUZZ, F_C5, T_BUZZ);
            delay(T_BUZZ);
            break;
        case BUZZ_CONNECT:     // 接続
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_E4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case BUZZ_DISCONNECT:  // 切断
            tone(PIN_BUZZ, F_E4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case BUZZ_HEART_RATE:  // 心拍モード
            tone(PIN_BUZZ, F_G4, T_BUZZ_SHORT);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_G4, T_BUZZ_SHORT);
            delay(T_BUZZ_SHORT);
            break;
        case BUZZ_POSTURE:     // 姿勢モード
            tone(PIN_BUZZ, F_G4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case BUZZ_CALIBRATING: // キャリブレーション中
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case BUZZ_CALIBRATED:  // キャリブレーション完了
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            break;
    }
    pinMode(PIN_BUZZ, INPUT); // ブザーOFF
}

// 姿勢データの送信
void sendPosture(float th_p, float th_r)
{
    float th = max(fabs(th_p), fabs(th_r));
    if(th > 30.0f) th = 30.0f;
    bleNeoPixelCentral.setPosture((int)th);
}   

// 初期化
void setup()
{
    // LEDとボタンの初期化
    led_init();
    button.begin(PIN_BTN, true, 10); // (ピン番号, 負論理, デバウンス時間)
    button.setHoldTime(T_LONG_PRESS);

    // Vbus検出 (USB接続時はシリアルポートを開くまで待つ)
    pinMode(PIN_VBUS, INPUT);
    Serial.begin(115200);
    delay(100);
    if(digitalRead(PIN_VBUS) == HIGH){
      while(!Serial){;}
    }
    Serial.println("Hello!");
    led_show();

    // 起動音
    buzz_out(BUZZ_POWER_ON);
    
    // BLE NeoPixel セントラルを開始
    bleNeoPixelCentral.begin();
    // 姿勢センサを開始
    postureSensor.begin(PIN_SS);
    // 心拍センサを開始
    heartSensor.begin(PIN_HR);
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
    if(postureSensor.wasCalibrated()){
        buzz_out(BUZZ_CALIBRATED);
    }
    float th_p = postureSensor.getPitch();
    float th_r = postureSensor.getRoll();

    // 心拍センサのタスク
    heartSensor.task();
    if(heartSensor.isPulse()){
        int bpm = heartSensor.getBPM();
        Serial.print("BPM: ");
        Serial.println(bpm);
        if(isConnected && mode == MODE_HEART_RATE){
            bleNeoPixelCentral.setBPM(bpm);
        }
    }

    // 周期処理
    if(interval1.elapsed()){
        // ボタン読み取り
        bool pressed_short = false;
        bool pressed_long  = false;
        button.read();
        if(button.pressedFor(T_LONG_PRESS, T_LONG_INTERVAL)){
            pressed_long = true;
        }else if(button.wasReleased()){
            pressed_short = true;
        }
        
        // ボタン長押しで姿勢センサのキャリブ開始
        if(pressed_long){
            buzz_out(BUZZ_CALIBRATING);
            Serial.println("Calibrating...");
            postureSensor.startCalibration();
        }

        // 接続時
        if(!isConnected && bleNeoPixelCentral.isConnected()){
            isConnected = true;
            Serial.println("BLE connected");
            led_show();
            buzz_out(BUZZ_CONNECT);
            if(mode == MODE_POSTURE){
                bleNeoPixelCentral.setPattern(PTN_POSTURE);
            }else{
                bleNeoPixelCentral.setPattern(PTN_HEART);
            }
        }
        // 切断時
        else if(isConnected && !bleNeoPixelCentral.isConnected()){
            isConnected = false;
            Serial.println("BLE disconnected");
            led_show();
            buzz_out(BUZZ_DISCONNECT);
        }
        // 接続中
        if(isConnected){
            // ボタン短押しでモード切替
            if(pressed_short){
                Serial.println("Button pressed");
                mode = (mode + 1) % 2; // MODE_POSTURE <-> MODE_HEART_RATE
                led_show();
                if(mode == MODE_POSTURE){
                    buzz_out(BUZZ_POSTURE);
                    bleNeoPixelCentral.setPattern(PTN_POSTURE);
                }else{
                    buzz_out(BUZZ_HEART_RATE);
                    bleNeoPixelCentral.setPattern(PTN_HEART);
                }
            }
            // 姿勢センサのデータをBLEで送信
            if(mode == MODE_POSTURE){
                sendPosture(th_p, th_r);
            }
        }
#ifdef _DEBUG
        static int cnt = 0;
        if(cnt++ % 4 == 0){
            Serial.print("th_p: ");
            Serial.print(th_p);
            Serial.print("\t");
            Serial.print("th_r: ");
            Serial.print(th_r);
            Serial.println();
        }
#endif
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
