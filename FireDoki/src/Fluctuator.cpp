// 1/fゆらぎ (間欠カオス法) 計算クラス
#include <Arduino.h>
#include "Fluctuator.h"

// 初期化する
void Fluctuator::init()
{
    x = random(10, 90) / 100.0;
}

// 計算する
// return: 0.0～1.0の範囲で順次ゆらぐ値を返す
float Fluctuator::calc()
{
    if(x < 0.5){
        x = x + 2 * x * x;
    } else {
        x = x - 2 * (1.0 - x) * (1.0 - x);
    }
    if(x < 0.05 || x > 0.995){
        x = random(10, 90) / 100.0;
    }
    return x;
}
