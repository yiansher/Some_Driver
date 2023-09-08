#include "mytimer.h"
#include "mytool.h"

#include "main.h"
#include "tim.h"
#include "extern.h"

//volatile uint8_t tim16_Delay_flag = 0;
//uint16_t tim16_Delay_count = 0;
volatile uint8_t tim16_1s_flag = 0;
uint16_t tim16_1s_count = 0;
volatile uint8_t tim16_100ms_flag = 0;
uint16_t tim16_100ms_count = 0;
volatile uint8_t tim16_10ms_flag = 0;
uint16_t tim16_10ms_count = 0;
volatile uint8_t tim16_200ms_flag = 0;
uint16_t tim16_200ms_count = 0;
volatile uint8_t tim16_20s_flag = 0;
uint16_t tim16_20s_count = 0;
volatile uint8_t tim16_delay_flag = 1;
volatile uint8_t tim16_delay_flag1 = 0;
uint32_t tim16_delay_count = 0;

uint16_t tim14cnt = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim-> Instance == htim3.Instance)
    {

        if (++receive_data_time >= 6 || readcnt > 40)
        {
            for (uint8_t i = 0; i < 60; i++)                   //还原串口接收数据默认值
                ringbuf[i] = 0;

            readcnt = 0;
            translate_flage = 0;
            sumtranslate_flage = 0;
            state = 0x01;
            receive_data_time = 0;
            HAL_TIM_Base_Stop_IT(&htim3);
        }

    }

    if (htim-> Instance == htim16.Instance)
    {
        if (!tim16_20s_flag)
        {
            if (++tim16_20s_count == 20000)
            {
//                tim16_20s_flag = 1;
                led_off();
                tim16_20s_count = 0;
            }
        }

        if (!tim16_1s_flag)
        {
            if (++tim16_1s_count == 1000)
            {
                tim16_1s_flag = 1;
                tim16_1s_count = 0;
            }
        }

        if (!tim16_100ms_flag)
        {
            if (++tim16_100ms_count == 100)
            {
                tim16_100ms_flag = 1;
                tim16_100ms_count = 0;
            }
        }

        if (!tim16_10ms_flag)
        {
            if (++tim16_10ms_count == 10)
            {
                tim16_10ms_flag = 1;
                tim16_10ms_count = 0;
            }
        }

        if (!tim16_200ms_flag)
        {
            if (++tim16_200ms_count == 200)
            {
                tim16_200ms_flag = 1;
                tim16_200ms_count = 0;
            }
        }

        if (!tim16_delay_flag)
        {
            if (++tim16_delay_count == (My_Delay_Time * 1000))
            {
                tim16_delay_flag = 1;
                tim16_delay_flag1 = 1;

            }
        }

    }
}

void Delay_1us(uint16_t xus)
{
    uint16_t differ = 0xfffff - xus - 5;
    __HAL_TIM_SetCounter(&htim14, differ);
    HAL_TIM_Base_Start(&htim14);

    while (differ < 0xffff - 5)
    {
        differ = __HAL_TIM_GetCounter(&htim14);
    }

    HAL_TIM_Base_Stop(&htim14);
}

void Delay_1ms(uint16_t xms)
{
    for (uint32_t i = 0; i < xms; i++)
    {
        Delay_1us(1000);
    }
}
