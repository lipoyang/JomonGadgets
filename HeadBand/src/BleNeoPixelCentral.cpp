#include "BleNeoPixelCentral.h"

// BLEサービスのUUID
const char* UUID_svcNeoPixel   = "446FF1A9-5023-26F0-1065-3AA7A53A8483";
// BLEキャラクタリスティックのUUID
const char* UUID_chrBrightness = "8C9C438A-ED7E-1538-488C-FC98E4312F55";
const char* UUID_chrH1         = "602C80A4-B3C0-79A4-8FCE-51B5322BAF8B";
const char* UUID_chrS1         = "962CC211-E355-7B44-EB55-D2503E14B83F";
const char* UUID_chrH2         = "AC2A3C37-5EEC-862C-6B75-97335387531C";
const char* UUID_chrS2         = "1685F91C-ADDA-6B15-496B-039DCBF642DC";
const char* UUID_chrT_2color   = "9D5FD8C0-9DE3-16E4-E89C-DAC40B2F81A2";
const char* UUID_chrT_fade     = "F61AE49D-D036-CD18-F3BA-025D7B93CC6B";
const char* UUID_chrT_round    = "66070B87-2B7F-001B-663F-E1110C37F642";
const char* UUID_chrT_fluct    = "5587D9AB-1927-A85C-A9C1-114DFC660496";
const char* UUID_chrDC         = "6EA7F285-3202-F28A-C609-C48CD759AB90";
const char* UUID_chrDV         = "81765DA4-71CF-79BC-8E1E-A23130995444";
const char* UUID_chrPattern    = "7D5C1067-D1A7-A8E8-9DD0-41CBE5E25F0A";
const char* UUID_chrCommand    = "0CBB4F9C-652E-ABFC-E004-40572A9F55EF";
const char* UUID_chrBPM        = "8654C32F-6ACC-4848-A28F-039D1F8156C9"; //【縄文ガジェット用に追加】
const char* UUID_chrPosture    = "D9A4A2E9-46FB-5D3C-2117-4C845B474766"; //【縄文ガジェット用に追加】

// コマンド定数
const uint8_t CMD_SAVE  = 0x80; // セーブ
const uint8_t CMD_RESET = 0x81; // リセット

// 初期化
void BleNeoPixel::begin()
{
    _isConnected = false;
    
    // BLEの開始
    if (!BLE.begin()) {
        Serial.println("ERROR: starting BLE module failed!");
        while (1);
    }

    // BLEスキャンの開始
    Serial.println("BLE scanning...");
    BLE.scanForUuid(UUID_svcNeoPixel);
}

// タスク
void BleNeoPixel::task()
{
    // 未接続のとき
    if(!_isConnected){
        peripheral = BLE.available();
        if (peripheral)
        {
            if (peripheral.localName() == "NeoPixel") {
                Serial.println("Peripheral found, connectiong...");
                BLE.stopScan();
                // 接続
                if (peripheral.connect()) {
                    if (peripheral.discoverService(UUID_svcNeoPixel)) {
                        Serial.println("BLE connected");
                        _isConnected = true;
                        onConnect(); // 接続時の処理
                    } else {
                        Serial.println("Peripheral connected, but NeoPixel service not found");
                        BLE.disconnect();
                        BLE.scanForUuid(UUID_svcNeoPixel);
                    }
                } else{
                    Serial.print("BLE connection failed");
                    BLE.scanForUuid(UUID_svcNeoPixel);
                }
            }else{
                Serial.print("Peripheral found, but not NeoPixel: ");
                Serial.println(peripheral.localName());
            }
        }
    }
    // 接続中のとき
    else{
        if(peripheral.connected())
        {
            ; 
        }else{
            _isConnected = false;
            Serial.print("BLE disconnected, scanning...");
            BLE.scanForUuid(UUID_svcNeoPixel);
        }
    }
}

