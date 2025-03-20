// strings_en.h
#ifndef STRINGS_EN_H
#define STRINGS_EN_H

#include <Arduino.h>

#define MAIN_MENU           F("[ \
    {\"command\":\"help\",  \"description\":\"Get help using the bot\"}, \
    {\"command\":\"start\", \"description\":\"A message is sent when you open a chat with a bot\"}, \
    {\"command\":\"status\",\"description\":\"Current device status.\"} \
    ]")
#define WORD_MODE0          F("DRYING")
#define WORD_MODE1          F("ROASTING")
#define WORD_MODE2          F("COOKING")
#define WORD_MODE3          F("SMOKING")
#define WORD_MODE           F("Mode: ")
#define GRAVE_ACCENT        F("```")
#define WORD_TITLE          F("```\n GRD Max4.")
#define ID_TITLE            F("  ID:")
#define WORD_AIR            F("- air: ")
#define WORD_PRODUCT        F("- product: ")
#define WORD_HUMIDITY       F("- humidity:  ")
#define WORD_SMOKE          F("- smoke: ")
#define WORD_HEATING        F("- heating: ")
#define WORD_FANSPEED       F("- fanSpeed: ")
#define WORD_TOTAL          F("- total: ")
#define WORD_DURATION       F("- duration: ")
#define WORD_TIME           F("- time: ")
#define WORD_MISTAKES       F("- mistakes: ")
#define WORD_NO             F(" none")
#define WORD_PCT            F("%")
#define WORD_COLON          F(":")
#define WORD_RPM            F("rpm.")
#define WORD_DAYS           F(" days")
#define WORD_HOURS          F(" hrs ")
#define WORD_MINUTS         F(" mins")
#define WORD_STATUS         F("STATUS: ")
#define WORD_WORK           F("at work")
#define WORD_STOP           F("stopped")
#define TXT_START           F("/start")
#define TXT_OPTIONS         F("/options")
#define TXT_STATUS          F("/status")

#define STR_CONNECTING      F("Connecting to server")
#define STR_DATA_RETRIEVED  F("Data retrieved successfully")
#define STR_ERROR           F("An error occurred")
#define SENSOR_ERROR_1      F("SENSOR ERROR #1\n")
#define SENSOR_ERROR_2      F("SENSOR ERROR #2\n")
#define SENSOR_ERROR_4      F("TEMPERATURE DEVIATION #1\n")
#define SENSOR_ERROR_8      F("TEMPERATURE DEVIATION #2\n")
#define NEW_STR             F("\n")

#endif // STRINGS_EN_H