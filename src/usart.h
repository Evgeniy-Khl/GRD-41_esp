#include <Arduino.h>
#include <SoftwareSerial.h>

#ifndef KLIMAT8266_H
#define KLIMAT8266_H
#define START_MARKER 0xDD	// Начало блока = 221

#define READDEFAULT 0
#define READEEPROM  1
#define SAVEEEPROM  2
#define READPROG    3
#define SAVEPROG    4
#define GET_VALUES	0x40    // 64
#define GET_EEPROM	0x41    // 65
#define SET_EEPROM	0x42    // 66

void getData(uint8_t byte);
void readData();
void saveEeprom();
void printData(const char* mess, uint8_t size);
// void sendData(int command, uint8_t *data = nullptr);

#endif // KLIMAT8266_H
