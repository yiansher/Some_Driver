/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "W25Qx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUF_LEN 4096
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t wData[0x100];
uint8_t rData[0x100];
uint16_t ID16;
uint32_t i;
uint8_t USART_RX_BUF[RX_BUF_LEN];

uint16_t write_addr = 0;

extern unsigned char gImage_1[];
unsigned char *image = gImage_1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f)
{
    uint8_t temp[1] = {ch};
    HAL_UART_Transmit(&huart1, temp, 1, 2);
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	/*##-1- Read the device ID  ########################*/ 
	BSP_W25Qx_Init();
    printf("\r\n SPI-W25Qxxx Example \r\n\r\n");
//    BSP_W25Qx_Read_ID(ID);
	ID16 = W25QXX_ReadID();

//	if((ID[0] != 0xEF) | (ID[1] != 0x17))
    if(ID16 != 0xef17)
	{
//        printf("\r\n 0x%.2x 0x%.2x \r\n\r\n",ID[0],ID[1]);
        printf("\r\n 0x%.4x \r\n\r\n",ID16);
        printf("\r\n ERROR! \r\n\r\n");
		Error_Handler();
	}
	else
	{
		printf(" CONNECTED ! \r\n W25Qxxx ID is : ");
//		for(i=0;i<2;i++)
//		{
//			printf("0x%02X ",ID[i]);
//		}
        printf("0x%.4x\r\n", ID16);
		printf("\r\n");
	}
	/*##-1.5- Cheak Flash 0xff ###########################*/ 
    #if 0
    W25QXX_Read(USART_RX_BUF, 0, 4096); 
	for(i=0; i<4096; i++)
	{
		if(USART_RX_BUF[i] > 0xf)
			printf("%x ", USART_RX_BUF[i]);
		else
			printf("0%x ", USART_RX_BUF[i]);
	}
	while(1){};
    #endif
    
//	/*##-2- Erase Block ##################################*/ 
//	if(BSP_W25Qx_Erase_Block(0) == W25Qx_OK)
//		printf(" QSPI Erase Block ok\r\n");
//	else
//    {
//        printf(" QSPI Erase Block error\r\n");
//        Error_Handler();
//    }
//		
//	
	/*##-2- Written to the flash ########################*/ 
	/* fill buffer */
    HAL_Delay(1000);
    printf("begin to write ------> \r\n");
    write_addr = PHOTO_NUM * 115200 ;
    for(i=0;i<115200/512;i++)
    {
        W25QXX_Write(image, write_addr,512);
        image+=512;
        write_addr+=512;
        printf("\r\n %d----> \r\n",i);
    }
    
//	/*##-3- Read the flash     ########################*/ 
//	if(BSP_W25Qx_Read(rData,0x00,0x100)== W25Qx_OK)
//		printf(" QSPI Read ok\r\n\r\n");
//	else
//		Error_Handler();
//	
//	printf("QSPI Read Data : \r\n");
//	for(i =0;i<0x100;i++)
//		printf("0x%02X  ",rData[i]);
//	printf("\r\n\r\n");
//	
//	/*##-4- check date          ########################*/ 	
//	if(memcmp(wData,rData,0x100) == 0 ) 
//		printf(" W25Q128FV QuadSPI Test OK\r\n");
//	else
//		printf(" W25Q128FV QuadSPI Test False\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
