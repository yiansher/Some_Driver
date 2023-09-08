#ifndef _MY_I2C_H
#define _MY_I2C_H

#include "stm32g0xx_hal.h"

#define E2addr  0xA0
#define E2size  256

void EE_IIC_Delay(uint8_t us);
void EE_SDA_IN(void);
void EE_SDA_OUT(void);
void EE_SCL_OUT(void);
uint8_t EE_READ_SDA(void);
void EE_IIC_Init(void);
void EE_IIC_Start(void);
void EE_IIC_Stop(void);
uint8_t EE_IIC_WaitAck(void);
void EE_IIC_Ack(void);
void EE_IIC_NAck(void);
void EE_IIC_SendByte(uint8_t data);
uint8_t EE_IIC_ReadByte(uint8_t ack);
uint8_t EE_IIC_ReadByteFromSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t *buf);
uint8_t EE_EE_IIC_SendByteToSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t data);

void E2_Write_Sence(uint8_t add, uint8_t SenceId);
void E2_Read_Sence(uint8_t *readbuf, uint8_t add);
void E2_Erase_Scene(uint8_t add);

#endif
