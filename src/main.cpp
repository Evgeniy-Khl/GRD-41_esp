

#include "main.h"
#include "usart.h"
#include "telegram.h"

char botToken[50] = "";  // your Bot Token (Get from Botfather);
char chatID [15] = "";   // your Chat ID (search for “IDBot” or open this link t.me/myidbot in your smartphone.)
//flag for saving data
bool shouldSaveConfig = false;
// Массив для приема / передачи по UART
uint8_t receiveBuff[BUF_CAPACITY], transmitBuff[BUF_CAPACITY], myIp[6]; 
uint8_t earlyMode = 0, mode = READDEFAULT, tmrResetMode = 0, errors, lastError, status, seconds = 0;
int8_t tmrTelegramOff = 30;
uint16_t begHeapSize, previousHeapSize, speedFan[8] = {1000,1200,1400,1600,1800,2000,2200,2400};
long lastSendTime = 0, allTime = 0; 
Interval interval = INTERVAL_4000;

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
