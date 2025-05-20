#pragma once

// 音階
#define F_C4    262
#define F_D4    294
#define F_E4    330
#define F_F4    349
#define F_G4    392
#define F_A4    440
#define F_B4    494
#define F_C5    523

// ブザーのパターン
#define BUZZ_POWER_ON    0
#define BUZZ_CONNECT     1
#define BUZZ_DISCONNECT  2
#define BUZZ_HEART_RATE  3
#define BUZZ_POSTURE     4
#define BUZZ_CALIBRATING 5
#define BUZZ_CALIBRATED  6

// ブザーの長さ
#define T_BUZZ          200
#define T_BUZZ_SHORT    100

// ボタン長押しの時間
#define T_LONG_PRESS    1000
#define T_LONG_INTERVAL 10000