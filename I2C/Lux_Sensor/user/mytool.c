#include "mytool.h"
#include "mytimer.h"
#include "extern.h"
#include "usart.h"

volatile struct DebugCount
{
    uint8_t count1;
    uint8_t count2;
    uint8_t count3;
    uint8_t count4;
} debugCount = {0, 0, 0, 0};

uint8_t address = 0;
uint8_t DialNum[1] = {0};

uint8_t adress_data = 0;

void USART_SendByte(UART_HandleTypeDef *huart, uint8_t temp)
{
    while (!__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE));

//    HAL_UART_Transmit( huart, &temp, 1, 0 );
    HAL_UART_Transmit_IT(huart, &temp, 1);

    while (!__HAL_UART_GET_FLAG(huart, UART_FLAG_TC));
}

#ifdef DEBUG
void Debug_SendByte(uint8_t temp)
{
    USART_SendByte(&huart2, temp);
}
#endif

void USART_SendString(UART_HandleTypeDef *huart, char *temp)
{
    char *str = temp;

    while (*str)
    {
        USART_SendByte(huart, *str);
        str++;
    }
}

void led_twinkling(void)
{
    led_off();
    Delay_1ms(100);
    led_on();
    Delay_1ms(100);
    led_off();
    Delay_1ms(100);
    led_on();
    Delay_1ms(100);
    led_off();
    Delay_1ms(100);
    led_on();
    Delay_1ms(100);
    led_off();
    Delay_1ms(100);
    led_on();
    Delay_1ms(100);
    led_off();
}

/*Debug Light*/
//void Debug_LightTwink(uint8_t ms)
//{
//    switch(ms){
//        case 1:{
//            debugCount.count1++;
//            if(debugCount.count1>=)
//        }
//    }//switch_end
//}



/******************************************************************************
*函  数：void Read_ecAddr(void)
*功　能：读拨码地址
*参  数：无
*返回值：无
*备  注：无
*******************************************************************************/
uint8_t address_judge(void)
{
    uint8_t address_bit = 0;
    address = 0x00;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);        //0
    address_bit = address_bit << 0;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);        //1
    address_bit = address_bit << 1;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);        //2
    address_bit = address_bit << 2;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);        //3
    address_bit = address_bit << 3;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);        //4
    address_bit = address_bit << 4;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);        //5
    address_bit = address_bit << 5;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);        //6
    address_bit = address_bit << 6;
    address = address | address_bit;

    address_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);        //7
    address_bit = address_bit << 7;
    address = address | address_bit;

    address = ~address;


    return address;
}

void address_deal(void)                    //地址判断消抖
{
    static uint8_t  i = 1;

    switch (i)
    {
    case 1:
    {
        address_judge();
        i++;
        break;
    }

    case 2:
    {
        address_judge();
        i++;
        break;
    }

    case 3:
    {
        address_judge();
        i++;
        break;
    }

    case 4:
    {
        address_judge();
        i++;
        break;
    }

    case 5:
    {
        address_judge();
        i++;
        break;
    }

    case 6:
    {
        address_judge();
        i++;
        break;
    }

    case 7:
    {
        address_judge();
        i++;
        break;
    }

    case 8:
    {
        address_judge();
        i++;
        break;
    }

    case 9:
    {
        address_judge();
        i++;
        break;
    }

    case 10:
    {
        if (DialNum[0] != address_judge())
        {
            DialNum[0] = address_judge();
            i = 1;
        }

        break;
    }
    }//switch end

    /************************************************************/
}



int fputc(int ch, FILE *f)
{
    uint8_t temp[1] = {ch};

    while (!__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE));

    HAL_UART_Transmit(&huart2, temp, 1, 0);

    while (!__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC));

    return ch;
}

