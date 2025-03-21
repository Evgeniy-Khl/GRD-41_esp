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


extern char chatID [];
extern uint8_t dataLed[5], seconds, mode, quarter;
extern long lastSendTime;
extern int tableData[32][4], tmrTelegramOff, speedFan[];
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
    tmrTelegramOff = 300;
    uint8_t fanSpeed;    // 1 байт ind=36 скорость вращения вентилятора
    uint8_t pvOut;       // 1 байт ind=37 активные выходы реле
    uint8_t dsplPW;      // 1 байт ind=38 мощность подаваемая на тены
    uint8_t errors;      // 1 байт ind=39 ошибки
    uint8_t currHour;    // 1 байт ind=40 часы
    uint8_t currMin;     // 1 байт ind=41 минуты
    uint8_t currSec;     // 1 байт ind=42 секунды
    JsonDocument data;
    data["model"] = "GRD Max4." + String(upv.pv.model) + "&nbsp;&nbsp;&nbsp;ID:" + String(chatID);
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
    data["settemp0"] = String(upv.pv.set[0]);
    data["settemp1"] = String(upv.pv.set[1]);
    data["temperature2"] = getFloat((float)upv.pv.t[2]/10,0);
    data["temperature3"] = getFloat((float)upv.pv.t[3]/10,0);
    if(upv.pv.portFlag & 2) data["fanSpeed"] = String(speedFan[upv.pv.set[VENT]]) + WORD_RPM;
    else data["fanSpeed"] = WORD_STOP;
    if(upv.pv.portFlag & 4){
      char time[12];
      sprintf(time,"%02d:%02d:%02d", upv.pv.currHour, upv.pv.currMin, upv.pv.currSec);
      data["duration"] = String(upv.pv.set[TMR0]/60) + WORD_HOURS + String(upv.pv.set[TMR0]%60) + WORD_MINUTS;
      data["time"] = String(time);
      data["power"] = String(upv.pv.dsplPW) + WORD_PCT;
    }
    
    if(upv.pv.errors) data["errors"] = String(upv.pv.errors);
    else data["errors"] = WORD_NONE;
    
    serializeJson(data, jsonResponse);
    // Serial.printf("SERVER responds to the client with VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
    // Serial.println("out=" + response);
    server.send(200, "application/json", jsonResponse);
    // Serial.printf("END VALUES: %d,%ld\n",seconds,millis()-lastSendTime);
}

/* void respondsEeprom(){
    String jsonResponse;
    JsonDocument doc;

        
        doc["model"] = upv.pv.model;
        
        doc["spT0"] = usp.sp.spT[0];
        doc["spT1"] = usp.sp.spT[1];
        doc["spRH0"] = usp.sp.spRH[0];
        doc["spRH1"] = usp.sp.spRH[1];
        doc["ventMode"] = usp.sp.ventMode;
        doc["extendMode"] = usp.sp.extMode;
        doc["relayMode"] = usp.sp.relayMode;
        doc["checkDry"] = usp.sp.checkDry;
        doc["rotate0"] = usp.sp.rotate[0];
        doc["rotate1"] = usp.sp.rotate[1];
        doc["program"] = usp.sp.program;
        doc["alarm0"] = usp.sp.alarm[0];
        doc["alarm1"] = usp.sp.alarm[1];
        doc["vent0"] = usp.sp.vent[0];
        doc["vent1"] = usp.sp.vent[1];
        doc["extOn"] = usp.sp.extOn;
        doc["extOff"] = usp.sp.extOff;
        doc["minRun"] = usp.sp.minRun;
        doc["maxRun"] = usp.sp.maxRun;
        doc["period"] = usp.sp.period;
        doc["hysteresis"] = usp.sp.hysteresis;
        doc["air0"] = usp.sp.air[0];
        doc["air1"] = usp.sp.air[1];
        doc["flpClose"] = usp.sp.flpClose;
        doc["flpOpen"] = usp.sp.flpOpen;
        doc["flpNow"] = usp.sp.flpNow;
        doc["pkoff0"] = usp.sp.pkoff[0];
        doc["pkoff1"] = usp.sp.pkoff[1];
        doc["ikoff0"] = usp.sp.ikoff[0];
        doc["ikoff1"] = usp.sp.ikoff[1];
        doc["identif"] = usp.sp.identif;
        doc["status"] = usp.sp.status;

        serializeJson(doc, jsonResponse); // Сериализуем JSON
        Serial.printf("SERVER responds to the client with EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
        Serial.println(jsonResponse);
        mode = SAVEEEPROM; interval = INTERVAL_1000;
        server.send(200, "application/json", jsonResponse); // Отправляем ответ
        // Serial.printf("END EEPROM: %d,%ld\n",seconds,millis()-lastSendTime);
}
 */
