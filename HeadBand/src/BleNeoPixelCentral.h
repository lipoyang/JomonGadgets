#pragma once
#include <ArduinoBLE.h>

// 発光パターン
typedef enum{
    PTN_OFF = 0,    // けす
    PTN_ONE_COLOR,  // ひといろ
    PTN_TWO_COLOR,  // ふたいろ
    PTN_FADE,       // ほたる
    PTN_ROUND,      // ぐるぐる
    PTN_FLUCTUATION // ゆらめき
} Iluminetion;

// BLE-NeoPixcel制御クラス
class BleNeoPixel
{
public:
    void begin();
    void task();
    
    uint8_t getBrightness() const { return brightness; }
    uint16_t getH1() const { return H1; }
    uint16_t getH2() const { return H2; }
    uint8_t  getS1() const { return S1; }
    uint8_t  getS2() const { return S2; }
    uint16_t getT_2color() const { return T_2color; }
    uint16_t getT_fade() const { return T_fade; }
    uint16_t getT_round() const { return T_round; }
    uint16_t getT_fluct() const { return T_fluct; }
    uint8_t getDC() const { return dC; }
    uint8_t getDV() const { return dV; }
    Iluminetion getPattern() const { return pattern; }

    bool setBrightness(uint8_t b){ return setChrU8(chrBrightness, brightness, b); }
    bool setH1(uint16_t h1){ return setChrU16(chrH1, H1, h1); }
    bool setH2(uint16_t h2){ return setChrU16(chrH2, H2, h2); }
    bool setS1(uint8_t s1){  return setChrU8(chrS1, S1, s1); }
    bool setS2(uint8_t s2){  return setChrU8(chrS2, S2, s2); }
    bool setT_2color(uint16_t t_2color){ return setChrU16(chrT_2color, T_2color, t_2color); }
    bool setT_fade(uint16_t t_fade){     return setChrU16(chrT_fade, T_fade, t_fade); }
    bool setT_round(uint16_t t_round){   return setChrU16(chrT_round, T_round, t_round); }
    bool setT_fluct(uint16_t t_fluct){   return setChrU16(chrT_fluct, T_fluct, t_fluct); }
    bool setDC(uint8_t dc){ return setChrU8(chrDC, dC, dc); }
    bool setDV(uint8_t dv){ return setChrU8(chrDV, dV, dv); }
    bool setPattern(Iluminetion ptn){
        uint8_t bPtn;
        if(setChrU8(chrPattern, bPtn, ptn)){
            pattern = (Iluminetion)bPtn;
            return true;
        }else{
            return false;
        }
    }
    bool isConnected() const { return _isConnected; }

private:
    void onConnect();
    bool setChrU8(BLECharacteristic &chr, uint8_t &field, uint8_t value);
    bool setChrU16(BLECharacteristic &chr, uint16_t &field, uint16_t value);

    BLEDevice peripheral;
    bool _isConnected;

    // BLEキャラクタリスティック
    BLECharacteristic chrBrightness; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrH1        ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrS1        ; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrH2        ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrS2        ; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrT_2color  ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrT_fade    ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrT_round   ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrT_fluct   ; // CHR_U16, BLERead | BLEWrite
    BLECharacteristic chrDC        ; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrDV        ; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrPattern   ; // CHR_U8 , BLERead | BLEWrite
    BLECharacteristic chrCommand   ; // CHR_U8 , BLEWrite

    uint8_t brightness;
    uint16_t H1;
    uint16_t H2;
    uint8_t  S1;
    uint8_t  S2;
    uint16_t T_2color;
    uint16_t T_fade;
    uint16_t T_round;
    uint16_t T_fluct;
    uint8_t dC;
    uint8_t dV;
    Iluminetion pattern;
};
