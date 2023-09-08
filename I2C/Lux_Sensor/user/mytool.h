#ifndef _MYTOOL_H
#define _MYTOOL_H

//#define DEBUG

#include "stm32g0xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

void USART_SendByte(UART_HandleTypeDef *huart, uint8_t temp);
void USART_SendString(UART_HandleTypeDef *huart, char *temp);
void Debug_SendByte(uint8_t temp);

void led_twinkling(void);

void address_deal(void);
uint8_t address_judge(void);

#define led_on()    HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
#define led_off()    HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);

#define APP_ERROR(fmt, ...) do {printf("[ERROR][%s,%d]: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);} while(0)
#define APP_TRACE(fmt, ...) do {printf("[TRACE][%s,%d]: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);} while(0)
#define APP_DEBUG(fmt, ...) do {printf("[DEBUG][%s,%d]: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);} while(0)

#endif
