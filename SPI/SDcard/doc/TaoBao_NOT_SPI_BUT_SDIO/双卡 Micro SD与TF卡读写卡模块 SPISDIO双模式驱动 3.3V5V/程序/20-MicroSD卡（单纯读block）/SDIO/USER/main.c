/******************** (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� ********************
 * �ļ���  ��main.c
 * ����    ��MicroSD��(SDIOģʽ)����ʵ�飬����������Ϣͨ������1�ڵ��Եĳ����ն���
 *           ��ӡ����         
 * ʵ��ƽ̨��Ұ��STM32������
 * ��汾  ��ST3.0.0
 *
 * ����    ��fire  QQ: 313303034 
 * ����    ��firestm32.blog.chinaunix.net
**********************************************************************************/
#include "stm32f10x.h"
#include "usart1.h"
#include "sdcard.h"


typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* �궨�� */
#define BlockSize            512 /* Block Size in Bytes */
#define BufferWordsSize      (BlockSize >> 2)
#define NumberOfBlocks       2  /* For Multi Blocks operation (Read/Write) */
#define MultiBufferWordsSize ((BlockSize * NumberOfBlocks) >> 2)

/* �������� */
SD_CardInfo SDCardInfo;
uint32_t Buffer_Block_Tx[BufferWordsSize], Buffer_Block_Rx[BufferWordsSize];
uint32_t Buffer_MultiBlock_Tx[MultiBufferWordsSize], Buffer_MultiBlock_Rx[MultiBufferWordsSize];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
SD_Error Status = SD_OK;

/* ����ԭ������ */
void NVIC_Configuration(void);
void Fill_Buffer(uint32_t *pBuffer, uint16_t BufferLenght, uint32_t Offset);
TestStatus Buffercmp(uint32_t* pBuffer1, uint32_t* pBuffer2, uint16_t BufferLength);
TestStatus eBuffercmp(uint32_t* pBuffer, uint16_t BufferLength);



/*
 * ��������main
 * ����  ����
 * ����  ����
 * ���  ����
 */
