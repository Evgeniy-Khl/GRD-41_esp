#include <MyTelegramBot.h>
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
    // transmitBuff[2] - transmitBuff[5] -> IP-адрес
    mySerial.write(dataToSend,2);
    mySerial.write(myIp,6);
    mySerial.write(transmitBuff,20);// TOTAL 28 byte
    //if(command != GET_VALUES) Serial.printf("---getData()->MODE=%d; COMMAND=%d; secons:%d;  All:%ld sec.\n",mode,command,seconds,allTime);
}

void saveSet(uint8_t stat){
    uint8_t dataToSend[2], crc = 0;
    dataToSend[0] = START_MARKER;
    dataToSend[1] = SET_EEPROM;
    Serial.printf("-----------saveEeprom()->%d; seconds: %d,%ld sec.\n",SET_EEPROM,seconds,millis()-lastSendTime);
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
    Serial.println(crc);
    Serial.printf("S0:%u; S1:%u; S2:%u; S3:%u; ",upv.pv.set[0],upv.pv.set[1],upv.pv.set[2],upv.pv.set[3]);
    Serial.printf("S4:%u; S5:%u; S6:%u; S7:%u; ",upv.pv.set[4],upv.pv.set[5],upv.pv.set[6],upv.pv.set[7]);
    Serial.printf("S8:%u; S9:%u; S10:%u; S11:%u;\n",upv.pv.set[8],upv.pv.set[9],upv.pv.set[10],upv.pv.set[11]);
    Serial.println("-----------mySerial.write()");
    
    mySerial.write(dataToSend,2);
    mySerial.write(&upv.receivedData[12],INDEX*2);// 24 byte
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
                    //Serial.printf("Read VALUES: %dsec. ",seconds);
                    if (receivedChecksum == calculatedChecksum) {
                        memcpy(upv.receivedData, &receiveBuff[1], RAMPV_SIZE);
                        //Serial.println("Valid VALUES.------------------------");
                        if(tmrTelegramOff <= 0){
                            if(upv.pv.errors && lastError != upv.pv.errors){
                                sendErrMessages(upv.pv.errors);
                                lastError = upv.pv.errors;              // exclude duplicate message
                            } else if (status != (upv.pv.portFlag & 4)){    // WORK - shutdown flag
                                status = upv.pv.portFlag&4;
                                sendStatus();
                            } else {                // check incoming messages
                                int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                while(numNewMessages) {
                                    Serial.println();
                                    Serial.print("got response:");
                                    handleNewMessages(numNewMessages);
                                    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
                                }
                                Serial.printf("==============Messages =%d;  %ld msek.=============\n",numNewMessages,millis()-readTime);
                            }
                            //---------------
                        } else {
                            // Serial.println("  Tmr OFF: "+ String(tmrTelegramOff));
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
            Serial.printf("Default! mode=%d; time: %d,%ld\n", mode,seconds,millis()-lastSendTime);
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
    Serial.printf("model:%u; node:%u; mode:%u; port:0x%02x; ",upv.pv.model,upv.pv.node,upv.pv.modeCell,upv.pv.portFlag);
    Serial.printf("t0:%3.1f; t1:%3.1f; t2:%3.1f; t3:%3.1f;\n",
        (float)upv.pv.t[0]/10,(float)upv.pv.t[1]/10,(float)upv.pv.t[2]/10,(float)upv.pv.t[3]/10);
    Serial.printf("S0:%u; S1:%u; S2:%u; S3:%u; ",upv.pv.set[0],upv.pv.set[1],upv.pv.set[2],upv.pv.set[3]);
    Serial.printf("S4:%u; S5:%u; S6:%u; S7:%u; ",upv.pv.set[4],upv.pv.set[5],upv.pv.set[6],upv.pv.set[7]);
    Serial.printf("S8:%u; S9:%u; S10:%u; S11:%u;\n",upv.pv.set[8],upv.pv.set[9],upv.pv.set[10],upv.pv.set[11]);
    
}

