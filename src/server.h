// server.h
#ifndef SERVER_H
#define SERVER_H

// #include <ESPAsyncWebServer.h>
#include <ESP8266WebServer.h>

// Функция для обработки параметров
void notFoundHandler();
void respondsEeprom();
void respondsProgram();
void acceptEeprom();
void acceptProgram();
void respondsValues();
String getFloat(float val, uint8_t brackets);
byte calculateChecksum(byte* data, int length);
void OutStatus();

#endif //SERVER_H