#include <PollingTimer.h>
#include <Button.h>
#include "BleNeoPixelCentral.h"
#include "PostureSensor.h"
#include "HeadBand.h"

#define _DEBUG

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
        case PTN_POWER_ON:    // 電源ON
            tone(PIN_BUZZ, F_C5, T_BUZZ);
            delay(T_BUZZ);
            break;
        case PTN_CONNECT:     // 接続
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_E4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case PTN_DISCONNECT:  // 切断
            tone(PIN_BUZZ, F_E4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case PTN_HEART_RATE:  // 心拍モード
            tone(PIN_BUZZ, F_G4, T_BUZZ_SHORT);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_G4, T_BUZZ_SHORT);
            delay(T_BUZZ_SHORT);
            break;
        case PTN_POSTURE:     // 姿勢モード
            tone(PIN_BUZZ, F_G4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case PTN_CALIBRATING: // キャリブレーション中
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            break;
        case PTN_CALIBRATED:  // キャリブレーション完了
            tone(PIN_BUZZ, F_D4, T_BUZZ);
            delay(T_BUZZ);
            tone(PIN_BUZZ, F_C4, T_BUZZ);
            delay(T_BUZZ);
            break;
    }
    pinMode(PIN_BUZZ, INPUT); // ブザーOFF
}

// 姿勢に応じた発光色データの送信
void sendPosture(float th_p, float th_r)
{
    float th = max(fabs(th_p), fabs(th_r));
    if(th > 30.0f) th = 30.0f;
    //int h = 0x1000 - (int)(th * (float)0x1000 / 30.0f);
    //if(h < 0) h = 0;
    int dC = 50 - (int)(th * (float)50 / 30.0f);
    if(dC < 0) dC = 0;
//  int dV = 40 - (int)(th * (float)40 / 30.0f);
//  if(dV < 0) dV = 0;
//    int dV = 40 + (int)(th * (float)40 / 30.0f);
    bleNeoPixelCentral.setDC(dC);
//    bleNeoPixelCentral.setDV(dV);
    int brightness = 32 + (int)(th * (float)32 / 30.0f);
    bleNeoPixelCentral.setBrightness(brightness);
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
    buzz_out(PTN_POWER_ON);
    
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
    if(postureSensor.wasCalibrated()){
        buzz_out(PTN_CALIBRATED);
    }
    float th_p = postureSensor.getPitch();
    float th_r = postureSensor.getRoll();

    // 周期処理
    if(interval1.elapsed()){
        // 接続時
        if(!isConnected && bleNeoPixelCentral.isConnected()){
            isConnected = true;
            Serial.println("BLE connected");
            led_show();
            buzz_out(PTN_CONNECT);
        }
        // 切断時
        else if(isConnected && !bleNeoPixelCentral.isConnected()){
            isConnected = false;
            Serial.println("BLE disconnected");
            led_show();
            buzz_out(PTN_DISCONNECT);
        }
        // 接続中
        if(isConnected){
            button.read();
            // ボタン長押しで姿勢センサのキャリブ開始
            if(button.pressedFor(T_LONG_PRESS, T_LONG_INTERVAL)){
                buzz_out(PTN_CALIBRATING);
                Serial.println("Calibrating...");
                postureSensor.startCalibration();
            }
            // ボタン短押しでモード切替
            else if(button.wasReleased()){
                Serial.println("Button pressed");
                mode = (mode + 1) % 2; // MODE_POSTURE <-> MODE_HEART_RATE
                led_show();
                if(mode == MODE_POSTURE){
                    buzz_out(PTN_POSTURE);
                    bleNeoPixelCentral.setPattern(PTN_FLUCTUATION);
                }else{
                    buzz_out(PTN_HEART_RATE);
                    bleNeoPixelCentral.setPattern(PTN_TWO_COLOR); // TODO
                    //bleNeoPixelCentral.setPattern(PTN_HEART);
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
