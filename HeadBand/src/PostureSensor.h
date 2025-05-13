#pragma once
#include <PollingTimer.h>

// 姿勢センサ
class PostureSensor
{
public:
    void begin(int pin_SS);
    void task();

    float th_y;
    float th_z;
private:
    IntervalTimer interval;
    bool resetTheta;
};

