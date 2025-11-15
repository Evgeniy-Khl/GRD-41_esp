/*
  https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/
  https://randomnerdtutorials.com/esp32-esp8266-relay-web-server/
  https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
  https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
  RAM:   [====      ]  44.3% (used 36252 bytes from 81920 bytes)
  Flash: [=====     ]  51.7% (used 539695 bytes from 1044464 bytes)
*/

#ifndef __MAIN_H
#define __MAIN_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MyTelegramBot.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <LittleFS.h>
#include "server.h"
#include "usart.h"
#include "procedure.h"

#define DEBUG

#ifdef DEBUG
  // Вариативные макросы, принимающие любое количество аргументов
  #define DEBUG_SPRINTF(...)  sprintf(__VA_ARGS__)
  #define MYDEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define MYDEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  // "Пустышки" остаются такими же
  #define DEBUG_SPRINTF(...)
  #define MYDEBUG_PRINT(...)
  #define MYDEBUG_PRINTLN(...)
#endif
// --- Конец блока макросов ---

#define MYPORT_TX   12
#define MYPORT_RX   13
#define BUF_CAPACITY 64
#define RAMPV_START_MARKER  0xAA  // Стартовый байт
#define RAMPV_END_MARKER    0x55  // Конечный байт
#define LED_PIN     2 // Или LED_BUILTIN (зависит от платы, 2 - стандарт для NodeMCU)
#define FLASH_DELAY 50 // Задержка для короткой вспышки (в мс)

enum Interval { INTERVAL_1000 = 1000, INTERVAL_4000 = 4000 };
enum {T0, T1, T2, T3, TMR0, VENT, TMON, TMOFF, TMR1, ALRM, HIST, CHILL, INDEX};

struct __attribute__((packed)) Rampv {
    uint8_t model;       // 1 байт модель прибора
    uint8_t id;          // 1 байт сетевой номер прибора
    uint8_t wifi;        // 1 байт подключение к сети
    uint8_t reserve;     // 1 байт резерв
    uint8_t modeCell;    // 1 байт номер режима
    uint8_t portFlag;    // 1 байт Flags CHECK; SPEED; WORK; NEWBUTT; VENTIL; PERFECT; RESERVE; PURGING
    int16_t t[4];        // 8 байт значения датчиков температуры
    uint16_t set[INDEX]; // 24байт Установки
    uint8_t fanSpeed;    // 1 байт скорость вращения вентилятора
    uint8_t pvOut;       // 1 байт активные выходы реле
    uint8_t dsplPW;      // 1 байт мощность подаваемая на тены
    uint16_t errors;     // 1 байт ошибки
    uint8_t currHour;    // 1 байт часы
    uint8_t currMin;     // 1 байт минуты
    uint8_t currSec;     // 1 байт секунды
};
#define RAMPV_SIZE sizeof(Rampv) // определение размера
union Upv{
  uint8_t dataUnion[RAMPV_SIZE]; // Массив для приема
  Rampv pv;
};

extern char botToken[50], chatID [15];
extern bool shouldSaveConfig, wifiEnable;
extern uint8_t mode, myIp[6]; 
extern int8_t tmrTelegramOff;
extern Interval interval;
extern Upv sendData, recvData;
extern ESP8266WebServer server;
extern WiFiClientSecure client;
extern MyTelegramBot bot;


#endif /* MAIN_H */
