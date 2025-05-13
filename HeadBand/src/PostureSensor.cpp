// 姿勢センサ
#include <Arduino.h>
#include <BMI160Gen.h>
#include "PostureSensor.h"

static const float F_PI=3.14159265f;
static const int   DELTA_T_MSEC = 10; //[msec]
static const float DELTA_T      = DELTA_T_MSEC / 1000.0f; //[sec]
static const float k = 0.99; // 相補フィルタ係数

// 角速度の換算
float convertRawGyro(int gRaw)
{
  float g = (gRaw * 250.0f) / 32768.0f;
  return g;
}

// 加速度の換算
float convertRawAcc(int aRaw) 
{
  float a = (aRaw * 2.0f) / 32768.0f;
  return a;
}

// 初期化
void PostureSensor::begin(int pin_SS)
{
    // BMI160の初期化
    BMI160.begin(BMI160GenClass::SPI_MODE, pin_SS);
    //BMI160.begin(BMI160GenClass::I2C_MODE);
    uint8_t dev_id = BMI160.getDeviceID();
    Serial.print("DEVICE ID: ");
    Serial.println(dev_id, HEX);

    // ジャイロと加速度のレンジ設定
    BMI160.setGyroRange(250);
    BMI160.setAccelerometerRange(2);
    Serial.println("Initializing IMU device...done.");
    
    // 初期化フラグ
    resetTheta = true;

    // 周期タイマ開始
    interval.set(DELTA_T_MSEC);
}

// 周期処理
void PostureSensor::task()
{
    // 周期処理
    if(interval.elapsed())
    {
        int gxRaw, gyRaw, gzRaw;         // raw gyro values
        int axRaw, ayRaw, azRaw;
        float gx, gy, gz;
        float ax, ay, az;
    
        // read raw gyro measurements from device
        BMI160.readGyro(gxRaw, gyRaw, gzRaw);
        BMI160.readAccelerometer(axRaw, ayRaw, azRaw);
    
        // convert the raw gyro data to degrees/second
        gx = convertRawGyro(gxRaw);
        gy = convertRawGyro(gyRaw);
        gz = convertRawGyro(gzRaw);
        ax = convertRawAcc(axRaw);
        ay = convertRawAcc(ayRaw);
        az = convertRawAcc(azRaw);
    
        // 加速度による姿勢角推定
        // θa = atan( az / ax )
        float tha_y = atan2f(-az, -ax) * 180.0f / F_PI;
        float tha_z = atan2f( ay, -ax) * 180.0f / F_PI;
        
        // 簡易相補フィルタによる姿勢角推定
        // θ = k * (θ + ωy * Δt) + (1-k) * θa
        if(resetTheta){
            resetTheta = false;
            // 初期値 θ = θa
            th_y = tha_y; 
            th_z = tha_z; 
        }else{
            th_y = k*(th_y + gy * DELTA_T) + (1 - k)*(tha_y);
            th_z = k*(th_z + gz * DELTA_T) + (1 - k)*(tha_z);
        }

#if 1   // デバッグ用
        static int cnt = 0;
        if(cnt++ % 20 == 0){
            Serial.print("th_y: ");
            Serial.print(th_y);
            Serial.print("\t");
            Serial.print("th_z: ");
            Serial.print(th_z);
            Serial.print("\t");
#if 0
            Serial.print("tha_y: ");
            Serial.print(tha_y);
            Serial.print("\t");
            Serial.print("tha_z: ");
            Serial.print(tha_z);
            Serial.print("\t");
#endif
#if 0
            Serial.print("g:\t");
            Serial.print(gx);
            Serial.print("\t");
            Serial.print(gy);
            Serial.print("\t");
            Serial.print(gz);
            Serial.print("\t a:\t");
            Serial.print(ax);
            Serial.print("\t");
            Serial.print(ay);
            Serial.print("\t");
            Serial.print(az);
#endif
            Serial.println();
        }
#endif  // デバッグ用
    }
}
