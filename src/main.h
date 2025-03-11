// main.h
#include <Arduino.h>
#include "server.h"
#include "usart.h"

#ifndef MAIN_H
#define MAIN_H

#define SET1a     40      // (35)*64*0.25= 560 us (0.5 mS)  (0x32)   0 грд.
#define SET1b     85      // (90)*64*0.25= 1440 us (1.4 mS)  (0x5E)  90 грд.
#define MIN_R_M   0

#define MYPORT_TX 13
#define MYPORT_RX 12
#define RAMPV_SIZE   33
#define EEPROM_SIZE  34
#define PROG_SIZE    64

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
    int16_t t[4];        // 8 байт ind=2-ind=9   значения датчиков температуры
    uint16_t set[INDEX];
    uint16_t fan;        // 1 байт ind=11 скорость вращения вентилятора
    uint8_t pvOut;       // 1 байт ind=9  активные выходы реле
    uint8_t timer;       // 1 байт ind=10 значение таймера до начала поворота лотков
    uint8_t power;       // 1 байт ind=13 мощность подаваемая на тены
    uint8_t modeCell;
    uint8_t portFlag;
    uint8_t errors;      // 1 байт ind=15 ошибки
    uint8_t currHour;    // 1 байт часы
    uint8_t currMin;     // 1 байт минуты
    uint8_t currSec;     // 1 байт секунды
};// ------- 20+13 bytes ------------
union Upv{
  uint8_t receivedData[RAMPV_SIZE]; // Массив для приема
  Rampv pv;
};

struct Eeprom {
  int16_t spT[2];     // 4 байт Уставка температуры spT[0]->Сухой датчик; spT[1]->Влажный датчик 
  int8_t  spRH[2];    // 2 байт spRH[0]->ПОДСТРОЙКА HIH-5030/AM2301 spRH[1]->Уставка HIH-5030/AM2301 
  uint8_t ventMode;   // 1 байт 1-ОХЛАЖДЕНИЕ; 2-ОСУШЕНИЕ; 3-ОХЛАЖДЕНИЕ + ОСУШЕНИЕ 
  uint8_t extMode;    // 1 байт 0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ 2-ВСПОМОГАТЕЛЬНЫЙ НАГРЕВАТЕЛЬ
  uint8_t relayMode;  // 1 байт релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->по кан.[1] импульсный режим
  uint8_t checkDry;   // 1 байт задержка регулировки по влажному, пока не установиться по сухому      
  uint8_t rotate[2];  // 2 байт [0]-отключ.состояниe [1]-включ.состояниe
  uint8_t program;   // 1 байт работа по программе
  uint8_t alarm[2];   // 2 байт СИРЕНА
  uint8_t vent[2];    // 2 байт ВЕНТИЛЯЦИЯ 
  uint8_t extOn;      // 1 байт смещение для ВКЛ. вспомогательного канала
  uint8_t extOff;     // 1 байт смещение для ОТКЛ. вспомогательного канала
  uint8_t minRun;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t maxRun;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t period;     // 1 байт импульсное управление помпой увлажнителя
  uint8_t hysteresis; // 1 байт гистерезис канала увлажнения маска 0x03; разрешение использования HIH-5030 маска 0x40; AM2301 маска 0x80;    
  uint8_t air[2];     // 2 байт таймер проветривания air[0]-пауза; air[1]-работа; если air[1]=0-ОТКЛЮЧЕНО
  uint8_t flpClose;   // 1 байт положение залонка ЗАКРЫТА
  uint8_t flpOpen;    // 1 байт положение залонка ОТКРЫТА
  uint8_t flpNow;     // 1 байт текущее положение залонки
  uint8_t pkoff[2];   // 2 байт ind=29;ind=30 пропорциональный коэфф.
  uint8_t ikoff[2];   // 2 байт ind=31;ind=32 интегральный коэфф.
  uint8_t identif;    // 1 байт сетевой номер прибора
  uint8_t status;     // 1 байт не используется
};// --------- 34 bytes --------

union Setting{
  uint8_t receivedData[EEPROM_SIZE]; // Массив для приема
  Eeprom sp;
};

extern Interval interval;
extern Upv upv;
extern ESP8266WebServer server;


#endif // MAIN_H