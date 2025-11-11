/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#ifndef _TELEGRAM_H
#define _TELEGRAM_H

bool botSetup();
void sendStatus();
// void sendErrMessages(int err);
void saveConfigCallback();
void handleNewMessages(int numNewMessages);
#endif /* _TELEGRAM_H */
