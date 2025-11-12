

#include "main.h"
#include "usart.h"
#include "telegram.h"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

EspSoftwareSerial::UART mySerial;
Upv upv;
ESP8266WebServer server(80);
WiFiClientSecure client;
MyTelegramBot bot(botToken, client);

void setup(){
  #ifdef DEBUG
    Serial.begin(115200);               // Инициализация последовательного порта для отладки
  #endif

  mySerial.begin(4800, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false, BUF_CAPACITY);// bufCapacity = 64 -> 70
  if (!mySerial) {
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
  } 

  //----------------------------------- MOUNTING FS ----------------------------------------
  MYDEBUG_PRINTLN("mounting FS...");
  bool lFS = LittleFS.begin();
  if(lFS) {
    MYDEBUG_PRINTLN("mounted file system");
    listFilesAndSizes();
    //--------------------------------- clean LittleFS, for testing -----------------------
    // **Здесь вы можете разместить LittleFS.format();  но ОЧЕНЬ ВАЖНО ПОНИМАТЬ КОГДА ЭТО ДЕЛАТЬ!**
    // Например, вы можете отформатировать файловую систему только при первом запуске или при определенном условии.
    // **ВНИМАНИЕ: Раскомментирование следующей строки приведет к форматированию LittleFS при каждом запуске!**
    // Проверка и форматирование, если необходимо
    // if (LittleFS.format()) {
    //   MYDEBUG_PRINTLN("LittleFS formatted successfully");
    // } else {
    //   MYDEBUG_PRINTLN("Failed to format LittleFS");
    // }
    //--------------------- checkSetpoint ----------------------------------
    // dataLed[2] = checkSetpoint();
    // dataLed[3] = checkConfig();
  } else {
    MYDEBUG_PRINTLN("failed to mount FS");
    // dataLed[4] = 1;
  }
  //---------------------------- инициализация WiFiManager -----------------------------------
  if(upv.pv.set[11] & 0x30) initWiFiManag(); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  else MYDEBUG_PRINTLN("Запрет на подключение к WiFi! Продолжаем работу в оффлайн-режиме.");
  //------------------------------------------------------------------------------
    begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
    Serial.printf("Free heap size: %d\n", begHeapSize);
}
 
void loop(){
  server.handleClient(); // Обработка входящих запросов
  long now = millis();
  if (now - lastSendTime > interval) {
    // if(earlyMode != mode){
      // Serial.printf("mode:%d; seconds:%d; All time:%ld; availableBytes:%d\n", mode, seconds, allTime, mySerial.available());
    //   earlyMode = mode;
    // }
    lastSendTime = now;
    // Serial.print("Free heap size: ");
    // Serial.println(system_get_free_heap_size());

    seconds += interval/1000;
    allTime += interval/1000;
    tmrTelegramOff -= interval/1000;  // if you use HTML telegram does not work (5 min.)
    // int16_t lostHeapSize = ESP.getFreeHeap()-begHeapSize;
    // if(lostHeapSize != previousHeapSize){
    //   previousHeapSize = lostHeapSize;
    //   Serial.printf("Lost heap size: %d bytes\n", lostHeapSize);
    // }
    if(mode == 0) getData(GET_VALUES);
  } else {
    readData();
  }
}
