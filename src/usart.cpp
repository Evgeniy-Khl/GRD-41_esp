#include "main.h"
#include "usart.h"
#include "server.h"
#include "telegram.h"

extern EspSoftwareSerial::UART mySerial;
extern MyTelegramBot bot;
extern long lastSendTime, allTime;
extern char chatID [];
extern uint8_t mode, errors, lastError, seconds, status, receiveBuff[], transmitBuff[], myIp[6];
extern int8_t tmrTelegramOff;

byte calculateChecksum(uint8_t* data, int length) {
    byte checksum = 0;
    for (int i = 1; i < length; i++) {
        checksum ^= data[i]; // Using XOR to calculate the checksum
    }
    return checksum;
}

void sendStruct() { 
    // Передаем байтовый массив и размер
    uint8_t checksum = calculateChecksum(sendData.dataUnion, RAMPV_SIZE);
    
    // Отправляем: [Старт] [Данные] [КС] [Конец]
    mySerial.write(RAMPV_START_MARKER);
    // Используем байтовый массив для отправки
    mySerial.write(sendData.dataUnion, RAMPV_SIZE);
    mySerial.write(checksum);
    mySerial.write(RAMPV_END_MARKER);
    
    Serial.print("Отправлено байт (46 данных + 4 протокол): ");
    Serial.println(RAMPV_SIZE + 4);
}

void getData(uint8_t command){
    uint8_t dataToSend[2];
    dataToSend[0] = START_MARKER;
    dataToSend[1] = command;
    // transmitBuff[2] - transmitBuff[5] -> IP-адрес
    mySerial.write(dataToSend,2);
    mySerial.write(myIp,6);
    mySerial.write(transmitBuff,20);// TOTAL 28 byte
    //if(command != GET_VALUES) DEBUG_PRINTF("---getData()->MODE=%d; COMMAND=%d; secons:%d;  All:%ld sec.\n",mode,command,seconds,allTime);
}

void saveSet(uint8_t stat){
    uint8_t dataToSend[2], crc = 0;
    dataToSend[0] = START_MARKER;
    dataToSend[1] = SET_EEPROM;
    DEBUG_PRINTF("-----------saveEeprom()->%d; seconds: %d,%ld sec.\n",SET_EEPROM,seconds,millis()-lastSendTime);
    for (uint8_t i = 0; i < 2; i++) {
        MYDEBUG_PRINT(dataToSend[i]);
        MYDEBUG_PRINT("; ");
    }
    for (uint8_t i = 12; i < 12+INDEX*2; i++) {
        crc ^= recvData.dataUnion[i];
        MYDEBUG_PRINT(recvData.dataUnion[i]);
        MYDEBUG_PRINT("; ");
    }
    MYDEBUG_PRINT("|| ");
    MYDEBUG_PRINTLN(crc);
    DEBUG_PRINTF("S0:%u; S1:%u; S2:%u; S3:%u; ",recvData.pv.set[0],recvData.pv.set[1],recvData.pv.set[2],recvData.pv.set[3]);
    DEBUG_PRINTF("S4:%u; S5:%u; S6:%u; S7:%u; ",recvData.pv.set[4],recvData.pv.set[5],recvData.pv.set[6],recvData.pv.set[7]);
    DEBUG_PRINTF("S8:%u; S9:%u; S10:%u; S11:%u;\n",recvData.pv.set[8],recvData.pv.set[9],recvData.pv.set[10],recvData.pv.set[11]);
    MYDEBUG_PRINTLN("-----------mySerial.write()");
    
    mySerial.write(dataToSend,2);
    mySerial.write(&recvData.dataUnion[12],INDEX*2);// 24 byte
    mySerial.write(&crc,1);
    crc = stat;             // added status
    mySerial.write(&crc,1); // TOTAL 28 byte
}

// Function for receiving data
void readData(){
    uint8_t availableBytes = mySerial.available();
    switch (mode){
        case READDEFAULT:
            if (availableBytes >= RAMPV_SIZE+2) {
                mySerial.readBytes(receiveBuff, RAMPV_SIZE+2);
                if (receiveBuff[0] == START_MARKER) {
                    byte receivedChecksum = receiveBuff[RAMPV_SIZE+1]; // The last byte is the checksum
                    byte calculatedChecksum = calculateChecksum(receiveBuff, RAMPV_SIZE+1);
                    long readTime = millis();
                    //DEBUG_PRINTF("Read VALUES: %dsec. ",seconds);
                    if (receivedChecksum == calculatedChecksum) {
                        memcpy(recvData.dataUnion, &receiveBuff[1], RAMPV_SIZE);
                        //MYDEBUG_PRINTLN("Valid VALUES.------------------------");
                        if(tmrTelegramOff <= 0){
                            if(recvData.pv.errors && lastError != recvData.pv.errors){
                                sendErrMessages(recvData.pv.errors);
                                lastError = recvData.pv.errors;              // exclude duplicate message
                            } else if (status != (recvData.pv.portFlag & 4)){    // WORK - shutdown flag
                                status = recvData.pv.portFlag&4;
                                sendStatus();
                            } else {                // check incoming messages
                                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                while(numNewMessages) {
                                    MYDEBUG_PRINTLN();
                                    MYDEBUG_PRINT("got response:");
                                    handleNewMessages(numNewMessages);
                                    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                }
                                DEBUG_PRINTF("==============Messages =%d;  %ld msek.=============\n",numNewMessages,millis()-readTime);
                            }
                            //---------------
                        } else {
                            // MYDEBUG_PRINTLN("  Tmr OFF: "+ String(tmrTelegramOff));
                        }
                        // printData("Valid VALUES.",availableBytes);
                    } else {
                        printData("VALUES is corrupted!",availableBytes);
                        mySerial.readBytes(receiveBuff, availableBytes);
                        errors++;
                        mySerial.flush(); // Сбрасываем буфер
                    }
                }else {
                    printData("VALUES No START_MARKER!",availableBytes);
                    mySerial.readBytes(receiveBuff, availableBytes);
                    errors++;
                    mySerial.flush(); // Сбрасываем буфер
                }
            }
            break;
        case SAVEEEPROM:  break;
        default: 
            DEBUG_PRINTF("Default! mode=%d; time: %d,%ld\n", mode,seconds,millis()-lastSendTime);
            break;
    }
}

