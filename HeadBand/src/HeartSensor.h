#pragma once
#include <PollingTimer.h>

// 心拍センサ
class HeartSensor
{
public:
    void begin(int pin_HR);
    void task();
private:
    int _pin_HR;
    IntervalTimer interval;
};

