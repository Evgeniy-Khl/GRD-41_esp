/*
  https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/
  https://randomnerdtutorials.com/esp32-esp8266-relay-web-server/
  https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
  https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

// Import required libraries
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MyTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: 
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <LittleFS.h>
#include "main.h"
#include "usart.h"
#include "telegram.h"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

EspSoftwareSerial::UART mySerial;
Upv upv;
// AsyncWebServer server(80);      // Create AsyncWebServer object on port 80
ESP8266WebServer server(80);

const int ledPin = 2;           // Set LED GPIO
uint16_t speedFan[8] = {1000,1200,1400,1600,1800,2000,2200,2400};

//define your default values here, if there are different values in config.json, they are overwritten.
char botToken[50] = "";  // your Bot Token (Get from Botfather);
char chatID [15] = "";   // your Chat ID (search for “IDBot” or open this link t.me/myidbot in your smartphone.)
WiFiClientSecure client;
MyTelegramBot bot(botToken, client);

//flag for saving data
bool shouldSaveConfig = false;
bool enabledListen = false;

uint8_t receiveBuff[70], transmitBuff[70]; // Массив для приема / передачи по UART
uint8_t earlyMode = 0, mode = READDEFAULT, tmrResetMode = 0, quarter = GET_PROG1, errors, lastError, seconds = 0;
int tableData[32][4] = {0}, tmrTelegramOff = 30;
uint16_t begHeapSize, previousHeapSize;
long lastSendTime = 0, allTime = 0; 
Interval interval = INTERVAL_4000;

void listDir(const char *dir) {
  Serial.printf("Directory contents: %s\n", dir);
  fs::Dir root = LittleFS.openDir("/");
  while (root.next()) {
    Serial.print("  ");
    if (root.isDirectory()) {
        Serial.print("DIR : ");
    } else {
        Serial.print("FILE: ");
    }
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.println(root.fileSize());
  }
}

void setup(){
    Serial.begin(115200);				// Serial port for debugging purposes
    Serial.println();
    #ifdef ESP8266
      configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
      client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
    #endif
    mySerial.begin(4800, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false, 70);// bufCapacity = 64 -> 70
    if (!mySerial) {
      Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    } 

    //read configuration from FS json
    Serial.println("------mounting FS...------");

    if (LittleFS.begin()) {
      Serial.println("mounted file system");
      if (LittleFS.exists("/config.json")) {
        //file exists, reading and loading
        Serial.println("reading config file");
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          Serial.println("opened config file");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);

          configFile.readBytes(buf.get(), size);

          JsonDocument json;
          auto deserializeError = deserializeJson(json, buf.get());
          serializeJson(json, Serial);
          if ( ! deserializeError ) {
            Serial.println("\nparsed json");
            strcpy(botToken, json["botToken"]);
            strcpy(chatID, json["chatID"]);
          } else {
            Serial.println("failed to load json config");
          }
          configFile.close();
        }
      }
    } else {
      Serial.println("failed to mount FS");
    }
//---------------------------------------------------------------------------------------clean LittleFS, for testing-----------------
// **Здесь вы можете разместить LittleFS.format();  но ОЧЕНЬ ВАЖНО ПОНИМАТЬ КОГДА ЭТО ДЕЛАТЬ!**
// Например, вы можете отформатировать файловую систему только при первом запуске или при определенном условии.
// **ВНИМАНИЕ: Раскомментирование следующей строки приведет к форматированию LittleFS при каждом запуске!**
// Проверка и форматирование, если необходимо
    // if (LittleFS.format()) {
    //   Serial.println("LittleFS formatted successfully");
    // } else {
    //   Serial.println("Failed to format LittleFS");
    // }
//-------------------------------------------------------
    // Получение информации о файловой системе
    FSInfo fs_info;
    LittleFS.info(fs_info);
    Serial.printf("Total space: %u bytes\n", fs_info.totalBytes);
    Serial.printf("Used space: %u bytes\n", fs_info.usedBytes);
    Serial.printf("Free space: %u bytes\n", fs_info.totalBytes - fs_info.usedBytes);
    // Просмотр содержимого файловой системы
    listDir("/");
    Serial.println("======== END FS =========");
    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_botToken("botToken", "BOT token", botToken, 50);
    WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID, 11);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_botToken);
    wifiManager.addParameter(&custom_chatID);

    //-------------------------------------------------------------------------------------reset settings - for testing--------------
    // wifiManager.resetSettings();
    //-----------------------------------------------------
    //set minimu quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //------------------------------------------------------------------------- fetches ssid and pass and tries to connect ---------
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("AutoConnectAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.print("connected...yeey   local ip:");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);	// Print ESP32 Local IP Address
    transmitBuff[2] = ip[0]; // Первый байт IP-адреса
    transmitBuff[3] = ip[1]; // Второй байт IP-адреса
    transmitBuff[4] = ip[2]; // Третий байт IP-адреса
    transmitBuff[5] = ip[3]; // Четвертый байт IP-адреса

    //read updated parameters
    strcpy(botToken, custom_botToken.getValue());
    strcpy(chatID, custom_chatID.getValue());
    Serial.println("----The values in the file are ----");
    Serial.println("botToken:" + String(botToken));
    Serial.println("chatID:" + String(chatID));
    Serial.println();
    // Проверяем, что botToken не пустая
    if (strlen(botToken) > 0) {
        bot.updateToken(botToken);
        // if(botSetup()) Serial.println("The command list was updated successfully.");
        bot.sendMessage(chatID, "GRD Max", "");//bot.sendMessage("25235518", "Hola amigo!", "Markdown");
    }
    
    //save the custom parameters to FS
    if (shouldSaveConfig) {
      Serial.println("saving config");
      JsonDocument json;
      json["botToken"] = botToken;
      json["chatID"] = chatID;

      File configFile = LittleFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
      }

      serializeJson(json, Serial);
      serializeJson(json, configFile);
      configFile.close();
      //end save
    }

    pinMode(ledPin, OUTPUT);

    server.on("/", HTTP_GET, []() {
      mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 300;
      if (!LittleFS.exists("/index.html")) {
        Serial.println("index.html not found");
      } else {
        File file = LittleFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "I can't open the index.html");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
      }
    });
    server.on("/setup", HTTP_GET, []() {
      // Serial.printf("/setup ----- EEPROM size: %d;  time: %d,%ld\n", EEPROM_SIZE,seconds,millis()-lastSendTime);
      if (!LittleFS.exists("/index.html")) {
        Serial.println("index.html not found");
      } else {
        File file = LittleFS.open("/setup.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found");
            return;
        }
      server.streamFile(file, "text/html");
      file.close();
      }
    });
    server.on("/getvalues", HTTP_GET, respondsValues);      // the server responds the completed index.html to the client
    server.on("/geteeprom", HTTP_GET, respondsEeprom);      // the server responds the completed setup.html to the client
    server.on("/seteeprom", HTTP_POST, acceptEeprom);       // the server accepts the edited setup.html from the client
    server.onNotFound(notFoundHandler);
    
    server.begin();   // Start server
    Serial.println("HTTP server started");
    
    begHeapSize = ESP.getFreeHeap();    // Проверка доступной памяти
    Serial.printf("Free heap size: %d\n", begHeapSize);
}
 
void loop(){
  server.handleClient(); // Обработка входящих запросов
  long now = millis();
  if (now - lastSendTime > interval) {
    // if(earlyMode != mode){
      Serial.printf("mode:%d; seconds:%d; All time:%ld; availableBytes:%d\n", mode, seconds, allTime, mySerial.available());
    //   earlyMode = mode;
    // }
    lastSendTime = now;
    Serial.print("Free heap size: ");
    Serial.println(system_get_free_heap_size());

    seconds += interval/1000;
    allTime += interval/1000;
    tmrTelegramOff -= interval/1000;  // if you use HTML telegram does not work (5 min.)
    // int16_t lostHeapSize = ESP.getFreeHeap()-begHeapSize;
    // if(lostHeapSize != previousHeapSize){
    //   previousHeapSize = lostHeapSize;
    //   Serial.printf("Lost heap size: %d bytes\n", lostHeapSize);
    // }
    getData(GET_VALUES);
  } else {
    readData();
  }
}