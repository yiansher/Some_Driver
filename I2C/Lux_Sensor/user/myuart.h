#ifndef _MYUART_H
#define _MYUART_H

#include "stm32g0xx_hal.h"

#define TATLE              0x01
#define ADDR               0x02
#define LENTH              0x03
#define CHECK              0x04

#define DATA_LENTH_MAX          0X0F
#define DATA_TATLE              0xFA

void Recvbuf(uint8_t *readcnt, uint8_t *ringbuf);
void Cmd485_deal(void);

void Sendbuf(uint8_t *data, uint8_t len);
int8_t Serialdata_inverttranslate(uint8_t *data, uint8_t *len);
void  My_Uart_Send(uint8_t count, ...);

uint8_t OR_sum(uint8_t *data, uint8_t lenth);

#endif
