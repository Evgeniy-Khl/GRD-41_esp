/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#include <main.h>
#include "server.h"

// Определите язык
#define LANGUAGE_EN  // Для английского
// #define LANGUAGE_UA  // Для русского

#ifdef LANGUAGE_EN
#include "strings_en.h"
#elif defined(LANGUAGE_UA)
#include "strings_ua.h"
#endif

extern char botToken[], chatID [];
extern MyTelegramBot bot;

bool botSetup(){
  return bot.setMyCommands(MAIN_MENU);
}

/******************************************************************
* errors = 0x01   // ОШИБКА ДАТЧИКА 0  199-потерян; 66,0-завис [E01]
* errors = 0x02   // ОШИБКА ДАТЧИКА 1  199-потерян; 66,0-завис [E02]
* errors = 0x04   // ОТКЛОНЕНИЕ КАНАЛ 0 [E04]
* errors = 0x08   // ОТКЛОНЕНИЕ КАНАЛ 1 [E08]
*******************************************************************/
void sendErrMessages(int err){
  String errMess = WORD_TITLE + String(2) + "sendErrMessages()" + NEW_STR + NEW_STR;
  errMess += "```";
  bot.sendMessage(chatID, errMess, "Markdown");
  // String keyboardJson = "[[{ \"text\" : \"Get a report\",  \"callback_data\" : \"/status\" }],[{ \"text\" : \"Help\", \"callback_data\" : \"/start\" }]]";
  // bot.sendMessageWithInlineKeyboard(chatID, errMess, "Markdown", keyboardJson);
}

void sendStatus() {
    String statusMess = WORD_TITLE + String(2) + "sendStatus()" + NEW_STR + NEW_STR;
  statusMess += "```";
  bot.sendMessage(chatID, statusMess, "Markdown");
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
    MYDEBUG_PRINTLN("handleNewMessages: "+String(numNewMessages));
    for (int i=0; i<numNewMessages; i++) {
        // Chat id of the requester
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != chatID){
          bot.sendMessage(chat_id, "Unauthorized user", "");
          continue;
        }
        
        // Print the received message
        String text = bot.messages[i].text;
        MYDEBUG_PRINTLN("received message: " + text);
    
        String from_name = bot.messages[i].from_name;
    
        if (text == TXT_START) {
          String welcome = "Welcome, " + from_name + ".\n";
          welcome += "Use the following commands to control your outputs.\n\n";
          welcome += "/led_on to turn GPIO ON \n";
          welcome += "/led_off to turn GPIO OFF \n";
          welcome += "/state to request current GPIO state \n";
          bot.sendMessage(chat_id, welcome, "");
        }
        if (text == TXT_OPTIONS){
          String keyboardJson = "[[{ \"text\" : \"Go to Graviton\", \"url\" : \"https://graviton.com.ua/ua/\" }],[{ \"text\" : \"Send\", \"callback_data\" : \"/start\" }]]";
          bot.sendMessageWithInlineKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson);
        }
        if (text == TXT_STATUS) sendStatus();
    }
}
