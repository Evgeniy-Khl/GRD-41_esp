#include <MyTelegramBot.h>
#include "main.h"
#include "usart.h"
#include "server.h"
#include "telegram.h"

extern EspSoftwareSerial::UART mySerial;
extern MyTelegramBot bot;
extern long lastSendTime, allTime;
extern uint8_t mode, errors, lastError, seconds, quarter, dataLed[], receiveBuff[], transmitBuff[];
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
        checksum ^= data[i]; // Using XOR to calculate the checksum
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
    for (uint8_t i = 12; i < 12+INDEX*2; i++) {
        crc ^= upv.receivedData[i];
        Serial.print(upv.receivedData[i]);
        Serial.print("; ");
    }
    Serial.print("|| ");
    Serial.print(crc);
    Serial.println();
    
    mySerial.write(dataToSend,2);
    mySerial.write(&upv.receivedData[12],INDEX*2);
    mySerial.write(&crc,1);
    crc = SET_EEPROM;   // added for parity
    mySerial.write(&crc,1); // TOTAL 27 byte
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
                    Serial.printf("Read VALUES: %dsec. ",seconds);
                    if (receivedChecksum == calculatedChecksum) {
                        memcpy(upv.receivedData, &receiveBuff[1], RAMPV_SIZE);
                        // OutStatus();
                        Serial.println("Valid VALUES.------------------------");
                        if(tmrTelegramOff <= 0){
                            if(upv.pv.errors && lastError != upv.pv.errors){
                                sendErrMessages(upv.pv.errors);
                                lastError = upv.pv.errors;      // exclude duplicate message
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
    Serial.printf("model:%u; node:%u; mode:%u; port:0x%02x;",upv.pv.model,upv.pv.node,upv.pv.modeCell,upv.pv.portFlag);
    Serial.printf("t0:%3.1f; t1:%3.1f; t2:%3.1f; t3:%3.1f;\n",
        (float)upv.pv.t[0]/10,(float)upv.pv.t[1]/10,(float)upv.pv.t[2]/10,(float)upv.pv.t[3]/10);
    Serial.printf("S0:%u; S1:%u; S2:%u; S3:%u;",upv.pv.set[0],upv.pv.set[1],upv.pv.set[2],upv.pv.set[3]);
    Serial.printf("S4:%u; S5:%u; S6:%u; S7:%u;",upv.pv.set[4],upv.pv.set[5],upv.pv.set[6],upv.pv.set[7]);
    Serial.printf("S8:%u; S9:%u; S10:%u; S11:%u;\n",upv.pv.set[8],upv.pv.set[9],upv.pv.set[10],upv.pv.set[11]);
    
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