/* void acceptEeprom() {
  // Логирование всех параметров
  Serial.printf("The SERVER has accepted EEPROM: %d, %ld\n", seconds, millis() - lastSendTime);
  
  for (uint8_t i = 0; i < server.args(); i++) {
      String paramName = server.argName(i);
      String paramValue = server.arg(i);
      
      // Логирование параметров (раскомментируйте, если нужно)
      // Serial.printf("Parameter: %s, Value: %s\n", paramName.c_str(), paramValue.c_str());
      
      if (paramName == "spT0") usp.sp.spT[0] = paramValue.toInt();
      else if (paramName == "spT1") usp.sp.spT[1] = paramValue.toInt();
      else if (paramName == "spRH0") usp.sp.spRH[0] = paramValue.toInt();
      else if (paramName == "spRH1") usp.sp.spRH[1] = paramValue.toInt();
      else if (paramName == "ventMode") usp.sp.ventMode = paramValue.toInt();
      else if (paramName == "extendMode") usp.sp.extMode = paramValue.toInt();
      else if (paramName == "relayMode") usp.sp.relayMode = paramValue.toInt();
      else if (paramName == "checkDry") usp.sp.checkDry = paramValue.toInt();
      else if (paramName == "rotate0") usp.sp.rotate[0] = paramValue.toInt();
      else if (paramName == "rotate1") usp.sp.rotate[1] = paramValue.toInt();
      else if (paramName == "program") usp.sp.program = paramValue.toInt();
      else if (paramName == "alarm0") usp.sp.alarm[0] = paramValue.toInt();
      else if (paramName == "alarm1") usp.sp.alarm[1] = paramValue.toInt();
      else if (paramName == "vent0") usp.sp.vent[0] = paramValue.toInt();
      else if (paramName == "vent1") usp.sp.vent[1] = paramValue.toInt();
      else if (paramName == "extOn") usp.sp.extOn = paramValue.toInt();
      else if (paramName == "extOff") usp.sp.extOff = paramValue.toInt();
      else if (paramName == "minRun") usp.sp.minRun = paramValue.toInt();
      else if (paramName == "maxRun") usp.sp.maxRun = paramValue.toInt();
      else if (paramName == "period") usp.sp.period = paramValue.toInt();
      else if (paramName == "hysteresis") usp.sp.hysteresis = paramValue.toInt();
      else if (paramName == "air0") usp.sp.air[0] = paramValue.toInt();
      else if (paramName == "air1") usp.sp.air[1] = paramValue.toInt();
      else if (paramName == "flpClose") usp.sp.flpClose = paramValue.toInt();
      else if (paramName == "flpOpen") usp.sp.flpOpen = paramValue.toInt();
      else if (paramName == "flpNow") usp.sp.flpNow = paramValue.toInt();
      else if (paramName == "pkoff0") usp.sp.pkoff[0] = paramValue.toInt();
      else if (paramName == "pkoff1") usp.sp.pkoff[1] = paramValue.toInt();
      else if (paramName == "ikoff0") usp.sp.ikoff[0] = paramValue.toInt();
      else if (paramName == "ikoff1") usp.sp.ikoff[1] = paramValue.toInt();
      else if (paramName == "identif") usp.sp.identif = paramValue.toInt();
  }

  server.send(200); // Отправляем только статус 200

  saveEeprom();
} */
