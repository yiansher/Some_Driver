#include "my_i2c.h"
#include "mytimer.h"
#include "main.h"
#include "extern.h"

void EE_IIC_Delay(uint8_t us)
{
    for (uint8_t i = 0; i < 20; i++)
    {
        __asm("NOP");
    }

}

void EE_SDA_IN(void)     //SDA配置成输入
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = i2c1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2c1_SDA_GPIO_Port, &GPIO_InitStruct);
}

void EE_SDA_OUT(void)   //SDA配置成输出
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = i2c1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2c1_SDA_GPIO_Port, &GPIO_InitStruct);

}

void EE_SCL_OUT(void)   //SCL配置成输出
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = i2c1_SCL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(i2c1_SCL_GPIO_Port, &GPIO_InitStruct);
}

#define EE_IIC_SCL_0   HAL_GPIO_WritePin(i2c1_SCL_GPIO_Port, i2c1_SCL_Pin, GPIO_PIN_RESET)    //SCL 
#define EE_IIC_SCL_1   HAL_GPIO_WritePin(i2c1_SCL_GPIO_Port, i2c1_SCL_Pin, GPIO_PIN_SET)

#define EE_IIC_SDA_0   HAL_GPIO_WritePin(i2c1_SDA_GPIO_Port, i2c1_SDA_Pin, GPIO_PIN_RESET)    //SDA 
#define EE_IIC_SDA_1   HAL_GPIO_WritePin(i2c1_SDA_GPIO_Port, i2c1_SDA_Pin, GPIO_PIN_SET)

uint8_t EE_READ_SDA(void)
{
    return HAL_GPIO_ReadPin(i2c1_SDA_GPIO_Port, i2c1_SDA_Pin);
}

/******************************************************************************
*函  数：void IIC_Init(void)
*功　能：IIC初始化
*参  数：无
*返回值：无
*备  注：无
*******************************************************************************/

void EE_IIC_Init(void)
{


//    EE_SCL_OUT();
//    EE_SDA_OUT();
    EE_IIC_SCL_1;
    EE_IIC_SDA_1;



}

void EE_IIC_Start(void)
{
//    EE_SDA_OUT(); //sda线输出
    EE_IIC_SDA_1;
    EE_IIC_SCL_1;
    EE_IIC_Delay(4);
    EE_IIC_SDA_0; //START:when CLK is high,DATA change form high to low
    EE_IIC_Delay(4);
    EE_IIC_SCL_0; //钳住I2C总线，准备发送或接收数据
}


void EE_IIC_Stop(void)
{
//    EE_SDA_OUT(); //sda线输出
    EE_IIC_SCL_0;
    EE_IIC_SDA_0; //STOP:when CLK is high DATA change form low to high
    EE_IIC_Delay(4);
    EE_IIC_SCL_1;
    EE_IIC_SDA_1; //发送I2C总线结束信号
    EE_IIC_Delay(4);
}


uint8_t EE_IIC_WaitAck(void)
{
    uint8_t ucErrTime = 0;
//    EE_SDA_IN(); //SDA设置为输入  （从机给一个低电平做为应答）
    EE_IIC_SDA_1;
    EE_IIC_Delay(1);
    EE_IIC_SCL_1;
    EE_IIC_Delay(1);;

    while (EE_READ_SDA())
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            EE_IIC_Stop();
            return 1;
        }
    }

    EE_IIC_SCL_0; //时钟输出0
    return 0;
}



void EE_IIC_Ack(void)
{
    EE_IIC_SCL_0;
//    EE_SDA_OUT();
    EE_IIC_SDA_0;
    EE_IIC_Delay(1);
    EE_IIC_SCL_1;
    EE_IIC_Delay(2);
    EE_IIC_SCL_0;
}


void EE_IIC_NAck(void)
{
    EE_IIC_SCL_0;
//    EE_SDA_OUT();
    EE_IIC_SDA_1;
    EE_IIC_Delay(1);
    EE_IIC_SCL_1;
    EE_IIC_Delay(1);
    EE_IIC_SCL_0;
}


void EE_IIC_SendByte(uint8_t data)
{
    uint8_t t;
    uint8_t d;
//    EE_SDA_OUT();
    EE_IIC_SCL_0; //拉低时钟开始数据传输

    for (t = 0; t < 8; t++)
    {
        d = (data & 0x80) >> 7;

        if (d == 1)  EE_IIC_SDA_1;
        else  EE_IIC_SDA_0;

//        EE_IIC_SDA((data&0x80)>>7);
        EE_IIC_Delay(1);
        EE_IIC_SCL_1;
        data <<= 1;
        EE_IIC_Delay(1);
        EE_IIC_SCL_0;
    }

    EE_IIC_Delay(1);
}


uint8_t EE_IIC_ReadByte(uint8_t ack)
{
    uint8_t i = 0; 
    uint8_t receive = 0;

//    EE_SDA_IN(); //SDA设置为输入模式 等待接收从机返回数据
    for (i = 0; i < 8; i++)
    {
        EE_IIC_SCL_0;
        EE_IIC_Delay(1);
        EE_IIC_SCL_1;
        receive <<= 1;

        if (EE_READ_SDA())receive++;  //从机发送的电平

        EE_IIC_Delay(1);
    }

    if (ack)
        EE_IIC_Ack(); //发送ACK
    else
        EE_IIC_NAck(); //发送nACK

    return receive;
}


uint8_t EE_IIC_ReadByteFromSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t *buf)
{
    EE_IIC_Start();
    EE_IIC_SendByte(I2C_Addr);    //发送从机地址

    if (EE_IIC_WaitAck())  //如果从机未应答则数据发送失败
    {
        EE_IIC_Stop();
        return 1;
    }

    EE_IIC_SendByte(reg);   //发送寄存器地址
    EE_IIC_WaitAck();

    EE_IIC_Start();
    EE_IIC_SendByte(I2C_Addr + 1);   //进入接收模式
    EE_IIC_WaitAck();
    *buf = EE_IIC_ReadByte(0);
    EE_IIC_Stop(); //产生一个停止条件
    return 0;
}


uint8_t EE_EE_IIC_SendByteToSlave(uint8_t I2C_Addr, uint8_t reg, uint8_t data)
{
    EE_IIC_Start();
    EE_IIC_SendByte(I2C_Addr);   //发送从机地址

    if (EE_IIC_WaitAck())
    {
        EE_IIC_Stop();
        return 1; //从机地址写入失败
    }

    EE_IIC_SendByte(reg);   //发送寄存器地址
    EE_IIC_WaitAck();
    EE_IIC_SendByte(data);

    if (EE_IIC_WaitAck())
    {
        EE_IIC_Stop();
        return 1; //数据写入失败
    }

    EE_IIC_Stop(); //产生一个停止条件

    return 0;
}


/*******************************************************
*写情景
*******************************************************/
void E2_Write_Sence(uint8_t add, uint8_t SenceId)
{
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 0, SenceId);
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 1, SenceId);
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 2, SenceId);
    Delay_1us(25);
}

/*******************************************************
*读情景
*******************************************************/
void E2_Read_Sence(uint8_t *readbuf, uint8_t add)
{
    EE_IIC_ReadByteFromSlave(0xA0, (add) * 2 + 0, readbuf);
}

/*******************************************************
*擦情景
*******************************************************/
void E2_Erase_Scene(uint8_t add)
{
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 0, 0xff);
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 1, 0xff);
    Delay_1us(25);
    EE_EE_IIC_SendByteToSlave(0xA0, (add) * 2 + 2, 0xff);
    Delay_1us(25);
}




