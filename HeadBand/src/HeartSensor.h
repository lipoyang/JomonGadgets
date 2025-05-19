#pragma once
#include <PollingTimer.h>

// 心拍センサ
class HeartSensor
{
public:
    void begin(int pin_HR);
    void task();
    int  getBPM() const { return BPM; }
    bool isPulse() {
        bool ret = QS;
        QS = false;
        return ret;
    }

private:
    IntervalTimer interval;
    
    int pulsePin;

    int BPM;            // 心拍数[BPM]
    int Signal;         // アナログ入力の生データ
    int IBI = 1000;     // 心拍間隔(IBI = Inter-Beat Interval) [msec]
    bool Pulse = false; // 心拍のデジタルパルス (true:HIGH / false:LOW)
    bool QS = false;    // 心拍の瞬間フラグ (心拍の瞬間のときtrue)

    int rate[10];               // 直近10回のIBI(心拍間隔)を保持
    uint32_t sampleCounter = 0; // サンプル時刻 [msec]
    uint32_t lastBeatTime  = 0; // 前回の心拍の時刻 [msec] (心拍間隔検出用)
    int P = 512;                // 脈波のピークの値
    int T = 512;                // 脈波の谷の値
    int thresh = 512;           // 心拍検出用の閾値
    int amp = 100;              // 脈波の振幅
    bool firstBeat  = true;     // 初期化用フラグ1
    bool secondBeat = true;     // 初期化用フラグ2
};

