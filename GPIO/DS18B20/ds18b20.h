#ifndef __DS18B20_H__
#define __DS18B20_H__

#include <stdio.h>
#include "gpio.h"

/* Read 64-bit ROM code: 8-bit family code, unique 48-bit serial number, and 8-bit CRC */
#define CMD_READ_ID 0x33
/* Match slave in multi-system, use command followed by a 64-bit ROM sequence */
#define CMD_MATCH_SLAVE 0x55
/* Allow master to access the memory function without providing the 64-bit ROM code, on single slave bus */
#define CMD_READ_ROM_UNUSE_ID 0xCC
/* Allow master to use a process of elimination to identify the 64-bit ROM codes of all slave devices on the bus */
#define CMD_SEARCH_ROM 0xF0
/* as name, no use */
#define CMD_ALARM_SEARCH 0xEC
/* write register starting at TH register, followed by 3 bytes */
#define CMD_WRITE_REGISTER 0x4E
/* read from byte 0 to byte 9(CRC), stop any time by issue a reset pulse */
#define CMD_READ_REGISTER 0xBE
/* convert temprature */
#define CMD_CONVERT_TEMP 0x44

enum Resolution
{
    BIT_9 = 9,
    BIT_10,
    BIT_11,
    BIT_12
};

uint8_t DS18B20_Reset(void);

float DS18B20_ConvertTemp(int16_t temp);

uint8_t DS18B20_ReadID(uint8_t* id);
int16_t DS18B20_ReadTempReg(void);
int16_t DS18B20_ReadTempRegByID(uint8_t* id);

uint8_t DS18B20_ReadTempRegCRC(int16_t* read_temp);

void DS18B20_SetResolution(uint8_t res);

#endif