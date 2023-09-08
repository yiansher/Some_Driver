#include "deal.h"
#include "extern.h"
#include "my_i2c.h"
#include "myuart.h"
#include "mytool.h"
#include "mytimer.h"

#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "iwdg.h"

uint8_t OCAddr[2] = {0};
uint8_t feedback[11] = {0};

uint8_t NetInFlag = 0;
uint8_t NetoutFlag = 0;

volatile uint8_t bodyflag = 0;
uint8_t sceneID[1] = {0};
uint8_t Scenebuf[1] = {0};
volatile uint8_t bodyflag2 = 1;

uint16_t LuxToUse = 0;
uint32_t LuxFromBH = 0;



//uint8_t Contact1Have = 0;
//uint8_t Contact2Have = 0;

void Net_deal(void)
{
    MyAddressH = (DialNum[0] & 0xf0) >> 4;
    MyAddressL = DialNum[0] & 0x0f;

    if (MyAddressL == 1)
    {
        NetTodoFlag = Net_In;
    }
    else if (MyAddressL == 9)
    {
        NetTodoFlag = Net_Out;
    }
    else if (MyAddressL == 5)
    {
        NetTodoFlag = ClearAll;
    }
    else
    {
        NetTodoFlag = Net_Addr;
    }

    switch (MyAddressH)
    {
    case 0:
        My_Delay_Time = 1;
        break;

    case 1:
        My_Delay_Time = 30;
        break;

    case 2:
        My_Delay_Time = 60;
        break;

    case 3:
        My_Delay_Time = 180;
        break;

    case 4:
        My_Delay_Time = 300;
        break;

    case 5:
        My_Delay_Time = 600;
        break;

    case 6:
        My_Delay_Time = 900;
        break;

    case 7:
        My_Delay_Time = 1200;
        break;

    case 8:
        My_Delay_Time = 1800;
        break;

    case 9:
        My_Delay_Time = 2400;
        break;
    }//switch end

    switch (NetTodoFlag)
    {
    case Net_In:
        NetInDeal();
        break;

    case Net_Out:
        NetOutDeal();
        break;

    case ClearAll:
        ClearAll_Deal();
        break;

    default :
//        recvbufOK = 0;
        break;
    }

}

void ClearAll_Deal(void)
{
    static uint8_t count = 0;
    if(++count<6)
    {
        return;
    }
    else
    {
    led_on();

    for (uint16_t i = 0; i < 512; i++)
    {
        EE_EE_IIC_SendByteToSlave(0xA0, i, 0xff);
        HAL_Delay(10);
        HAL_IWDG_Refresh(&hiwdg);
    }

    IfAddress = 0;
    IfScene = 0;
    NetInFlag = 0;
    
    led_twinkling();
    count = 0; 
}
}

void NetInDeal(void)
{
    if ((!NetInFlag) && (IfAddress))
    {
        Delay_1ms(rand() % 200);
        My_Uart_Send(11, 0x00, 0x00, DevAddress[0], 0x00, 0x19, 0x00, 0x00, 0X14);
    }

    return;
}

void NetOutDeal(void)
{
    if (IfAddress && NetInFlag)
    {
        My_Uart_Send(11, 0x00, 0x00, DevAddress[0], 0x00, 0x1B, 0x00, 0x00, 0x14);
    }

    return;
}

