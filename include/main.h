#ifndef __MAIN_H
#define __MAIN_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MyTelegramBot.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <LittleFS.h>
#include "server.h"
#include "usart.h"

#define MYPORT_TX 12
#define MYPORT_RX 13
#define RAMPV_SIZE   46
#define BUF_CAPACITY 64

enum Interval { INTERVAL_1000 = 1000, INTERVAL_4000 = 4000 };
enum {T0, T1, T2, T3, TMR0, VENT, TMON, TMOFF, TMR1, ALRM, HIST, CHILL, INDEX};

struct Rampv {
    uint8_t model;       // 1 байт ind=0  модель прибора
    uint8_t node;        // 1 байт ind=1  сетевой номер прибора
    uint8_t modeCell;    // 1 байт ind=2  номер режима
    uint8_t portFlag;    // 1 байт ind=3  Flags CHECK; SPEED; WORK; NEWBUTT; VENTIL; PERFECT; RESERVE; PURGING
    int16_t t[4];        // 8 байт ind=4-ind=11  значения датчиков температуры
    uint16_t set[INDEX]; // 24байт ind=12-ind=35 Установки
    uint8_t fanSpeed;    // 1 байт ind=36 скорость вращения вентилятора
    uint8_t pvOut;       // 1 байт ind=37 активные выходы реле
    uint8_t dsplPW;      // 1 байт ind=38 мощность подаваемая на тены
    uint8_t errors;      // 1 байт ind=39 ошибки
    uint8_t currHour;    // 1 байт ind=40 часы
    uint8_t currMin;     // 1 байт ind=41 минуты
    uint8_t currSec;     // 1 байт ind=42 секунды
};// ------- 42 bytes ------------
union Upv{
  uint8_t receivedData[RAMPV_SIZE]; // Массив для приема
  Rampv pv;
};

extern Interval interval;
extern Upv upv;
extern ESP8266WebServer server;


#endif /* MAIN_H */
