/******************** (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� ********************
 * �ļ���  ��main.c
 * ����    ����MicroSD��(���ļ�ϵͳFATFS����)�����mp3�ļ����ų�����
 * ʵ��ƽ̨��Ұ��STM32������
 * ��汾  ��ST3.0.0
 *
 * ����    ��fire  QQ: 313303034
 * ����    ��firestm32.blog.chinaunix.net
**********************************************************************************/

#include "stm32f10x.h"
#include "usart1.h"
#include "SysTick.h"
#include "vs1003.h"

#include "sdcard.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>


/* �������� */
SD_CardInfo SDCardInfo;    // ���SD������Ϣ

FATFS fs;                  // Work area (file system object) for logical drive
FIL fsrc, fdst;            // file objects
BYTE buffer[512];          // file copy buffer
FRESULT res;               // FatFs function common result code
UINT br, bw;               // File R/W count

/* ����ԭ������ */
void NVIC_Configuration(void);
SD_Error SD_USER_Init(void);
void MP3_Play(void);

/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */
int main(void)
{
    /* ����ϵͳʱ��Ϊ 72M */
    SystemInit();

    /* ����SysTick Ϊ10us�ж�һ�� */
    SysTick_Init();

    /* ���ô���1 115200 8-N-1 */
    USART1_Config();

    /* SD���ж����� */
    NVIC_Configuration();

    USART1_printf(USART1, " \r\n ����һ��MP3�������� !\r\n ");

    /* MP3Ӳ����ʼ�� */
    VS1003_SPI_Init();

    /* SD ��Ӳ����ʼ��������ʼ���̷�Ϊ0 */
    disk_initialize(0);

    /* MP3������׼������ */
    MP3_Start();

    /* ����SD��(FATFS)�������Ƶ�ļ� */
    MP3_Play();

    while (1)
    {
    }

} /* end of main */


/*
 * ��������NVIC_Configuration
 * ����  ��SDIO �ж�ͨ������
 * ����  ����
 * ���  ����
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
 * ��������SD_USER_Init
 * ����  ��SD����ʼ����ֻ����һ���ɹ��ˣ�FATFS����������
 * ����  ����
 * ���  ��-SD_Error ����SD_OK��ʾ�ɹ�
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
        /* ��ѡһ�ֶ����Թ��� */
        Status = SD_SetDeviceMode(SD_DMA_MODE);
        //Status = SD_SetDeviceMode( SD_POLLING_MODE );
        //Status = SD_SetDeviceMode( SD_INTERRUPT_MODE );
    }
    return (Status);
}

/*
 * ��������MP3_Play
 * ����  ����ȡSD���������Ƶ�ļ�����ͨ���������ų���
 *         ֧�ֵĸ�ʽ��mp3,mid,wav,wma
 * ����  ����
 * ���  ����
 * ˵��  ���ݲ�֧�ֳ��ļ�������֧�����ġ�
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

    f_mount(0, &fs);                                    /* �����ļ�ϵͳ��0�� */

    if (f_opendir(&dirs, path) == FR_OK)              /* �򿪸�Ŀ¼ */
    {
        while (f_readdir(&dirs, &finfo) == FR_OK)       /* ���ζ�ȡ�ļ��� */
        {
            if (finfo.fattrib & AM_ARC)                 /* �ж��Ƿ�Ϊ�浵���ĵ� */
            {
                if (!finfo.fname[0])                               /* �ļ���Ϊ�ռ�������Ŀ¼��ĩβ���˳� */
                    break;
                USART1_printf(USART1, " \r\n the music file name is: %s \r\n", finfo.fname);

                result1 = strstr(finfo.fname, ".mp3");   /* �ж��Ƿ�Ϊ��Ƶ�ļ� */
                result2 = strstr(finfo.fname, ".mid");
                result3 = strstr(finfo.fname, ".wav");
                result4 = strstr(finfo.fname, ".wma");

                if (result1 != NULL || result2 != NULL || result3 != NULL || result4 != NULL)
                {
                    res = f_open(&fsrc, finfo.fname, FA_OPEN_EXISTING | FA_READ);   /* ��ֻ����ʽ�� */
                    br = 1;                         /* br Ϊȫ�ֱ��� */
                    TXDCS_SET(0);         /* ѡ��VS1003�����ݽӿ�    */
                    /* ------------------- һ����ʼ --------------------*/
                    USART1_printf(USART1, " \r\n ��ʼ���� \r\n");
                    for (;;)
                    {
                        res = f_read(&fsrc, buffer, sizeof(buffer), &br);
                        if (res == 0)
                        {
                            count = 0;                              /* 512�ֽ������¼��� */
                            Delay_us(1000);           /* 10ms */
                            while (count < 512)        /* SD����ȡһ��sector��һ��sectorΪ512�ֽ� */
                            {
                                if (DREQ != 0)         /* �ȴ�DREQΪ�ߣ������������� */
                                {
                                    for (j = 0; j < 32; j++) /* VS1003��FIFOֻ��32���ֽڵĻ��� */
                                    {
                                        VS1003_WriteByte(buffer[count]);
                                        count++;
                                    }
                                }
                            }
                        }
                        if (res || br == 0) break;   /* ������ߵ����ļ�β */
                    }
                    USART1_printf(USART1, " \r\n ���Ž��� \r\n\n\n\n\n");
                    /* ------------------- һ������ --------------------*/
                    count = 0;
                    /* ����VS1003��Ҫ����һ���������跢��2048��0��ȷ����һ�׵��������� */
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
                    TXDCS_SET(1);     /* �ر�VS1003���ݶ˿� */
                    f_close(&fsrc);   /* �رմ򿪵��ļ� */
                }
            }
        } /* end of while */
    }
} /* end of MP3_Play */

/******************* (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� *****END OF FILE****/
