#include <stdlib.h>
#include "NeoPixelCtrl.h"
#include "PollingTimer.h"

#define DELTA_T     30  // 更新周期[ms]

// 設定値保存用の構造体
typedef struct
{
    uint8_t brightness; // 明るさ(全体) (0-255)
    uint16_t H1;        // 色1の色相 (0-255)
    uint8_t  S1;        // 色1の彩度 (0-255)
    uint16_t H2;        // 色2の色相 (0-255)
    uint8_t  S2;        // 色2の彩度 (0-255)
    int T_2color;       // ふたいろの周期 [ms]
    int T_fade;         // ほたるの周期 [ms]
    int T_round;        // ぐるぐるの周期 [ms]
    int T_fluct;        // ゆらめきの更新周期 [ms]
    float dC;           // 色のゆらめき     (0.0 - 1.0)
    float dV;           // 明るさのゆらめき (0.0 - 1.0)
    Iluminetion pattern; // 発光パターン
    uint16_t magicNumber; // 有効データ確認用マジックナンバー
} PrefsData;

#define MAGIC_NUMBER  0xAA55 // 有効データ確認用マジックナンバー

// 設定値の初期値
static const PrefsData INIT_DATA = {
    .brightness = 32,
    .H1         = 0x0000,
    .S1         = 255,
    .H2         = 0x1000,
    .S2         = 255,
    .T_2color   = 4000,
    .T_fade     = 4000,
    .T_round    = 2000,
    .T_fluct    = 30,
    .dC         = 0.5F,
    .dV         = 0.4F,
    .pattern    = PTN_ONE_COLOR,
    .magicNumber = MAGIC_NUMBER
};

// コンストラクタ
NeoPixelCtrl::NeoPixelCtrl() : 
    pixels( Adafruit_NeoPixel(LED_MAX, LED_PIN, NEO_GRB + NEO_KHZ800) )
{
    n_cnt = 0;
}

// 明るさの設定 (0-255)
void NeoPixelCtrl::setBrightness(uint8_t brightness)
{
    pixels.setBrightness(brightness);
    this->brightness = brightness;
}

// 色1の設定 (色相 0x0000-0xFFFF, 彩度 0-255)
void NeoPixelCtrl::setColor1(uint16_t h, uint8_t s)
{
    H1 = h;
    S1 = s;
}

// 色2の設定 (色相 0x0000-0xFFFF, 彩度 0-255)
void NeoPixelCtrl::setColor2(uint16_t h, uint8_t s)
{
    H2 = h;
    S2 = s;
}

// ふたいろの周期の設定 [ms]
void NeoPixelCtrl::setT_2color(int ms)
{
    if(ms > 0 && ms <= 10000) T_2color = ms;
}

// ほたるの周期の設定 [ms]
void NeoPixelCtrl::setT_fade  (int ms)
{
    if(ms > 0 && ms <= 10000) T_fade = ms; 
}

// ぐるぐるの周期の設定 [ms]
void NeoPixelCtrl::setT_round (int ms)
{
    if(ms > 0 && ms <= 10000) T_round = ms;
}

// ゆらめきの更新周期の設定 [ms]
void NeoPixelCtrl::setT_fluct (int ms)
{
    if(ms > 0 && ms <= 10000) T_fluct = ms;
}

//// ゆらめきの設定(色相 0-50%, 彩度 0-100%, 明度 0-100%)
//void NeoPixelCtrl::setFluctuation(int h, int s, int v)
// ゆらめきの設定(色 0-100%, 明るさ 0-100%)
void NeoPixelCtrl::setFluctuation(int c, int v)
{
//  if(h >= 0 && h <= 100) dH = (float)h / 100.0F;
//  if(s >= 0 && s <= 100) dS = (float)s / 100.0F;
    if(c >= 0 && c <= 100) dC = (float)c / 100.0F;
    if(v >= 0 && v <= 100) dV = (float)v / 100.0F;
}

// 発光パターンの設定
void NeoPixelCtrl::setPattern (Iluminetion pattern)
{
    n_cnt = 0;
    this->pattern = pattern;
}

