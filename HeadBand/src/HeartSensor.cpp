// 心拍センサ
#include <Arduino.h>
#include "HeartSensor.h"

static const int   DELTA_T_MSEC = 10; //[msec]

// 初期化
void HeartSensor::begin(int pin_HR)
{
    _pin_HR = pin_HR;

    // 周期タイマ開始
    interval.set(DELTA_T_MSEC);
}

// 周期処理
void HeartSensor::task()
{
    // 周期処理
    if(interval.elapsed())
    {
        int hr_raw = analogRead(_pin_HR);
    }
}
