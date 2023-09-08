#ifndef __BH1730_H
#define __BH1730_H

#include "i2c.h"
#include "main.h"

//#define BH1730 1
#define LTR303 1

#define LTR303_ADDR_S        (uint8_t)0x29
#define LTR303_ADDR          (LTR303_ADDR_S << 1)

#define BH1730_DEBUG 0

#define BH1730_ADDR_S                 (uint8_t)0x29
#define BH1730_ADDR                   (BH1730_ADDR_S << 1)
// #define BH1730_ADDR                   BH1730_ADDR_S

#define BH1730_PART_NUMBER 0x7

#define BH1730_CMD 0x80
#define BH1730_CMD_SPECIAL 0x60
#define BH1730_CMD_SPECIAL_SOFT_RESET 0x4

#define BH1730_REG_CONTROL 0x00
#define BH1730_REG_GAIN 0x7
#define BH1730_REG_TIMING 0x01
#define BH1730_REG_PART_ID 0x12
#define BH1730_REG_DATA0_LOW  0x14
#define BH1730_REG_DATA0_HIGH 0x15
#define BH1730_REG_DATA1_LOW 0x16
#define BH1730_REG_DATA1_HIGH 0x17

#define BH1730_REG_CONTROL_POWER 0x1
#define BH1730_REG_CONTROL_ADC_EN 0x2
#define BH1730_REG_CONTROL_ONE_TIME 0x8
#define BH1730_REG_CONTROL_ADC_VALID 0x10

#define BH1730_GAIN_X1_MODE 0x00
#define BH1730_GAIN_X2_MODE 0x01
#define BH1730_GAIN_X64_MODE 0x02
#define BH1730_GAIN_X128_MODE 0x03

#define BH1730_RET_TIMEOUT 50

#define BH1730_ITIME 218
#define BH1730_T_INT 2.8
#define BH1730_ITIME_MS ((BH1730_T_INT/1000.0) * 964.0 * (256.0 - BH1730_ITIME))

#define BH1730_GAIN_SET 0x00

#define GAIN_X1         1
#define GAIN_X2         2
#define GAIN_X64        64
#define GAIN_X128       128

void BH1730_init(void);
void setGain(uint8_t gain);
uint32_t BH1730_GetLux(void);

void LTR303_Write(uint8_t RegAddress, uint8_t Cmd);
uint8_t LTR303_Read(uint8_t RegAddress);
void LTR303_Init(void);
uint16_t LTR303_ReadADC_CH1(void);
uint16_t LTR303_ReadADC_CH0(void);

#endif

