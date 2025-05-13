#pragma once

#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <NanoBLEFlashPrefs.h>
#include "Fluctuator.h"
#include "PollingTimer.h"

#define LED_MAX         16      // NeoPixelの数
#define LED_PIN         D0      // NeoPixelの制御ピン
#define POWER_ON_OFF            // NeoPixelの電源ON/OFFをFET等で制御
#define POWER_PIN       D1      // NeoPixelの電源制御ピン

// ゆらぎのHSV
enum{
//  F_HUE = 0,
//  F_SAT,
//  F_VAL,
//  F_HSV
    F_COL = 0,
    F_VAL,
    F_HSV
};

// 発行パターン
typedef enum{
    PTN_OFF = 0,    // けす
    PTN_ONE_COLOR,  // ひといろ
    PTN_TWO_COLOR,  // ふたいろ
    PTN_FADE,       // ほたる
    PTN_ROUND,      // ぐるぐる
    PTN_FLUCTUATION // ゆらめき
} Iluminetion;

// NeoPixelコントローラ
class NeoPixelCtrl
{
public:
    // コンストラクタ
    NeoPixelCtrl();
    
    // 設定
    void setBrightness(uint8_t brightness);
    void setColor1(uint16_t h, uint8_t s);
    void setColor2(uint16_t h, uint8_t s);
    void setT_2color(int ms);
    void setT_fade  (int ms);
    void setT_round (int ms);
    void setT_fluct (int ms);
//  void setFluctuation(int h, int s, int v);
    void setFluctuation(int c, int v);
    void setPattern (Iluminetion pattern);
    // コマンド
    void save();
    void reset();
    void load();
    
    // beginとtask
    void begin();
    void task();
    
    // 設定の取得
    void getParams(
        uint8_t &brightness,
        uint16_t &H1, uint8_t &S1,
        uint16_t &H2, uint8_t &S2,
        int &T_2color, int &T_fade, int &T_round, int &T_fluct,
        float &dC, float &dV,
        Iluminetion &pattern
        );
    
private:
    // コンフィグ
    int led_num;        // NeoPixelの数
    
    // 設定値
    uint8_t brightness; // 明るさ(全体) (0-255)
    uint16_t H1;        // 色1の色相 (0-255)
    uint8_t  S1;        // 色1の彩度 (0-255)
    uint16_t H2;        // 色2の色相 (0-255)
    uint8_t  S2;        // 色2の彩度 (0-255)
    int T_2color;       // ふたいろの周期 [ms]
    int T_fade;         // ほたるの周期 [ms]
    int T_round;        // ぐるぐるの周期 [ms]
    int T_fluct;        // ゆらめきの更新周期 [ms]
//  float dH;           // 色相のゆらめき (0.0 - 1.0)
//  float dS;           // 彩度のゆらめき (0.0 - 1.0)
//  float dV;           // 明度のゆらめき (0.0 - 1.0)
    float dC;           // 色のゆらめき     (0.0 - 1.0)
    float dV;           // 明るさのゆらめき (0.0 - 1.0)
    Iluminetion pattern; // 発光パターン
    
    // 各パターンの処理
    void patternOff();      // けす
    void patternOneColor(); // ひといろ
    void patternTwoColor(); // ふたいろ
    void patternFade();     // ほたる
    void patternRound();    // ぐるぐる
    void patternFluction(); // ゆらめき
    
    // NeoPixel
    Adafruit_NeoPixel pixels;
    // ゆらぎ発生器
    Fluctuator fluct[LED_MAX][F_HSV];
    // 周期タイマ
    IntervalTimer interval;
    // 回数カウント
    int n_cnt;
    
    // 設定値保存
    NanoBLEFlashPrefs prefs;
    
    // デバッグ用
    void printSettings();
};
