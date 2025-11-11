#include "procedure.h"

/**
 * @brief Выводит в Serial Monitor список всех файлов и их размеры.
 * Также показывает общую информацию о занятом и свободном месте.
 */
void listFilesAndSizes() {
  Serial.println("\n--- Список файлов в LittleFS ---");
  
  Dir dir = LittleFS.openDir("/");
  long totalSize = 0;
  int fileCount = 0;

  while (dir.next()) {
    // Для каждого элемента получаем объект File
    File entry = dir.openFile("r");
    if (entry) {
      Serial.print("Файл: ");
      Serial.print(entry.name());
      Serial.print("\tРазмер: ");
      Serial.print(entry.size());
      Serial.println(" Байт");
      totalSize += entry.size();
      fileCount++;
      entry.close(); // Важно закрывать файл после использования
    }
  }

  Serial.println("------------------------------------");
  Serial.printf("Всего файлов: %d\n", fileCount);
  Serial.printf("Общий размер: %ld Байт\n", totalSize);

  // Дополнительная информация о файловой системе
  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.printf("Всего места:  %d Байт\n", fs_info.totalBytes);
  Serial.printf("Использовано: %d Байт\n", fs_info.usedBytes);
  Serial.println("------------------------------------");
  // Просмотр содержимого файловой системы
  listDir("/");
}

void initWiFiManag(void){
// The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_botToken("botToken", "BOT token", botToken, 50);
    WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID, 11);
    WiFiManagerParameter custom_nameID("nameID", "Name ID", nameID, 11);

    //WiFiManager https://github.com/tzapu/WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_botToken);
    wifiManager.addParameter(&custom_chatID);
    wifiManager.addParameter(&custom_nameID);

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
    myIp[0] = ip[0]; // Первый байт IP-адреса
    myIp[1] = ip[1]; // Второй байт IP-адреса
    myIp[2] = ip[2]; // Третий байт IP-адреса
    myIp[3] = ip[3]; // Четвертый байт IP-адреса

    //read updated parameters
    strcpy(botToken, custom_botToken.getValue());
    strcpy(chatID, custom_chatID.getValue());
    strcpy(nameID, custom_nameID.getValue());
    myIp[4] = strlen(botToken); // For C-style strings (null-terminated character arrays):
    myIp[5] = strlen(chatID);   // For C-style strings (null-terminated character arrays):
    Serial.println("----The values in the file are ----");
    Serial.println("botToken:" + String(botToken));
    Serial.println("chatID:" + String(chatID));
    Serial.println("nameID:" + String(nameID));
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
      json["nameID"] = nameID;

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
      mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 120;
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
    server.on("/geteeprom", HTTP_GET, respondsSet);      // the server responds the completed setup.html to the client
    server.on("/seteeprom", HTTP_POST, acceptSet);       // the server accepts the edited setup.html from the client
    server.onNotFound(notFoundHandler);
    
    server.begin();   // Start server
    Serial.println("HTTP server started");
}

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

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}