void printData(const char* mess, uint8_t size){
    MYDEBUG_PRINTLN(mess);
    for (uint8_t i = 0; i < size; i++) {
        MYDEBUG_PRINT(receiveBuff[i]);
        MYDEBUG_PRINT("; ");
        if(i==size-1) MYDEBUG_PRINT("|| ");
    }              
    MYDEBUG_PRINTLN("  size:"+String(size));
    DEBUG_PRINTF("model:%u; node:%u; mode:%u; port:0x%02x; ",recvData.pv.model,recvData.pv.node,recvData.pv.modeCell,recvData.pv.portFlag);
    DEBUG_PRINTF("t0:%3.1f; t1:%3.1f; t2:%3.1f; t3:%3.1f;\n",
        (float)recvData.pv.t[0]/10,(float)recvData.pv.t[1]/10,(float)recvData.pv.t[2]/10,(float)recvData.pv.t[3]/10);
    DEBUG_PRINTF("S0:%u; S1:%u; S2:%u; S3:%u; ",recvData.pv.set[0],recvData.pv.set[1],recvData.pv.set[2],recvData.pv.set[3]);
    DEBUG_PRINTF("S4:%u; S5:%u; S6:%u; S7:%u; ",recvData.pv.set[4],recvData.pv.set[5],recvData.pv.set[6],recvData.pv.set[7]);
    DEBUG_PRINTF("S8:%u; S9:%u; S10:%u; S11:%u;\n",recvData.pv.set[8],recvData.pv.set[9],recvData.pv.set[10],recvData.pv.set[11]);
    
}

// Функция блокирует выполнение, пока не получит корректный пакет
void waitForCorrectData() {
    MYDEBUG_PRINTLN("--- Ожидание корректного пакета данных в setup()...");
    
    // Переменные для отслеживания состояния приема
    uint8_t receiveIndex = 0;
    bool receivingData = false;
    uint8_t receivedChecksum = 0;
    Upv tempReceivedUnion; // Временное хранилище для приема

    // Блокирующий цикл
    while (true) {
        // --- ВИЗУАЛЬНАЯ ИНДИКАЦИЯ: Две короткие вспышки ---
        for (int i = 0; i < 2; i++) {
            digitalWrite(LED_PIN, LOW); // LED ON
            delay(FLASH_DELAY);
            digitalWrite(LED_PIN, HIGH); // LED OFF
            delay(FLASH_DELAY*5);
        }
        
        // ----------------------------------------------------
        if (mySerial.available()) {
            uint8_t incomingByte = mySerial.read();

            if (!receivingData) {
                // СОСТОЯНИЕ 1: Ищем Стартовый Маркер
                if (incomingByte == RAMPV_START_MARKER) {
                    receivingData = true;
                    receiveIndex = 0;
                    MYDEBUG_PRINTLN("Стартовый маркер (0xAA) получен. Начинаем прием...");
                }
            } else {
                // СОСТОЯНИЕ 2: Принимаем Данные и Протокол
                if (receiveIndex < RAMPV_SIZE) {
                    // Прием байтов структуры
                    tempReceivedUnion.dataUnion[receiveIndex] = incomingByte;
                    receiveIndex++;
                } else if (receiveIndex == RAMPV_SIZE) {
                    // Прием Контрольной Суммы
                    receivedChecksum = incomingByte;
                    receiveIndex++;
                } else if (receiveIndex == RAMPV_SIZE + 1) {
                    // Прием Конечного Маркера
                    if (incomingByte == RAMPV_END_MARKER) {
                        // Пакет получен! Проверка КС
                        uint8_t expectedChecksum = calculateChecksum(tempReceivedUnion.dataUnion, RAMPV_SIZE);

                        if (receivedChecksum == expectedChecksum) {
                            // **УСПЕХ** - Копируем данные в основную структуру и ВЫХОДИМ.
                            recvData.pv = tempReceivedUnion.pv; 
                            MYDEBUG_PRINTLN("Пакет принят и ЧЕКСУММ СОВПАЛ. Выход из setup().");
                            return; 
                        } else {
                            // Ошибка контрольной суммы
                            MYDEBUG_PRINT("Ошибка КС: Принято=");
                            MYDEBUG_PRINT(receivedChecksum, HEX);
                            MYDEBUG_PRINT(", Ожидалось=");
                            MYDEBUG_PRINTLN(expectedChecksum, HEX);
                        }
                    } else {
                        MYDEBUG_PRINTLN("Ошибка: Неверный конечный маркер (0x55). Сброс приема.");
                    }
                    
                    // Сброс состояния приема для поиска нового пакета
                    receivingData = false;
                    receiveIndex = 0;
                }
            }
        }
        // Задержка необходима для корректной работы ESP8266 (Wi-Fi и другие задачи)
        delay(500); 
    }
}