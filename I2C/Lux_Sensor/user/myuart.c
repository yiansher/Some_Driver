#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "extern.h"
#include "myuart.h"
#include "mytool.h"
#include "string.h"
#include "deal.h"
#include "stdarg.h"
#include "mytimer.h"

uint8_t translate_flage = 0;
uint8_t sumtranslate_flage = 0;
uint8_t ringbuf[60] = {0};
uint8_t readcnt = 0;
uint8_t recvbufOK = 0;
uint8_t  RxData[41] = {0};
volatile uint8_t receive_data_time = 0;

uint8_t state = TATLE;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
     HAL_UART_Receive_IT(&huart2, u8RxData, 1);
    if (huart -> Instance == huart2.Instance)
    {
        receive_data_time = 0;
        HAL_TIM_Base_Start_IT(&htim3);

        if (translate_flage == 1)
        {
            translate_flage = 0;

            if (u8RxData[0] == 0x01)
            {
                ringbuf[readcnt] = 0xFA;       //�������ݵ�����
            }
            else if (u8RxData[0] == 0x02)
            {
                ringbuf[readcnt] = 0xFE;        //�������ݵ�����
            }
            else
            {
                for (uint8_t i = 0; i < 60; i++)                   //��ԭ���ڽ�������Ĭ��ֵ
                    ringbuf[i] = 0;

                readcnt = 0;
                translate_flage = 0;
                sumtranslate_flage = 0;
                state = 0x01;
                receive_data_time = 0;
                HAL_TIM_Base_Stop_IT(&htim3);
            }
        }
        else
        {
            if (u8RxData[0] == 0xFE)
            {
                translate_flage = 1;
                return;
            }
            else
            {
                ringbuf[readcnt] = u8RxData[0];//�������ݵ�����
            }
        }

        readcnt++;
        Recvbuf(&readcnt, ringbuf);
    } 
 //   HAL_UART_Receive_IT(&huart2, u8RxData, 1);
}


void Recvbuf(uint8_t *readcnt, uint8_t *ringbuf)
{

    uint8_t ch, i;

    if (readcnt == NULL || ringbuf == NULL)
    {
        return;
    }

    switch (state)
    {
    case TATLE:
        while ((*readcnt) > 1)
        {
            if (ringbuf[0] != DATA_TATLE)
            {
                (*readcnt) --;   //�ȼ�

                for (i = 0; i < (*readcnt); ++i) ringbuf[i] = ringbuf[i + 1];

            }
            else
            {
                state = ADDR;
                return;
            }
        }

        return;

    case ADDR:
        if ((ringbuf[1] != DevAddress[0]) && (ringbuf[1] != 0XFF))
        {
            state = TATLE;
            ringbuf[0] = 0X00;//�ƻ���ͷ�����������
            (*readcnt) = 0;
        }
        else
        {
            state = LENTH;
        }

        return;

    case LENTH:
        if (((*readcnt) > 5) && ((ringbuf[5]) == (*readcnt)))
        {
            state = CHECK;
        }
        else
        {
            state = LENTH;
            return;
        }

    case CHECK:
        ch = OR_sum(ringbuf, ringbuf[5] - 1);

        if (ch == ringbuf[ringbuf[5] - 1])
        {
            //�±�־λ
            if (recvbufOK == 0)  //���ݴ�����Ϻ��ٴ�����һ��
            {
                memcpy(RxData, ringbuf, ringbuf[5]);
                recvbufOK = 1;
            }
        }

        state = TATLE;
        (*readcnt) -= ringbuf[5];
        return;

    default:
        state = TATLE;
        return;
    }
}

void Cmd485_deal(void)
{
    if (!recvbufOK)
        return;
    
    switch (RxData[6])
    {
    case SENCE_CONFIG:
        sence_config_deal();
        break;               //�龰����

    case CONTACTOR_CONFIG:
        Contact_Input_Config();
        break;               //������������

    case SENCE_CONFIG_DELETE:
        sence_config_delect();
        break;               //�龰ɾ��

//      case SENCE_CONTROL:                     sence_data_deal();                      break;               //�龰����
    case JOIN_NETWORK_FEEDBECK:
        NetIN_Feedback_Deal();
        break;               //��������

    case EXIT_NETWORK_FEEDBECK:
        NetOUT_Feedback_Deal();
        break;               //��������

//      case IR_RESET:                          ir_Reset();                             break;               //�����ʼ��
//      case CHECK_NETWORK_STATE:               network_check();                        break;               //����״̬��ѯ
    case Write_Address:
        Write_Address_deal();
        break;

    case Write_Lux:
        Write_Lux_deal();
        break;

    case HOLISTIC_LED_CONTROL:
    {
        led_twinkling();
        recvbufOK = 0;
    }
    break;

    default :
        recvbufOK = 0;
        break;
    }

}

