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
#define GET_PROG1	0x50    // 80
#define GET_PROG2	0x51    // 81
#define GET_PROG3	0x52    // 82
#define GET_PROG4	0x53    // 83

#define SET_PROG1	0x30    // 48
#define SET_PROG2	0x31    // 49
#define SET_PROG3	0x32    // 50
#define SET_PROG4	0x33    // 51


void getData(uint8_t byte);
void readData();
void saveEeprom();
void saveProgram(uint8_t quater);
void fillTable(uint8_t quarter);
void fillReceiveBuff(uint8_t quarter);
void printData(const char* mess, uint8_t size);
// void sendData(int command, uint8_t *data = nullptr);

#endif // KLIMAT8266_H
