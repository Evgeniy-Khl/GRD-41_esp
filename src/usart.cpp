#include <MyTelegramBot.h>
#include "main.h"
#include "usart.h"
#include "server.h"
#include "telegram.h"

extern EspSoftwareSerial::UART mySerial;
extern MyTelegramBot bot;
extern long lastSendTime, allTime;
extern uint8_t mode, errors, seconds, quarter, dataLed[], receiveBuff[], transmitBuff[];
Setting usp;
extern int tableData[32][4], tmrTelegramOff;
extern bool enabledListen;
extern char chatID [];

void OutStatus(){
    for(uint8_t i = 0; i < 5; i++){
      uint8_t numBit = 1 << i;
      dataLed[i] = upv.pv.pvOut & numBit;
    }
}

byte calculateChecksum(byte* data, int length) {
    byte checksum = 0;
    for (int i = 1; i < length; i++) {
        checksum ^= data[i]; // Используем XOR для вычисления контрольной суммы
    }
    return checksum;
}

void getData(uint8_t command){
    uint8_t dataToSend[2];
    dataToSend[0] = START_MARKER;
    dataToSend[1] = command;
    mySerial.write(dataToSend,2);
    /*if(command != GET_VALUES) */Serial.printf("---getData()->MODE=%d; COMMAND=%d; secons:%d;  All:%ld sec.\n",mode,command,seconds,allTime);
}

void saveEeprom(){
    uint8_t dataToSend[2], crc = 0;
    dataToSend[0] = START_MARKER;
    dataToSend[1] = SET_EEPROM;
    Serial.printf("-----------saveEeprom()->%d; %d,%ld sec.\n",SET_EEPROM,seconds,millis()-lastSendTime);
    for (uint8_t i = 0; i < 2; i++) {
        Serial.print(dataToSend[i]);
        Serial.print("; ");
    }
    for (uint8_t i = 0; i < EEPROM_SIZE; i++) {
        crc ^= usp.receivedData[i];
        Serial.print(usp.receivedData[i]);
        Serial.print("; ");
    }
    Serial.print("|| ");
    Serial.print(crc);
    Serial.println();
    
    mySerial.write(dataToSend,2);
    mySerial.write(usp.receivedData,EEPROM_SIZE);
    mySerial.write(&crc,1);
    crc = SET_EEPROM;   // added for parity
    mySerial.write(&crc,1);
}

void saveProgram(uint8_t quater){
    uint8_t dataToSend[2], crc = 0;
    dataToSend[0] = START_MARKER;
    dataToSend[1] = quater;
    Serial.printf("-----------saveProgram()->%d; %d,%ld sec.\n",quater,seconds,millis()-lastSendTime);
    for (uint8_t i = 0; i < 2; i++) {
        Serial.print(dataToSend[i]);
        Serial.print("; ");
    }

    for (uint8_t i = 0; i < PROG_SIZE; i++) {
        crc ^= transmitBuff[i];
        Serial.print(transmitBuff[i]);
        Serial.print("; ");
    }
    Serial.print("|| ");
    Serial.print(crc);
    Serial.println();

    mySerial.write(dataToSend,2);
    mySerial.write(transmitBuff,PROG_SIZE);
    mySerial.write(&crc,1);
    crc = quater;   // added for parity
    mySerial.write(&crc,1);
}