// 設定のセーブ
void NeoPixelCtrl::save()
{
    Serial.println("Saving...");
    
    PrefsData data;
    data.brightness = brightness;
    data.H1         = H1;
    data.S1         = S1;
    data.H2         = H2;
    data.S2         = S2;
    data.T_2color   = T_2color;
    data.T_fade     = T_fade;
    data.T_round    = T_round;
    data.T_fluct    = T_fluct;
    data.dC         = dC;
    data.dV         = dV;
    data.pattern    = pattern;
    data.magicNumber = MAGIC_NUMBER;
    
    int rc = prefs.writePrefs(&data, sizeof(data));
    Serial.println((rc == FDS_SUCCESS) ? "Save OK" : "Save ERROR");
    printSettings();
}

// 設定のリセット
void NeoPixelCtrl::reset()
{
    Serial.println("Resetting...");
    
    brightness = INIT_DATA.brightness;
    H1         = INIT_DATA.H1;
    S1         = INIT_DATA.S1;
    H2         = INIT_DATA.H2;
    S2         = INIT_DATA.S2;
    T_2color   = INIT_DATA.T_2color;
    T_fade     = INIT_DATA.T_fade;
    T_round    = INIT_DATA.T_round;
    T_fluct    = INIT_DATA.T_fluct;
    dC         = INIT_DATA.dC;
    dV         = INIT_DATA.dV;
    pattern    = INIT_DATA.pattern;
    
    int rc = prefs.writePrefs((void*)(&INIT_DATA), sizeof(INIT_DATA));
    Serial.println((rc == FDS_SUCCESS) ? "Reset OK" : "Reset ERROR");
    printSettings();
}

// 設定のロード
void NeoPixelCtrl::load()
{
    Serial.println("Loading...");
    PrefsData data; 
    int rc = prefs.readPrefs(&data, sizeof(data));
    if (rc == FDS_SUCCESS)
    {
        if(data.magicNumber == MAGIC_NUMBER){
            Serial.println("Load OK");
            brightness = data.brightness;
            H1         = data.H1;
            S1         = data.S1;
            H2         = data.H2;
            S2         = data.S2;
            T_2color   = data.T_2color;
            T_fade     = data.T_fade;
            T_round    = data.T_round;
            T_fluct    = data.T_fluct;
            dC         = data.dC;
            dV         = data.dV;
            pattern    = data.pattern;
            
            printSettings();
        }else{
            Serial.println("No Data found");
            // 既定値で初期化
            this->reset();
        }
    }else{
        Serial.print("Load Error");
        Serial.print(rc); Serial.print(", ");
        Serial.println(prefs.errorString(rc));
        // 既定値で初期化
        this->reset();
    }
}

// 初期化
void NeoPixelCtrl::begin()
{
    // 保存データ読み出し
    this->load();
    
    // NeoPixelの電源制御
#ifdef POWER_ON_OFF
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, LOW);
#endif
    
    // NeoPixelの初期化
    pixels.begin();
    pixels.setBrightness(brightness);
    
    // ゆらぎ発生器の初期化
    for(int i=0;i<LED_MAX;i++){
        for(int j=0;j<F_HSV;j++){
            fluct[i][j].init();
        }
    }
    // 周期タイマの初期化
    interval.set(DELTA_T);
}

// タスク
void NeoPixelCtrl::task()
{
    // 更新周期ごとに処理
    if(interval.elapsed())
    {
        switch(pattern){
        case PTN_OFF:
            patternOff();       // けす
            break;
        case PTN_ONE_COLOR:
            patternOneColor();  // ひといろ
            break;
        case PTN_TWO_COLOR:
            patternTwoColor();  // ふたいろ
            break;
        case PTN_FADE:
            patternFade();      // ほたる
            break;
        case PTN_ROUND:
            patternRound();     // ぐるぐる
            break;
        case PTN_FLUCTUATION:
            patternFluction();  // ゆらめき
            break;
        }
        // LEDの色更新
        pixels.show();
        // 回数カウント
        n_cnt++;
    }
}

