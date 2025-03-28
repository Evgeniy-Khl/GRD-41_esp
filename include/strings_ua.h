// strings_ua.h
#ifndef STRINGS_UA_H
#define STRINGS_UA_H

#include <Arduino.h>

#define MAIN_MENU           F("[ \
    {\"command\":\"help\",  \"description\":\"Отримайте допомогу з використанням бота\"}, \
    {\"command\":\"start\", \"description\":\"Повідомлення яке надсилається, коли ви відкриваєте чат із ботом\"}, \
    {\"command\":\"status\",\"description\":\"Поточний стан пристрою.\"} \
    ]")
#define WORD_MODE0          F("СУШІННЯ")
#define WORD_MODE1          F("ОБЖАРКА")
#define WORD_MODE2          F("ВАРІННЯ")
#define WORD_MODE3          F("КОПЧЕННЯ")
#define WORD_MODE           F("Режим: ")
#define GRAVE_ACCENT        F("```")
#define WORD_TITLE          F("```\n GRD Max4.")
#define ID_TITLE            F("  ID:")
#define WORD_CHAMBER        F("- камера:  ")
#define WORD_PRODUCT        F("- продукт: ")
#define WORD_HUMID          F("- вологий: ")
#define WORD_SMOKE          F("- дим:     ")
#define WORD_HEATING        F("- нагрів: ")
#define WORD_FANSPEED       F("- вентилятор: ")
#define WORD_DURATION       F("- тривалість: ")
#define WORD_TIME           F("- час роботи: ")
#define WORD_MISTAKES       F("- помилки: ")
#define WORD_NONE           F("немає")
#define WORD_PCT            F("%")
// #define OPEN_BRACKET        F(" [")
// #define CLOSE_BRACKET       F(",0]")
#define WORD_RPM            F("об/хвл.")
#define WORD_DAYS           F(" діб")
#define WORD_HOURS          F(" год.")
#define WORD_MINUTS         F(" хвл.")
#define WORD_STATUS         F("Стан: ")
#define WORD_WORK           F("В РОБОТІ")
#define WORD_STOP           F("ЗУПИНЕНО")
#define TXT_START           F("/start")
#define TXT_OPTIONS         F("/options")
#define TXT_STATUS          F("/status")

#define STR_CONNECTING      F("Підключення до сервера")
#define STR_DATA_RETRIEVED  F("Дані успішно отримані")
#define STR_ERROR           F("Сталася помилка")
#define SENSOR_ERROR_1      F("ПОМИЛКА ДАТЧИКА №1\n")
#define SENSOR_ERROR_2      F("ПОМИЛКА ДАТЧИКА №2\n")
#define SENSOR_ERROR_4      F("ВІДХИЛЕННЯ ТЕМПЕРАТУРИ №1\n")
#define SENSOR_ERROR_8      F("ВІДХИЛЕННЯ ТЕМПЕРАТУРИ №2\n")
#define NEW_STR             F("\n")

#endif // STRINGS_UA_H