#ifndef __USART_H
#define __USART_H

#define START_MARKER 0xDD	// Начало блока = 221

#define READDEFAULT 0
#define READEEPROM  1
#define SAVEEEPROM  2
#define READPROG    3
#define SAVEPROG    4
#define GET_VALUES	0x40    // 64
#define GET_EEPROM	0x41    // 65
#define SET_EEPROM	0x42    // 66
#define SET_OFF     0x50    // 80
#define SET_ON      0x51    // 81

void getData(uint8_t byte);
void readData();
void saveSet(uint8_t status);
void printData(const char* mess, uint8_t size);
// void sendData(int command, uint8_t *data = nullptr);

#endif /* _USART_H */