// けす
void NeoPixelCtrl::patternOff()
{
    for(int i=0;i<LED_MAX;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
}

// ひといろ
void NeoPixelCtrl::patternOneColor()
{
    for(int i=0;i<LED_MAX;i++){
        pixels.setPixelColor(i, pixels.ColorHSV(H1,S1,255));
    }
}

// ふたいろ
void NeoPixelCtrl::patternTwoColor()
{
    // ratio = 256 ～ 0 ～ 256 (周期 T_2color)
    int ratio = abs(-256 + 512 * ((DELTA_T * n_cnt) % T_2color) / T_2color);
    
    // 2色の中間色
    int h = (H1*ratio + H2*(256 - ratio)) / 256;
    int s = (S1*ratio + S2*(256 - ratio)) / 256;
    
    for(int i=0;i<LED_MAX;i++){
        pixels.setPixelColor(i, pixels.ColorHSV(h,s,255));
    }
}

// ほたる
void NeoPixelCtrl::patternFade()
{
    // fade = 255 ～ 0 ～ 255 (周期 T_fade)
    int fade = abs(-255 + 510 * ((DELTA_T * n_cnt) % T_fade) / T_fade);
    
    for(int i=0;i<LED_MAX;i++){
        pixels.setPixelColor(i, pixels.ColorHSV(H1,S1,fade));
    }
}

// ぐるぐる
void NeoPixelCtrl::patternRound()
{
#if 0
    // offset = 0 ～ LED_MAX-1 (周期 T_round)
    int offset = LED_MAX * ((DELTA_T * n_cnt) % T_round) / T_round;
    
    for(int i=0;i<LED_MAX;i++){
        
        int ratio = abs(2 * ((i + offset) % LED_MAX) - LED_MAX);
        
        // 2色の中間色
        int h = (H1*ratio + H2*(LED_MAX - ratio)) / LED_MAX;
        int s = (S1*ratio + S2*(LED_MAX - ratio)) / LED_MAX;
        pixels.setPixelColor(i, pixels.ColorHSV(h,s,255));
    }
#else
    // ratio = 256 ～ 0 ～ 256 (周期 T_round)
    int ratio = abs(-256 + 512 * ((DELTA_T * n_cnt) % T_round) / T_round);
    
    for(int i=0;i<LED_MAX;i++){
        
        // ratio = 256 ～ 0 ～ 256 (周期 T_round)
        int offset = (T_round * i) / LED_MAX;
        int ratio = abs(-256 + 512 * ((DELTA_T * n_cnt + offset) % T_round) / T_round);
        
        // 2色の中間色
        int h = (H1*ratio + H2*(256 - ratio)) / 256;
        int s = (S1*ratio + S2*(256 - ratio)) / 256;
        pixels.setPixelColor(i, pixels.ColorHSV(h,s,255));
    }
#endif
}

// ゆらめき
void NeoPixelCtrl::patternFluction()
{
    static int cnt = 0;
    cnt++;
    if(cnt >= T_fluct / DELTA_T){
        cnt = 0;
        for(int i=0;i<LED_MAX;i++){
            
            // ゆらぎの計算
//          float fh = fluct[i][F_HUE].calc() * 2.0 - 1.0; // -1.0 ～ 1.0
//          float fs = fluct[i][F_SAT].calc(); // 0.0 ～ 1.0
            float fc = fluct[i][F_COL].calc(); // 0.0 ～ 1.0
            float fv = fluct[i][F_VAL].calc(); // 0.0 ～ 1.0
            
//          uint16_t h = H1 + (int)((float)0x10000 * dH * fh);
//          int s = (int)((float)S1 * (1.0 - dS * fs));
            int h = (int)((float)H1 * (1.0F - dC * fc) + (float)H2 * dC * fc);
            int s = (int)((float)S1 * (1.0F - dC * fc) + (float)S2 * dC * fc);
            int v = (int)( 255.0F   * (1.0F - dV * fv));
            
            pixels.setPixelColor(i, pixels.ColorHSV(h,s,v));
        }
    }
}

// 設定の取得
void NeoPixelCtrl::getParams(
    uint8_t &brightness,
    uint16_t &H1, uint8_t &S1,
    uint16_t &H2, uint8_t &S2,
    int &T_2color, int &T_fade, int &T_round, int &T_fluct,
    float &dC, float &dV,
    Iluminetion &pattern
    )
{
    brightness  = this->brightness;
    H1          = this->H1;
    S1          = this->S1;
    H2          = this->H2;
    S2          = this->S2;
    T_2color    = this->T_2color;
    T_fade      = this->T_fade;
    T_round     = this->T_round;
    T_fluct     = this->T_fluct;
    dC          = this->dC;
    dV          = this->dV;
    pattern     = this->pattern;
}

// デバッグ用
void NeoPixelCtrl::printSettings()
{
    Serial.println(brightness);
    Serial.println(H1);
    Serial.println(S1);
    Serial.println(H2);
    Serial.println(S2);
    Serial.println(T_2color);
    Serial.println(T_fade);
    Serial.println(T_round);
    Serial.println(T_fluct);
    Serial.println(dC);
    Serial.println(dV);
    Serial.println(pattern);
}
