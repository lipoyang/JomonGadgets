#pragma once
#include <ArduinoBLE.h>
#include "NeoPixelCtrl.h"

// BLE-NeoPixcel制御クラス
class BleNeoPixel
{
public:
    void begin(NeoPixelCtrl& controller);
    void task();
    
private:
    NeoPixelCtrl *controller;
    BLEDevice central;
    bool isConnected;
};
