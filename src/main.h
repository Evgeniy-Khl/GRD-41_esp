// main.h
#include <Arduino.h>
#include "server.h"
#include "usart.h"

#ifndef MAIN_H
#define MAIN_H

#define MYPORT_TX 13
#define MYPORT_RX 12
#define RAMPV_SIZE   42

enum Interval { INTERVAL_1000 = 1000, INTERVAL_4000 = 4000 };
enum {T0, T1, T2, T3, TMR0, VENT, TMON, TMOFF, TMR1, ALRM, HIST, CHILL, INDEX};
enum {CHECK, SPEED, WORK, NEWBUTT, VENTIL, PERFECT, RESERVE, PURGING};

// const char* modeName[4] = {"СУШЫННЯ","ОБЖАРКА","ВАРЫННЯ","КОПЧЕННЯ"};

/* switch (errors){
  case 0x01: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N1
  case 0x02: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N2
  case 0x04: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N3
  case 0x08: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N4
  case ERR3: ticBeep = 80; break; // ПЕРЕГРЫВ В КАМЕРI
  case ERR4: ticBeep =120; break; // ПЕРЕГРЫВ В ПРОДУКТI
  case ERR5: ticBeep = 10; break; // ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ
  case ERR6: ticBeep = 20; break; // ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ ДИМA
  case ERR7: ticBeep = 60; break; //
  case ERR8: ticBeep = 60; break; // НЕ ПРАЦЮЭ ВЕНТИЛЯТОР
  default: 
	if(errors==0x0C) ticBeep = 80;
	else ticBeep =200;
  break;
} */

struct Rampv {
    uint8_t model;       // 1 байт ind=0  модель прибора
    uint8_t node;        // 1 байт ind=1  сетевой номер прибора
    uint8_t modeCell;    // 1 байт ind=2  номер режима
    uint8_t portFlag;    // 1 байт ind=3  Flags
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


#endif // MAIN_H