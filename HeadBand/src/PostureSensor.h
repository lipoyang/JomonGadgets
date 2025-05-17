#pragma once
#include <PollingTimer.h>

// 姿勢センサ
class PostureSensor
{
public:
    void begin(int pin_SS);
    void task();
    void startCalibration();
    bool wasCalibrated() {
        bool ret = isCalibrated;
        isCalibrated = false;
        return ret;
    }
    float getPitch() const { return th_y - th0_y; }
    float getRoll() const { return th_z; }

private:
    IntervalTimer interval;
    bool resetTheta;
    bool isCalibrating;
    bool isCalibrated;
    float th_y;
    float th_z;
    float th0_y;
    float calibAcc;
    int calibCnt;
};

