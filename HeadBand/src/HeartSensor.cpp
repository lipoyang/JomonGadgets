// 心拍センサ
#include <Arduino.h>
#include "HeartSensor.h"

//#define _DEBUG
#define PIN_TEST   D4

static const int   DELTA_T_MSEC = 10; //[msec]

// 移動平均
const int bufferSize = 8;
int buffer[bufferSize];
int bufferIndex = 0;

// 微分値とゼロクロス用
int prevFiltered = 0;
int prevDiff = 0;

// ゼロクロス＋しきい値
const int diffThreshold = 5;  // 微分しきい値（調整可能）

// 心拍検出
unsigned long lastBeatTime = 0;
const int minBeatInterval = 300; // ms、200BPM上限

// BPM計算
#define MAX_BEATS 10
unsigned long beatIntervals[MAX_BEATS];
int beatIndex = 0;

// 初期化
void HeartSensor::begin(int pin_HR)
{
    pulsePin = pin_HR;

#ifdef _DEBUG
    pinMode(PIN_TEST, OUTPUT);
#endif

    // 周期タイマ開始
    interval.set(DELTA_T_MSEC);
}

// 周期処理
void HeartSensor::task()
{
    // 周期処理
    if(interval.elapsed())
    {
        Signal = analogRead(pulsePin);              // 脈波センサを読む
        sampleCounter += DELTA_T_MSEC;              // 時刻のカウント[msec]
        int N = sampleCounter - lastBeatTime;       // 前回の心拍からの時間 (ノッチ回避用)

        // 脈波のピークと谷を検出する
        if(Signal < thresh && N > (IBI/5)*3){       // ノッチ回避のため、直近の心拍間隔の 3/5 だけ待つ
            if (Signal < T){                        // 脈波の谷の値を保持
                T = Signal;
            }
        }
        
        if(Signal > thresh && Signal > P){          // 閾値条件はノイズ回避のため
            P = Signal;                             // 脈波のピークの値を保持
        }
        
        // 脈波の立ち上がり検出
        if (N > 250){
            if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){
#ifdef _DEBUG
                digitalWrite(PIN_TEST, HIGH);           // テスト用LED
#endif
                Pulse = true;                           // 心拍パルス HIGH
                IBI = sampleCounter - lastBeatTime;     // 心拍間隔
                lastBeatTime = sampleCounter;           // 心拍の時刻を保持
                
                if(firstBeat){                          // 初回の心拍は信頼できないので破棄
                    firstBeat = false;
                    return;
                }
                if(secondBeat){                         // 2回目の心拍のとき
                    secondBeat = false;
                    for(int i=0; i<=9; i++){            // rate[]に適当な初期値をセット
                        rate[i] = IBI;
                    }
                }
                
                // 直近10回のIBIの移動平均
                int runningTotal = 0;
                for(int i=0; i<=8; i++){
                    rate[i] = rate[i+1];
                    runningTotal += rate[i];
                }
                rate[9] = IBI;
                runningTotal += rate[9];
                runningTotal /= 10;
                BPM = 60000/runningTotal;               // IBIからBPMを計算
                QS = true;                              // 心拍の瞬間フラグ
            }
        }
        
        // 脈波の立ち下がり検出
        if (Signal < thresh && Pulse == true){
#ifdef _DEBUG
            digitalWrite(PIN_TEST, LOW);           // テスト用LED
#endif
            Pulse = false;                         // 心拍パルス LOW
            amp = P - T;                           // 脈波の振幅を計算
            thresh = amp/2 + T;                    // 振幅の中点を立ち上がり判定閾値とする
            P = thresh;                            // ピークと谷の値をリセット
            T = thresh;
        }
        
        // 2.5秒間心拍が検出されなかったら仕切り直し
        if (N > 2500){
            thresh = 512;
            P = 512;
            T = 512;
            lastBeatTime = sampleCounter;
            firstBeat = true;
            secondBeat = true;
        }
#ifdef _DEBUG
        static int cnt = 0;
        cnt = (cnt + 1) % 20;
        if(cnt == 0){
            Serial.println(Signal);
        }
#endif
    }
}
