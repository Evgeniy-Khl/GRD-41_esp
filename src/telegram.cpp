/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#include <Arduino.h>
#include <MyTelegramBot.h>
#include "main.h"
#include "server.h"
// Определите язык
// #define LANGUAGE_EN  // Для английского
#define LANGUAGE_UA  // Для русского

#ifdef LANGUAGE_EN
#include "strings_en.h"
#elif defined(LANGUAGE_UA)
#include "strings_ua.h"
#endif

extern char botToken[], chatID [], nameID [];
extern MyTelegramBot bot;
extern uint16_t speedFan[];
bool botSetup(){
  return bot.setMyCommands(MAIN_MENU);
}

/******************************************************************
* errors = 0x01   // ОШИБКА ДАТЧИКА 0  199-потерян; 66,0-завис [E01]
* errors = 0x02   // ОШИБКА ДАТЧИКА 1  199-потерян; 66,0-завис [E02]
* errors = 0x04   // ОТКЛОНЕНИЕ КАНАЛ 0 [E04]
* errors = 0x08   // ОТКЛОНЕНИЕ КАНАЛ 1 [E08]
* errors = 0x10   // отказ одного из двух датчиков температуры
* errors = 0x20   // отказ вспомогательного датчика температуры
* errors = 0x80   // ПЕРЕГРЕВ СИМИСТОРА ! [ПРГ] 
*******************************************************************/
void sendErrMessages(int err){
  String errMess = WORD_TITLE + String(upv.pv.model) + ID_TITLE + String(nameID) + NEW_STR + NEW_STR;
  if(upv.pv.errors&1) errMess += SENSOR_ERROR_1;
  else {
    if(upv.pv.errors&4){
      errMess += SENSOR_ERROR_4;
      errMess += WORD_CHAMBER + getFloat((float)upv.pv.t[0]/10,0) + NEW_STR;
    }
  }
  if(upv.pv.errors&2) errMess += SENSOR_ERROR_2;
  else {
    if(upv.pv.errors&8){
      errMess += SENSOR_ERROR_8;
      errMess += WORD_PRODUCT + getFloat((float)upv.pv.t[1]/10,0) + NEW_STR;
    }
  }
  errMess += GRAVE_ACCENT;
  bot.sendMessage(chatID, errMess, "Markdown");
  // String keyboardJson = "[[{ \"text\" : \"Get a report\",  \"callback_data\" : \"/status\" }],[{ \"text\" : \"Help\", \"callback_data\" : \"/start\" }]]";
  // bot.sendMessageWithInlineKeyboard(chatID, errMess, "Markdown", keyboardJson);
}

void sendStatus(String chatid){
  String welcome = WORD_TITLE + String(upv.pv.model) + ID_TITLE + String(nameID) + NEW_STR + NEW_STR;
  welcome += WORD_STATUS;
  if(upv.pv.portFlag & 4) welcome += WORD_WORK;
  else welcome += WORD_STOP;
  welcome += NEW_STR;
  welcome += WORD_MODE;
  switch (upv.pv.modeCell){
    case 0: welcome += WORD_MODE0; break;
    case 1: welcome += WORD_MODE1; break;
    case 2: welcome += WORD_MODE2; break;
    case 3: welcome += WORD_MODE3; break;
    default: welcome += NEW_STR; break;
  }
  welcome += NEW_STR;
  welcome += WORD_CHAMBER + getFloat((float)upv.pv.t[0]/10,0);
  welcome += getFloat((float)upv.pv.set[0]/1,1) + NEW_STR;
  welcome += WORD_PRODUCT + getFloat((float)upv.pv.t[1]/10,0);
  welcome += getFloat((float)upv.pv.set[1]/1,1) + NEW_STR;
  welcome += WORD_SMOKE   + getFloat((float)upv.pv.t[2]/10,0) + NEW_STR;
  welcome += WORD_HUMID   + getFloat((float)upv.pv.t[3]/10,0) + NEW_STR;
  welcome += WORD_FANSPEED;
  if(upv.pv.portFlag & 2) welcome += String(speedFan[upv.pv.set[VENT]]) + WORD_RPM;
  else welcome += WORD_STOP;
  welcome += NEW_STR;
  if(upv.pv.portFlag & 4){
    char time[12];
    sprintf(time,"%02d:%02d:%02d", upv.pv.currHour, upv.pv.currMin, upv.pv.currSec);
    welcome += WORD_DURATION + String(upv.pv.set[TMR0]/60) + WORD_HOURS + String(upv.pv.set[TMR0]%60) + WORD_MINUTS + NEW_STR;
    welcome += WORD_TIME + String(time) + NEW_STR;
    welcome += WORD_HEATING + String(upv.pv.dsplPW) + WORD_PCT + NEW_STR;
  }
  if(upv.pv.errors) welcome += WORD_MISTAKES + String(upv.pv.errors) + NEW_STR;
  else {welcome += WORD_MISTAKES;  welcome += WORD_NONE; welcome += NEW_STR;}
  welcome += GRAVE_ACCENT;
  bot.sendMessage(chatid, welcome, "Markdown");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
    Serial.println("handleNewMessages(): "+String(numNewMessages));
    for (int i=0; i<numNewMessages; i++) {
        // Chat id of the requester
        String chat_id = String(bot.messages[i].chat_id);
        // if (chat_id != chatID){
        //   bot.sendMessage(chat_id, "Unauthorized user", "");
        //   continue;
        // }
        
        // Print the received message
        String text = bot.messages[i].text;
        Serial.println("received message: " + text);
    
        // String from_name = bot.messages[i].from_name;
    
        // if (text == TXT_START) {
        //   String welcome = "Welcome, " + from_name + ".\n";
        //   welcome += "Use the following commands to control your outputs.\n\n";
        //   welcome += "/led_on to turn GPIO ON \n";
        //   welcome += "/led_off to turn GPIO OFF \n";
        //   welcome += "/state to request current GPIO state \n";
        //   bot.sendMessage(chat_id, welcome, "");
        // }
        // if (text == TXT_OPTIONS){
        //   String keyboardJson = "[[{ \"text\" : \"Go to Graviton\", \"url\" : \"https://graviton.com.ua/ua/\" }],[{ \"text\" : \"Send\", \"callback_data\" : \"/start\" }]]";
        //   bot.sendMessageWithInlineKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson);
        // }
        if (text == TXT_STATUS) sendStatus(chat_id);
    }
}