// Function for receiving data
void readData(){
    uint8_t availableBytes = mySerial.available();
    switch (mode){
        case READDEFAULT:
            if (availableBytes >= RAMPV_SIZE+2) {
                mySerial.readBytes(receiveBuff, RAMPV_SIZE+2);
                if (receiveBuff[0] == START_MARKER) {
                    byte receivedChecksum = receiveBuff[RAMPV_SIZE+1]; // Последний байт - контрольная сумма
                    byte calculatedChecksum = calculateChecksum(receiveBuff, RAMPV_SIZE+1);
                    long readTime = millis();
                    Serial.printf("Read VALUES: %dsec. ",seconds);
                    if (receivedChecksum == calculatedChecksum) {
                        memcpy(upv.receivedData, &receiveBuff[1], RAMPV_SIZE);
                        OutStatus();
                        Serial.println("Valid VALUES.------------------------");
                        if(tmrTelegramOff <= 0){
                            if(upv.pv.errors){ //
                                sendErrMessages(upv.pv.errors);
                            } else {
                                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                while(numNewMessages) {
                                    Serial.println();
                                    Serial.print("got response:");
                                    handleNewMessages(numNewMessages);
                                    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                }
                                Serial.printf("         Messages =%d;  %ld msek.------------------\n",numNewMessages,millis()-readTime);
                            }
                            //---------------
                        } else {
                            Serial.println("  Tmr OFF: "+ String(tmrTelegramOff));
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
        case READEEPROM:
            if (availableBytes >= EEPROM_SIZE+2) {
                mySerial.readBytes(receiveBuff, EEPROM_SIZE+2);
                if (receiveBuff[0] == START_MARKER) {
                    byte receivedChecksum = receiveBuff[EEPROM_SIZE+1]; // Последний байт - контрольная сумма
                    byte calculatedChecksum = calculateChecksum(receiveBuff, EEPROM_SIZE+1);
                    Serial.printf("Read EEPROM from Climat-5: %d,%ld     ",seconds,millis()-lastSendTime);
                    if (receivedChecksum == calculatedChecksum) {
                        memcpy(usp.receivedData, &receiveBuff[1], EEPROM_SIZE);
                        mode = READDEFAULT; interval = INTERVAL_4000;
                        // Serial.println("Valid EEPROM.");
                        // printData("Valid EEPROM.",availableBytes);
                    } else {
                        printData("EEPROM is corrupted!",availableBytes);
                        mySerial.readBytes(receiveBuff, availableBytes);
                        errors++;
                        mySerial.flush(); // Сбрасываем буфер
                    }
                }else {
                    printData("EEPROM No START_MARKER!",availableBytes);
                    mySerial.readBytes(receiveBuff, availableBytes);
                    errors++;
                    mySerial.flush(); // Сбрасываем буфер
                }
            }
            break;
        case READPROG:
            if (availableBytes >= PROG_SIZE+2){
                mySerial.readBytes(receiveBuff, PROG_SIZE+2);
                if (receiveBuff[0] == START_MARKER) {
                    byte receivedChecksum = receiveBuff[PROG_SIZE+1]; // Последний байт - контрольная сумма
                    byte calculatedChecksum = calculateChecksum(receiveBuff, PROG_SIZE+1);
                    Serial.printf("Read PROGRAM from Climat-5 Quarter=%d   %d,%ld     ",quarter,seconds,millis()-lastSendTime);
                    if (receivedChecksum == calculatedChecksum) {
                        // printData("Valid READPROG.",availableBytes);
                        fillTable(quarter);
                        if(++quarter>GET_PROG4) {mode = READDEFAULT; interval = INTERVAL_4000; quarter = GET_PROG1;}
                    } else {
                        printData("PROGRAM is corrupted!",availableBytes);
                        mySerial.readBytes(receiveBuff, availableBytes);
                        errors++;
                        mySerial.flush(); // Сбрасываем буфер
                    }
                }
            }    
            break;
        case SAVEEEPROM:  break;
        case SAVEPROG:
            if (availableBytes >= 2){
                mySerial.readBytes(receiveBuff, PROG_SIZE+2);
                if (receiveBuff[0] == START_MARKER) {
                    printData("====================Valid SAVEPROG.",availableBytes);
                    quarter = receiveBuff[1];
                    if(quarter > SET_PROG4) {mode = READDEFAULT; interval = INTERVAL_4000; quarter = GET_PROG1;}
                    else fillReceiveBuff(quarter);
                } else {
                    printData("SAVEPROG No START_MARKER!",availableBytes);
                    fillReceiveBuff(quarter);
                }
            } else {
                Serial.printf("...mode:%d; quarter:%d; availableBytes:%d     %d,%ld\n", mode, quarter, mySerial.available(), seconds, millis()-lastSendTime);
                if(quarter == SET_PROG1) fillReceiveBuff(quarter);
            }
            if(mode == SAVEPROG) delay(500);//?????????????????????????????????????????????????????????????????????????
            break;
        default: 
            Serial.printf("DEFAULT !!!: %d,%ld\n",seconds,millis()-lastSendTime);
            break;
    }
}

void printData(const char* mess, uint8_t size){
    Serial.println(mess);
    for (uint8_t i = 0; i < size; i++) {
        Serial.print(receiveBuff[i]);
        Serial.print("; ");
        if(i==size-1) Serial.print("|| ");
    }              
    Serial.println("  size:"+String(size));

}

void fillTable(uint8_t quarter){
    uint8_t row=0, item=1 ; // receiveBuff[0]==221
    switch (quarter) {
        case GET_PROG2: row = 8;  break;
        case GET_PROG3: row = 16;  break;
        case GET_PROG4: row = 24;  break;
    }
    // преобразовать 64 байт в 8 строк по 4 int
    for(uint8_t i = row; i < row+8; i++){
        for (uint8_t j = 0; j < 4; j++){
            if(j==0){
                // Преобразуйте 2 последовательных элемента uint8_t в одно значение int16_t
                tableData[i][j] = (int16_t)((receiveBuff[item + 1] << 8) | receiveBuff[item]);
                item += 2;
            } else if(j==1) {
                tableData[i][j] = receiveBuff[item++] * 2;
            } else {
                tableData[i][j] = receiveBuff[item++];
            }

            Serial.print(tableData[i][j]); Serial.print("; ");
        }
        Serial.println();
        item += 3;
    }
    Serial.println();
}

void fillReceiveBuff(uint8_t quarter) {
    uint8_t row=0, item = 0;
    switch (quarter) {
        case SET_PROG2: row = 8;  break;
        case SET_PROG3: row = 16;  break;
        case SET_PROG4: row = 24;  break;
    }
    // Заполнение transmitData из tableData
    for (uint8_t i = row; i < row+8; i++) {
        for (uint8_t j = 0; j < 7; j++) {
            if (j == 0) {
                // Преобразуем int16_t обратно в 2 последовательных uint8_t J==0; item==0,1
                transmitBuff[item] = (uint8_t)(tableData[i][j] & 0xFF); // младший байт
                transmitBuff[item + 1] = (uint8_t)((tableData[i][j] >> 8) & 0xFF); // старший байт
                item += 2;
            } else if (j == 1) {
                // Для второго элемента, делаем деление на 2 J==1; item==2
                transmitBuff[item] = tableData[i][j] / 2;
                item++;
            } else if (j > 3) {
                // Элементы j==4,5,6 в ableData отсутствуют item==5,6,7
                transmitBuff[item] = 255;
                item++;
            } else {
                transmitBuff[item] = tableData[i][j];  //J==2,3; item==3,4 копируем на прямую
                item++;
            }
        }
    }
    Serial.println("-----------------------------------------------------------------------------------------");
    Serial.printf("fillReceiveBuff() Quarter=%d; item=%d;   %d,%ld\n",quarter,item,seconds,millis()-lastSendTime);
    saveProgram(quarter);
}