int main(void)
{	
	/* config the sysclock to 72M */   
	SystemInit();

  /* USART1 config */
	USART1_Config();

	/* Interrupt Config */
  NVIC_Configuration();

	printf( "\r\n ����һ��MicroSD��ʵ��(û�����ļ�ϵͳ)......... " );
			  
  /*-------------------------- SD Init ----------------------------- */ 
  Status = SD_Init(); 
  
  if (Status == SD_OK)
  {		  	
   /*----------------- Read CSD/CID MSD registers ------------------*/	 
   printf( " \r\n SD_Init is ok " );
	 Status = SD_GetCardInfo(&SDCardInfo);	 	   
  }

  //printf( " \r\n SD_GetCardInfo Status is: %d ", Status );
  printf( " \r\n CardType is ��%d ", SDCardInfo.CardType );
	printf( " \r\n CardCapacity is ��%d ", SDCardInfo.CardCapacity );
	printf( " \r\n CardBlockSize is ��%d ", SDCardInfo.CardBlockSize );
	printf( " \r\n RCA is ��%d ", SDCardInfo.RCA);
	printf( " \r\n ManufacturerID is ��%d ", SDCardInfo.SD_cid.ManufacturerID );
	 
  if (Status == SD_OK)
  {																	 
	  printf("\r\n SD_GetCardInfo is ok ");
    /*----------------- Select Card --------------------------------*/
    Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));  
  }

  if (Status == SD_OK)
  {
  	printf(" \r\n SD_SelectDeselect is ok  ");
		// SDIO_BusWide_4b �������ѭ��,����δ���			   
    Status = SD_EnableWideBusOperation(SDIO_BusWide_1b); 		  
  }
  
  /*------------------- Block Erase -------------------------------*/
  if (Status == SD_OK)
  {	 
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    Status = SD_Erase(0x00, (BlockSize * NumberOfBlocks));   
  }	 

  if (Status == SD_OK)
  { 
	 printf(" \r\n SD_Erase is ok "); 
   Status = SD_SetDeviceMode(SD_DMA_MODE);	 
  }  
 
  if (Status == SD_OK)
  {	 
     printf(" \r\n SD_SetDeviceMode is ok  ");
		 // 4bit ģʽʱ��ͣ�������ˣ�������ѭ����1bit ģʽʱ����Զ�ȡ�ɹ�	   
     Status = SD_ReadMultiBlocks(0x00, Buffer_MultiBlock_Rx, BlockSize, NumberOfBlocks);	 
  }  

  if (Status == SD_OK)
  {	     	
    printf(" \r\n SD_ReadMultiBlocks is ok  "); 
    EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MultiBufferWordsSize);
    printf( " \r\n eBuffercmp EraseStatus is %d ", EraseStatus ); 
  }
  
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_Block_Tx, BufferWordsSize, 0x12345678);


  if (Status == SD_OK)
  {		  
    /* Write block of 512 bytes on address 0 */
    Status = SD_WriteBlock(0x00, Buffer_Block_Tx, BlockSize);
  }

  if (Status == SD_OK)
  {	
	  printf( " \r\n SD_WriteBlock is ok " );
    /* Read block of 512 bytes from address 0 */
    Status = SD_ReadBlock(0x00, Buffer_Block_Rx, BlockSize);  
  }

  if (Status == SD_OK)
  {	
	  printf(" \r\n SD_ReadBlock is ok ");
    /* Check the corectness of written dada */
    TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BufferWordsSize);
  }

	if (TransferStatus1 == PASSED)
	{
		/* д��ȥ��ֵ�Ͷ�������ֵ��ͬ */
		printf( "\r\n Single Block Read/Write is ok " );
	}

  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_MultiBlock_Tx, MultiBufferWordsSize, 0x0);

  if (Status == SD_OK)
  {
    /* Write multiple block of many bytes on address 0 */
    Status = SD_WriteMultiBlocks(0x00, Buffer_MultiBlock_Tx, BlockSize, NumberOfBlocks); 
  }

  if (Status == SD_OK)
  {
    /* Read block of many bytes from address 0 */
    Status = SD_ReadMultiBlocks(0x00, Buffer_MultiBlock_Rx, BlockSize, NumberOfBlocks);	 
  }

  if (Status == SD_OK)
  {	 
    /* Check the corectness of written dada */
		/* д��ȥ��ֵ�Ͷ�������ֵ��ͬ */
    TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MultiBufferWordsSize);	  
  }

	if ( TransferStatus2 == PASSED )
	{
		printf( " \r\n Multiple Block Read/Write is ok " );
	}

	printf(" \r\n ����Ϣ��MicroSD����д���Գɹ� \r\n ");	

  while (1)
  {      
  }
} /* end of main */


/*
 * ��������NVIC_Configuration
 * ����  ��SDIO ���ȼ�����Ϊ������ȼ���
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
 * ��������Buffercmp
 * ����  ���Ƚ������������е������Ƿ����
 * ����  ��-pBuffer1, -pBuffer2 : Ҫ�ȽϵĻ�������ָ��
 *         -BufferLength ����������
 * ���  ��-PASSED ���
 *         -FAILED ����
 */
TestStatus Buffercmp(uint32_t* pBuffer1, uint32_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}


/*
 * ��������Fill_Buffer
 * ����  ���ڻ���������д����
 * ����  ��-pBuffer Ҫ���Ļ�����
 *         -BufferLength Ҫ���Ĵ�С
 *         -Offset ���ڻ������ĵ�һ��ֵ
 * ���  ���� 
 */
void Fill_Buffer(uint32_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLenght; index++ )
  {
    pBuffer[index] = index + Offset;
  }
}


/*
 * ��������eBuffercmp
 * ����  ����黺�����������Ƿ�Ϊ0
 * ����  ��-pBuffer Ҫ�ȽϵĻ�����
 *         -BufferLength ����������        
 * ���  ��PASSED ������������ȫΪ0
 *         FAILED ������������������һ����Ϊ0 
 */
TestStatus eBuffercmp(uint32_t* pBuffer, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer != 0x00)
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}



/******************* (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� *****END OF FILE****/