uint8_t OR_sum(uint8_t *data, uint8_t lenth)
{
    uint8_t sum = 0;

    if (data == NULL)
    {
        return 0;
    }

    for (uint8_t i = 0; i < lenth; i++)
    {
        sum ^= *(data + i);
    }

    return (sum);
}


/*********************************************************************
*
*
*
*
*
********************************************************************/

void  My_Uart_Send(uint8_t count, ...)   //Need to specify the count of vars          ������ʱ�Ĵ��ڷ��ͺ���
{
    va_list valist;
    va_arg(valist, int);
    uint8_t i = 0;
    va_start(valist, count);

    for (i = 0; i < count - 3; i++)
        My_Send_Data[i] = va_arg(valist, int);

    va_end(valist);
//    MY_485_TX();
    Sendbuf(My_Send_Data, count - 3);
    Delay_1us(100);
//      MY_485_RX();
    HAL_UART_Receive_IT(&huart2, u8RxData, 1);
}

void Sendbuf(uint8_t *data, uint8_t len)
{
    static uint8_t uartbuf[DATA_LENTH_MAX];
    uint8_t i, lenth, addlen, addlen1;
    uartbuf[0] = 0xFA;
    uartbuf[5] = len + 3;
    lenth = len + 3;
    addlen = 0;
    addlen1 = 0;

    for (i = 0; i < 4 ; i++)
    {
        uartbuf[i + 1] = data[i];

        if (uartbuf[i + 1] == 0XFA || uartbuf[i + 1] == 0XFE)
        {
            addlen1++; //����ת���ĳ��ȴ洢λ
        }
    }

    for (i = 0; i < (len - 4) ; i++)
    {
        uartbuf[i + 6] = data[i + 4];
    }

    uartbuf[len + 2] = OR_sum(uartbuf, uartbuf[5] - 1);

    while (lenth >= 1)
    {
        if (uartbuf[lenth] == 0XFA || uartbuf[lenth] == 0XFE)
        {
            addlen++;//����ת�������ݳ���
        }

        lenth--;
    }

    lenth = len + 3;

    if (uartbuf[1] == uartbuf[3])
    {
//        if( recvbufOK == 0 )              //��ѭ��
//        {
//            memcpy( RxData, uartbuf, uartbuf[5] );
//            recvbufOK = 1;
//        }
    }
    else
    {
        if ((uartbuf[6] == 0x0D) || (uartbuf[6] == 0x03) || (uartbuf[6] == 0x04))
        {
//            memcpy( RxData, uartbuf, uartbuf[5] );
//            recvbufOK = 1;
        }

        Serialdata_inverttranslate(uartbuf, &lenth);

        for (i = 0; i < uartbuf[5 + addlen1] + addlen; i++)
        {
            USART_SendByte(&huart2, uartbuf[i]);
        }
    }
}

static int8_t Serialdata_inverttranslate(uint8_t *data, uint8_t *len)
{
    uint8_t i, j;

    if ((data == NULL) || (len == NULL))
    {
        return ERROR;//��ָ��
    }

    for (i = 1; i <= (*len) - 1; ++i)
    {
        if (data[i] == 0XFA)
        {
            (*len) += 1;
            data[i] = 0xFE;

            for (j = (*len) - 1; j > i + 1; j--)
            {
                data[j] = data[j - 1];
            }

            data[i + 1] = 0x01;
        }
        else if (data[i] == 0xFE)
        {
            data[i] = 0xFE;
            (*len) += 1;

            for (j = (*len) - 1; j > i + 1; j--)
            {
                data[j] = data[j - 1];
            }

            data[i + 1] = 0x02;
        }
        else
        {
            ;
        }
    }

    return SUCCESS;
}
