/******************** (C) COPYRIGHT 2011 野火嵌入式开发工作室 ********************
 * 文件名  ：main.c
 * 描述    ：将MicroSD卡(以文件系统FATFS访问)里面的mp3文件播放出来。
 * 实验平台：野火STM32开发板
 * 库版本  ：ST3.0.0
 *
 * 作者    ：fire  QQ: 313303034
 * 博客    ：firestm32.blog.chinaunix.net
**********************************************************************************/

#include "stm32f10x.h"
#include "usart1.h"
#include "SysTick.h"
#include "vs1003.h"

#include "sdcard.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>


/* 变量定义 */
SD_CardInfo SDCardInfo;    // 存放SD卡的信息

FATFS fs;                  // Work area (file system object) for logical drive
FIL fsrc, fdst;            // file objects
BYTE buffer[512];          // file copy buffer
FRESULT res;               // FatFs function common result code
UINT br, bw;               // File R/W count

/* 函数原型声明 */
void NVIC_Configuration(void);
SD_Error SD_USER_Init(void);
void MP3_Play(void);

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */
int main(void)
{
    /* 配置系统时钟为 72M */
    SystemInit();

    /* 配置SysTick 为10us中断一次 */
    SysTick_Init();

    /* 配置串口1 115200 8-N-1 */
    USART1_Config();

    /* SD卡中断配置 */
    NVIC_Configuration();

    USART1_printf(USART1, " \r\n 这是一个MP3测试例程 !\r\n ");

    /* MP3硬件初始化 */
    VS1003_SPI_Init();

    /* SD 卡硬件初始化，并初始化盘符为0 */
    disk_initialize(0);

    /* MP3就绪，准备播放 */
    MP3_Start();

    /* 播放SD卡(FATFS)里面的音频文件 */
    MP3_Play();

    while (1)
    {
    }

} /* end of main */


/*
 * 函数名：NVIC_Configuration
 * 描述  ：SDIO 中断通道配置
 * 输入  ：无
 * 输出  ：无
 */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * 函数名：SD_USER_Init
 * 描述  ：SD卡初始化，只有这一步成功了，FATFS才能跑起来
 * 输入  ：无
 * 输出  ：-SD_Error 返回SD_OK表示成功
 */
SD_Error SD_USER_Init(void)
{
    SD_Error Status = SD_OK;

    /* SD Init */
    Status = SD_Init();

    if (Status == SD_OK)
    {
        /* Read CSD/CID MSD registers */
        Status = SD_GetCardInfo(&SDCardInfo);
    }

    if (Status == SD_OK)
    {
        /* Select Card */
        Status = SD_SelectDeselect((u32)(SDCardInfo.RCA << 16));
    }

    if (Status == SD_OK)
    {
        /* set bus wide */
        Status = SD_EnableWideBusOperation(SDIO_BusWide_1b);
    }

    /* Set Device Transfer Mode to DMA */
    if (Status == SD_OK)
    {
        /* 任选一种都可以工作 */
        Status = SD_SetDeviceMode(SD_DMA_MODE);
        //Status = SD_SetDeviceMode( SD_POLLING_MODE );
        //Status = SD_SetDeviceMode( SD_INTERRUPT_MODE );
    }
    return (Status);
}

/*
 * 函数名：MP3_Play
 * 描述  ：读取SD卡里面的音频文件，并通过耳机播放出来
 *         支持的格式：mp3,mid,wav,wma
 * 输入  ：无
 * 输出  ：无
 * 说明  ：暂不支持长文件名，不支持中文。
 */
void MP3_Play(void)
{
    FRESULT res;
    FILINFO finfo;
    DIR dirs;
    u16 count = 0;
    char j = 0;
    char path[50] = {""};
    char *result1, *result2, *result3, *result4;

    f_mount(0, &fs);                                    /* 挂载文件系统到0区 */

    if (f_opendir(&dirs, path) == FR_OK)              /* 打开根目录 */
    {
        while (f_readdir(&dirs, &finfo) == FR_OK)       /* 依次读取文件名 */
        {
            if (finfo.fattrib & AM_ARC)                 /* 判断是否为存档型文档 */
            {
                if (!finfo.fname[0])                               /* 文件名为空即到达了目录的末尾，退出 */
                    break;
                USART1_printf(USART1, " \r\n the music file name is: %s \r\n", finfo.fname);

                result1 = strstr(finfo.fname, ".mp3");   /* 判断是否为音频文件 */
                result2 = strstr(finfo.fname, ".mid");
                result3 = strstr(finfo.fname, ".wav");
                result4 = strstr(finfo.fname, ".wma");

                if (result1 != NULL || result2 != NULL || result3 != NULL || result4 != NULL)
                {
                    res = f_open(&fsrc, finfo.fname, FA_OPEN_EXISTING | FA_READ);   /* 以只读方式打开 */
                    br = 1;                         /* br 为全局变量 */
                    TXDCS_SET(0);         /* 选择VS1003的数据接口    */
                    /* ------------------- 一曲开始 --------------------*/
                    USART1_printf(USART1, " \r\n 开始播放 \r\n");
                    for (;;)
                    {
                        res = f_read(&fsrc, buffer, sizeof(buffer), &br);
                        if (res == 0)
                        {
                            count = 0;                              /* 512字节完重新计数 */
                            Delay_us(1000);           /* 10ms */
                            while (count < 512)        /* SD卡读取一个sector，一个sector为512字节 */
                            {
                                if (DREQ != 0)         /* 等待DREQ为高，请求数据输入 */
                                {
                                    for (j = 0; j < 32; j++) /* VS1003的FIFO只有32个字节的缓冲 */
                                    {
                                        VS1003_WriteByte(buffer[count]);
                                        count++;
                                    }
                                }
                            }
                        }
                        if (res || br == 0) break;   /* 出错或者到了文件尾 */
                    }
                    USART1_printf(USART1, " \r\n 播放结束 \r\n\n\n\n\n");
                    /* ------------------- 一曲结束 --------------------*/
                    count = 0;
                    /* 根据VS1003的要求，在一曲结束后需发送2048个0来确保下一首的正常播放 */
                    while (count < 2048)
                    {
                        if (DREQ != 0)
                        {
                            for (j = 0; j < 32; j++)
                            {
                                VS1003_WriteByte(0);
                                count++;
                            }
                        }
                    }
                    count = 0;
                    TXDCS_SET(1);     /* 关闭VS1003数据端口 */
                    f_close(&fsrc);   /* 关闭打开的文件 */
                }
            }
        } /* end of while */
    }
} /* end of MP3_Play */

/******************* (C) COPYRIGHT 2011 野火嵌入式开发工作室 *****END OF FILE****/