void NetIN_Feedback_Deal(void)
{
    if (RxData[9] == 0x01)
    {
        for (uint8_t i = 45; i < 50; i += 2)
        {
            EE_EE_IIC_SendByteToSlave(0xA0, i, RxData[6]);
            Delay_1ms(2);
            EE_EE_IIC_SendByteToSlave(0xA0, i + 1, RxData[6]);
            Delay_1ms(2);
        }

        led_twinkling();
        NetInFlag = 1;
    }

    recvbufOK = 0;
}
void NetOUT_Feedback_Deal(void)
{
    if (RxData[9] == 0x01)
    {

        for (uint8_t i = 41; i < 50; i += 2)
        {
            EE_EE_IIC_SendByteToSlave(0xA0, i, 0xFF);
            Delay_1ms(2);
            EE_EE_IIC_SendByteToSlave(0xA0, i + 1, 0xFF);
            Delay_1ms(2);
        }

        led_twinkling();
        NetInFlag = 0;
    }

    recvbufOK = 0;
}
/*******************************************************************************
* 函 数 名         : sence_config_deal
* 函数功能           : 写 情景配置
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void sence_config_deal(void)    //情景配置
{
    My_Uart_Send(9, 0x00, 0x00, DevAddress[0], RxData[2], 0x0A, 0x01);

    recvbufOK = 0;
}

/*******************************************************************************
* 函 数 名         : Contact_Input_Config
* 函数功能           : 写 触点配置处理
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void Contact_Input_Config(void)     //触点配置
{
    if ((RxData[2] < 0X61) || (RxData[2] > 0x68) || (RxData[7] != 0x01) || (RxData[8] != 0x00))
        return;

    switch (RxData[2])
    {    
    case SubAddress1:
    {
        E2_Write_Sence(SubAddress1, RxData[9]);
        Delay_1ms(rand()%100);
        E2_Read_Sence(Scenebuf, SubAddress2);

        if (Scenebuf[0] != 0xff && Scenebuf[0] != 0)
        {
            IfScene = 1;
            bodyflag2 = 0;
//            USART_SendByte(&huart2, 0xAA);
        }
        else
        {
            IfScene = 0;
            bodyflag2 = 0;
//            USART_SendByte(&huart2, Scenebuf[0]);
        }

        My_Uart_Send(9, 0x00, 0x00, DevAddress[0], RxData[2], 0X12, 0x01);   //触点配置反馈
        break;
    }

    case SubAddress2:
    {
        E2_Write_Sence(SubAddress2, RxData[9]);
        Delay_1ms(rand()%100);
        E2_Read_Sence(Scenebuf, SubAddress1);

        if (Scenebuf[0] != 0xff && Scenebuf[0] != 0)
        {
            IfScene = 1;
//            USART_SendByte(&huart2, 0xCC);
        }
        else
        {
            IfScene = 0;
//            USART_SendByte(&huart2, Scenebuf[0]);
        }

        My_Uart_Send(9, 0x00, 0x00, DevAddress[0], RxData[2], 0X12, 0x01);   //触点配置反馈
        break;
    }
    default:break;
    }

    recvbufOK = 0;
}


/*******************************************************************************
* 函 数 名         : sence_config_delect
* 函数功能           : 写 情景删除
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void sence_config_delect(void)    //情景删除
{
    uint8_t Delete_Scene1[1] = {0};
    uint8_t Delete_Scene2[1] = {0};

    EE_IIC_ReadByteFromSlave(0xA0, SubAddress1 * 2, Delete_Scene1);
    Delay_1ms(50);
    EE_IIC_ReadByteFromSlave(0xA0, SubAddress2 * 2, Delete_Scene2);
    Delay_1ms(50);

    if (RxData[7] == Delete_Scene1[0])
    {
        E2_Erase_Scene(SubAddress1);
        Delay_1ms(rand() % 50);
        My_Uart_Send(10, 0x00, 0x00, DevAddress[0], RxData[2], 0x0c, RxData[7], 0x01);
        IfScene = 0;
    }
    else if (RxData[7] == Delete_Scene2[0])
    {
        E2_Erase_Scene(SubAddress2);
        Delay_1ms(rand() % 50);
        My_Uart_Send(10, 0x00, 0x00, DevAddress[0], RxData[2], 0x0c, RxData[7], 0x01);
        IfScene = 0;
    }
    else
    {
        My_Uart_Send(10, 0x00, 0x00, DevAddress[0], RxData[2], 0x0c, RxData[7], 0x02);
    }

    recvbufOK = 0;
}

void BodySence_deal(void)
{
    if (IfScene)
    {
        E2_Read_Sence(sceneID, SubAddress1);
        bodyflag = 0;

        for (uint8_t i = 0; i < 20; i++)  //判断红外
        {
            Delay_1ms(5);

            if (HAL_GPIO_ReadPin(body_GPIO_Port, body_Pin))
            {
                bodyflag = 1;
            }
            else
            {
                bodyflag = 0;
            }
        }

        if (bodyflag && (!bodyflag2))
        {
            Delay_1ms(rand() % 100);
            My_Uart_Send(9, 0xff, 0xff, DevAddress[0], SubAddress1, 0x0d, sceneID[0]);
            bodyflag2 = 1;
            tim16_delay_flag = 0;
        }
    }

//    recvbufOK = 0;
}


void Write_Address_deal(void)
{
    uint8_t ver[10] = {0};

    for (uint8_t i = 0; i < 10; i++)
    {
        EE_EE_IIC_SendByteToSlave(0xA0, i, RxData[7]);
        HAL_Delay(10);
    }

    EE_IIC_ReadByteFromSlave(0xA0, 1, &ver[1]);

    for (uint8_t i = 2; i < 10; i++)
    {
        EE_IIC_ReadByteFromSlave(0xA0, i, &ver[i]);

        if (ver[i] != ver[i - 1])
        {
            My_Uart_Send(9, 0x00, 0x00, 0x00, 0x00, 0x87, 0x02);
            recvbufOK = 0;
            return;
        }
    }

    DevAddress[0] = ver[1];
    My_Uart_Send(9, DevAddress[0], 0x00, 0x00, 0x00, 0x87, 0x01);
    IfAddress = 1;
    led_twinkling();
    recvbufOK = 0;

}

void Write_Lux_deal(void)
{
    uint8_t ver1[40] = {0};
//  uint8_t WhyNotOutput[2] = {0xBB,0xCC};

    LuxToUse = (uint16_t)RxData[7] << 8 | RxData[8];

    for (uint8_t i = 20; i < 40; i += 2)
    {
        EE_EE_IIC_SendByteToSlave(0xA0, i, RxData[7]);
        Delay_1ms(2);
        EE_EE_IIC_SendByteToSlave(0xA0, i + 1, RxData[8]);
        Delay_1ms(2);
    }

    EE_IIC_ReadByteFromSlave(0xA0, 20, &ver1[20]);
    Delay_1ms(2);
    EE_IIC_ReadByteFromSlave(0xA0, 21, &ver1[21]);
    Delay_1ms(2);

    for (uint8_t i = 22; i < 40; i += 2)
    {
        EE_IIC_ReadByteFromSlave(0xA0, i, &ver1[i]);
        Delay_1ms(2);
        EE_IIC_ReadByteFromSlave(0xA0, i + 1, &ver1[i + 1]);

        if (ver1[i] != ver1[i - 2] || ver1[i + 1] != ver1[i - 1])
        {
            My_Uart_Send(9, DevAddress[0], 0x00, 0x00, 0x00, 0x89, 0x02);
            recvbufOK = 0;
            return;
        }
    }

    My_Uart_Send(11, DevAddress[0], 0x00, 0x00, 0x00, 0x89, ver1[20], ver1[21], 0x01);
    led_twinkling();
    Delay_1ms(2);

    recvbufOK = 0;
}





