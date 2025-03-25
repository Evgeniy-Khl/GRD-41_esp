// server.cpp
#include <ArduinoJson.h>
#include "main.h"
// Определите язык
// #define LANGUAGE_EN  // Для английского
#define LANGUAGE_UA  // Для русского

#ifdef LANGUAGE_EN
#include "strings_en.h"
#elif defined(LANGUAGE_UA)
#include "strings_ua.h"
#endif


extern char chatID [], nameID [];
extern uint8_t seconds, mode, status;
extern long lastSendTime;
extern int8_t tmrTelegramOff;
extern uint16_t speedFan[];
extern ESP8266WebServer server;

void notFoundHandler() {
  server.send(404, "text/plain", "Not found");
}

String getFloat(float val, uint8_t brackets) {
  char buffer[8];
  if(brackets) snprintf(buffer, sizeof(buffer), "[%.1f]", val);
  else snprintf(buffer, sizeof(buffer), "%.1f", val);
  for (unsigned int i = 0; i < sizeof(buffer); i++) {
    if (buffer[i] == '.') {
      buffer[i] = ',';
      break;
    }
  }
  return String(buffer); // Возвращаем отформатированную строку
}

void respondsValues() {
    String string, jsonResponse;
    tmrTelegramOff = 120;
    JsonDocument data;
    data["model"] = "GRD Max4." + String(upv.pv.model) + "&nbsp;&nbsp;&nbsp;ID:&nbsp;" + String(nameID);
    if(upv.pv.portFlag & 4) data["status"] = WORD_WORK;
    else data["status"] = WORD_STOP;
    switch (upv.pv.modeCell){
      case 0: data["modeCell"] = WORD_MODE0; break;
      case 1: data["modeCell"] = WORD_MODE1; break;
      case 2: data["modeCell"] = WORD_MODE2; break;
      case 3: data["modeCell"] = WORD_MODE3; break;
      default: data["modeCell"] = ""; break;
    }
    data["temperature0"] = getFloat((float)upv.pv.t[0]/10,0);
    data["temperature1"] = getFloat((float)upv.pv.t[1]/10,0);
    data["settemp0"] = getFloat((float)upv.pv.set[0]/1,1);
    data["settemp1"] = getFloat((float)upv.pv.set[1]/1,1);
    data["temperature2"] = getFloat((float)upv.pv.t[2]/10,0);
    data["temperature3"] = getFloat((float)upv.pv.t[3]/10,0);
    uint8_t speed = upv.pv.set[VENT];
    if(upv.pv.portFlag & 2) data["fanSpeed"] = String(speedFan[speed]) + WORD_RPM;
    else data["fanSpeed"] = WORD_STOP;
    data["duration"] = String(upv.pv.set[TMR0]/60) + WORD_HOURS + String(upv.pv.set[TMR0]%60) + WORD_MINUTS;
    if(upv.pv.portFlag & 4){
      char time[12];
      sprintf(time,"%02d:%02d:%02d", upv.pv.currHour, upv.pv.currMin, upv.pv.currSec);
      data["time"] = String(time);
      data["power"] = String(upv.pv.dsplPW) + WORD_PCT;
    } else {
      data["time"] = WORD_STOP;
      data["power"] = String(0) + WORD_PCT;
    }
    
    if(upv.pv.errors) data["errors"] = String(upv.pv.errors);
    else data["errors"] = WORD_NONE;
    
    serializeJson(data, jsonResponse);
    // Serial.printf("SERVER responds to the client with VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
    // Serial.println("out=" + response);
    server.send(200, "application/json", jsonResponse);
    // Serial.printf("END VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
}

void respondsSet(){
    String jsonResponse;
    JsonDocument doc;
        doc["set0"] = upv.pv.set[0];
        doc["set1"] = upv.pv.set[1];
        doc["set2"] = upv.pv.set[2];
        doc["set3"] = upv.pv.set[3];
        doc["set4"] = upv.pv.set[4];
        doc["set5"] = upv.pv.set[5];
        doc["set6"] = upv.pv.set[6];
        doc["set7"] = upv.pv.set[7];
        doc["set8"] = upv.pv.set[8];
        doc["set9"] = upv.pv.set[9];
        doc["set10"] = upv.pv.set[10];
        doc["set11"] = upv.pv.set[11];
        doc["status"] = upv.pv.portFlag & 4;
        
        serializeJson(doc, jsonResponse); // Сериализуем JSON
        Serial.printf("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        Serial.println(jsonResponse);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
        server.send(200, "application/json", jsonResponse); // Отправляем ответ
        // Serial.printf("END EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
}

void acceptSet() {
  // Логирование всех параметров
  Serial.printf("The SERVER has accepted EEPROM seconds: %d, %ld\n", seconds, millis() - lastSendTime);
  
  for (uint8_t i = 0; i < server.args(); i++) {
      String paramName = server.argName(i);
      String paramValue = server.arg(i);
      
      // Логирование параметров (раскомментируйте, если нужно)
      // Serial.printf("Parameter: %s, Value: %s\n", paramName.c_str(), paramValue.c_str());
      
      if (paramName == "set0") upv.pv.set[0] = paramValue.toInt();
      else if (paramName == "set1") upv.pv.set[1] = paramValue.toInt();
      else if (paramName == "set2") upv.pv.set[2] = paramValue.toInt();
      else if (paramName == "set3") upv.pv.set[3] = paramValue.toInt();
      else if (paramName == "set4") upv.pv.set[4] = paramValue.toInt();
      else if (paramName == "set5") upv.pv.set[5] = paramValue.toInt();
      else if (paramName == "set6") upv.pv.set[6] = paramValue.toInt();
      else if (paramName == "set7") upv.pv.set[7] = paramValue.toInt();
      else if (paramName == "set8") upv.pv.set[8] = paramValue.toInt();
      else if (paramName == "set9") upv.pv.set[9] = paramValue.toInt();
      else if (paramName == "set10") upv.pv.set[10] = paramValue.toInt();
      else if (paramName == "set11") upv.pv.set[11] = paramValue.toInt();
      else if (paramName == "status") status = paramValue.toInt();
  }
  server.send(200); // Отправляем только статус 200

  saveSet(status);
  // if (status) getData(SET_ON); else getData(SET_OFF);
}
