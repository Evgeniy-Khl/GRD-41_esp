// server.h
#ifndef _SERVER_H
#define _SERVER_H

// #include <ESPAsyncWebServer.h>
#include <ESP8266WebServer.h>

// Функция для обработки параметров
void notFoundHandler();
void respondsValues();
void respondsSet();
void acceptSet();
String getFloat(float val, uint8_t brackets);
byte calculateChecksum(byte* data, int length);
void OutStatus();

#endif /* _SERVER_H */