// 接続時の処理
void BleNeoPixel::onConnect()
{
    // キャラクタリスティックの取得
    chrBrightness = peripheral.characteristic(UUID_chrBrightness);
    chrH1         = peripheral.characteristic(UUID_chrH1        );
    chrS1         = peripheral.characteristic(UUID_chrS1        );
    chrH2         = peripheral.characteristic(UUID_chrH2        );
    chrS2         = peripheral.characteristic(UUID_chrS2        );
    chrT_2color   = peripheral.characteristic(UUID_chrT_2color  );
    chrT_fade     = peripheral.characteristic(UUID_chrT_fade    );
    chrT_round    = peripheral.characteristic(UUID_chrT_round   );
    chrT_fluct    = peripheral.characteristic(UUID_chrT_fluct   );
    chrDC         = peripheral.characteristic(UUID_chrDC        );
    chrDV         = peripheral.characteristic(UUID_chrDV        );
    chrPattern    = peripheral.characteristic(UUID_chrPattern   );
    chrCommand    = peripheral.characteristic(UUID_chrCommand   );
    chrBPM        = peripheral.characteristic(UUID_chrBPM       ); //【縄文ガジェット用に追加】
    chrPosture    = peripheral.characteristic(UUID_chrPosture   ); //【縄文ガジェット用に追加】

#if 0
    // キャラクタリスティックのREAD
    Serial.println("Reading characteristics...");
    if(chrBrightness.readValue(brightness)) {
        Serial.print("Brightness = ");
        Serial.println(brightness);
    }else{
        Serial.println("chrBrightness read failed");
    }
    if(chrH1.readValue(H1)) {
        Serial.print("H1 = ");
        Serial.println(H1);
    }else{
        Serial.println("chrH1 read failed");
    }
    if(chrS1.readValue(S1)) {
        Serial.print("S1 = ");
        Serial.println(S1);
    }else{
        Serial.println("chrS1 read failed");
    }
    if(chrH2.readValue(H2)) {
        Serial.print("H2 = ");
        Serial.println(H2);
    }else{
        Serial.println("chrH2 read failed");
    }
    if(chrS2.readValue(S2)) {
        Serial.print("S2 = ");
        Serial.println(S2);
    }else{
        Serial.println("chrS2 read failed");
    }
    if(chrT_2color.readValue(T_2color)) {
        Serial.print("T_2color = ");
        Serial.println(T_2color);
    }else{
        Serial.println("chrT_2color read failed");
    }
    if(chrT_fade.readValue(T_fade)) {
        Serial.print("T_fade = ");
        Serial.println(T_fade);
    }else{
        Serial.println("chrT_fade read failed");
    }
    if(chrT_round.readValue(T_round)) {
        Serial.print("T_round = ");
        Serial.println(T_round);
    }else{
        Serial.println("chrT_round read failed");
    }
    if(chrT_fluct.readValue(T_fluct)) {
        Serial.print("T_fluct = ");
        Serial.println(T_fluct);
    }else{
        Serial.println("chrT_fluct read failed");
    }
    if(chrDC.readValue(dC)) {
        Serial.print("dC = ");
        Serial.println(dC);
    }else{
        Serial.println("chrDC read failed");
    }
    if(chrDV.readValue(dV)) {
        Serial.print("dV = ");
        Serial.println(dV);
    }else{
        Serial.println("chrDV read failed");
    }
    uint8_t bPattern;
    if(chrPattern.readValue(bPattern)) {
        pattern = (Iluminetion)bPattern;
        Serial.print("pattern = ");
        Serial.println(pattern);
    }else{
        Serial.println("chrPattern read failed");
    }
#endif
}

bool BleNeoPixel::setChrU8(BLECharacteristic &chr, uint8_t &field, uint8_t value)
{
    if (!_isConnected) {
        Serial.println("Not connected to BLE device");
        return false;
    }
    if(chr.writeValue(value, false)) {
        field = value;
        return true;
    }else{
        Serial.println("Characteristic write failed");
        return false;
    }
}

bool BleNeoPixel::setChrU16(BLECharacteristic &chr, uint16_t &field, uint16_t value)
{
    if (!_isConnected) {
        Serial.println("Not connected to BLE device");
        return false;
    }
    if(chr.writeValue(value, false)) {
        field = value;
        return true;
    }else{
        Serial.println("Characteristic write failed");
        return false;
    }
}
