/*********************************************************************************************************
*
* File                : ws_W25Qx.c
* Hardware Environment: 
* Build Environment   : RealView MDK-ARM  Version: 4.20
* Version             : V1.0
* By                  : 
*
*                                  (c) Copyright 2005-2011, WaveShare
*                                       http://www.waveshare.net
*                                          All Rights Reserved
*
*********************************************************************************************************/

#include "W25Qx.h"


/**
  * @brief  Initializes the W25Q128FV interface.
  * @retval None
  */
uint8_t BSP_W25Qx_Init(void)
{ 
	/* Reset W25Qxxx */
	BSP_W25Qx_Reset();
	
	return BSP_W25Qx_GetStatus();
}

/**
  * @brief  This function reset the W25Qx.
  * @retval None
  */
static void	BSP_W25Qx_Reset(void)
{
	uint8_t cmd[2] = {RESET_ENABLE_CMD,RESET_MEMORY_CMD};
	
	W25Qx_Enable();
	/* Send the reset command */
	HAL_SPI_Transmit(&hspi2, cmd, 2, W25Qx_TIMEOUT_VALUE);	
	W25Qx_Disable();

}

/**
  * @brief  Reads current status of the W25Q128FV.
  * @retval W25Q128FV memory status
  */
static uint8_t BSP_W25Qx_GetStatus(void)
{
	uint8_t cmd[] = {READ_STATUS_REG1_CMD};
	uint8_t status;
	
	W25Qx_Enable();
	/* Send the read status command */
	HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	HAL_SPI_Receive(&hspi2,&status, 1, W25Qx_TIMEOUT_VALUE);
	W25Qx_Disable();
	
	/* Check the value of the register */
  if((status & W25Q128FV_FSR_BUSY) != 0)
  {
    return W25Qx_BUSY;
  }
	else
	{
		return W25Qx_OK;
	}		
}
#if BSP
/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @retval None
  */
uint8_t BSP_W25Qx_WriteEnable(void)
{
	uint8_t cmd[] = {WRITE_ENABLE_CMD};
	uint32_t tickstart = HAL_GetTick();

	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	
	return W25Qx_OK;
}

/**
  * @brief  Read Manufacture/Device ID.
	* @param  return value address
  * @retval None
  */
void BSP_W25Qx_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD,0x00,0x00,0x00};
//	uint8_t *ID1 = ID;
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	HAL_SPI_Receive(&hspi2,ID+1, 1, W25Qx_TIMEOUT_VALUE);

    HAL_SPI_Receive(&hspi2,(ID), 1, W25Qx_TIMEOUT_VALUE);
	W25Qx_Disable();
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read    
  * @retval QSPI memory status
  */
uint8_t BSP_W25Qx_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
	uint8_t cmd[4];

	/* Configure the command */
	cmd[0] = READ_CMD;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)(ReadAddr);
	
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	if (HAL_SPI_Receive(&hspi2, pData,Size,W25Qx_TIMEOUT_VALUE) != HAL_OK)
  {
    return W25Qx_ERROR;
  }
	W25Qx_Disable();
	return W25Qx_OK;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be written
  * @param  WriteAddr: Write start address
  * @param  Size: Size of data to write,No more than 256byte.    
  * @retval QSPI memory status
  */
uint8_t BSP_W25Qx_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
	uint8_t cmd[4];
	uint32_t end_addr, current_size, current_addr;
	uint32_t tickstart = HAL_GetTick();
	
	/* Calculation of the size between the write address and the end of the page */
  current_addr = 0;

  while (current_addr <= WriteAddr)
  {
    current_addr += W25Q128FV_PAGE_SIZE;
  }
  current_size = current_addr - WriteAddr;

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;
	
  /* Perform the write page by page */
  do
  {
		/* Configure the command */
		cmd[0] = PAGE_PROG_CMD;
		cmd[1] = (uint8_t)(current_addr >> 16);
		cmd[2] = (uint8_t)(current_addr >> 8);
		cmd[3] = (uint8_t)(current_addr);

		/* Enable write operations */
		BSP_W25Qx_WriteEnable();
	
		W25Qx_Enable();
    /* Send the command */
    if (HAL_SPI_Transmit(&hspi2,cmd, 4, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qx_ERROR;
    }
    
    /* Transmission of the data */
    if (HAL_SPI_Transmit(&hspi2, pData,current_size, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qx_ERROR;
    }
			W25Qx_Disable();
    	/* Wait the end of Flash writing */
		while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
		{
			/* Check for the Timeout */
			if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
			{        
				return W25Qx_TIMEOUT;
			}
		}
    
    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + W25Q128FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q128FV_PAGE_SIZE;
  } while (current_addr < end_addr);

	
	return W25Qx_OK;
}

/**
  * @brief  Erases the specified block of the QSPI memory. 
  * @param  BlockAddress: Block address to erase  
  * @retval QSPI memory status
  */
uint8_t BSP_W25Qx_Erase_Block(uint32_t Address)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = SECTOR_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);
	
	/* Enable write operations */
	BSP_W25Qx_WriteEnable();
	
	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi2, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Q128FV_SECTOR_ERASE_MAX_TIME)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	return W25Qx_OK;
}

/**
  * @brief  Erases the entire QSPI memory.This function will take a very long time.
  * @retval QSPI memory status
  */
uint8_t BSP_W25Qx_Erase_Chip(void)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = SECTOR_ERASE_CMD;
	
	/* Enable write operations */
	BSP_W25Qx_WriteEnable();
	
	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi2, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() != W25Qx_BUSY)
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Q128FV_BULK_ERASE_MAX_TIME)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	return W25Qx_OK;
}
#endif

//SPI ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
static uint8_t SPI1_ReadWriteByte(uint8_t TxData)
{
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&hspi2, &TxData, &Rxdata, 1, 1);       
    return Rxdata;                      //�����յ�������       
}

uint16_t W25QXX_TYPE;                   //����W25QXXоƬ�ͺ�
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;    
    /* ʹ��Ƭѡ */
    W25Qx_Enable();                
    SPI1_ReadWriteByte(0x90);//���Ͷ�ȡID����     
    SPI1_ReadWriteByte(0x00);       
    SPI1_ReadWriteByte(0x00);       
    SPI1_ReadWriteByte(0x00);                  
    Temp|=SPI1_ReadWriteByte(0xFF)<<8;  
    Temp|=SPI1_ReadWriteByte(0xFF);  
    W25QXX_TYPE=Temp;
    /* ȡ��Ƭѡ */
    W25Qx_Disable();       
    return Temp;
} 

//��ȡW25QXX��״̬�Ĵ�����W25QXXһ����3��״̬�Ĵ���
//״̬�Ĵ���1��
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
//״̬�Ĵ���2��
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//״̬�Ĵ���3��
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:״̬�Ĵ����ţ���:1~3
//����ֵ:״̬�Ĵ���ֵ
uint8_t W25QXX_ReadSR(uint8_t regno)   
{  
    uint8_t byte=0,command=0; 
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //��״̬�Ĵ���1ָ��
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //��״̬�Ĵ���2ָ��
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //��״̬�Ĵ���3ָ��
            break;
        default:
            command=W25X_ReadStatusReg1;    
            break;
    }    
    /* ʹ��Ƭѡ */
    W25Qx_Enable();
    SPI1_ReadWriteByte(command);            //���Ͷ�ȡ״̬�Ĵ�������    
    byte=SPI1_ReadWriteByte(0Xff);          //��ȡһ���ֽ�  
    /* ȡ��Ƭѡ */
    W25Qx_Disable();
    return byte;   
} 

//�ȴ�����
void W25QXX_Wait_Busy(void)   
{   
    while((W25QXX_ReadSR(1)&0x01)==0x01);   // �ȴ�BUSYλ���
}

//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
    uint16_t i;                             
    /* ʹ��Ƭѡ */
    W25Qx_Enable();
    SPI1_ReadWriteByte(W25X_ReadData);      //���Ͷ�ȡ����  
    if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
    {
        SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>24));    
    }
    SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>16));   //����24bit��ַ    
    SPI1_ReadWriteByte((uint8_t)((ReadAddr)>>8));   
    SPI1_ReadWriteByte((uint8_t)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
    { 
        pBuffer[i]=SPI1_ReadWriteByte(0XFF);    //ѭ������  
    }
    /* ȡ��Ƭѡ */
    W25Qx_Disable();                          
}

//W25QXXдʹ�� 
//��WEL��λ   
void W25QXX_Write_Enable(void)   
{
    W25Qx_Enable();                       
    SPI1_ReadWriteByte(W25X_WriteEnable);   //����дʹ��  
    W25Qx_Disable();                                 
} 

//W25QXXд��ֹ 
//��WEL����  
void W25QXX_Write_Disable(void)   
{  
    W25Qx_Enable();                        
    SPI1_ReadWriteByte(W25X_WriteDisable);  //����д��ָֹ��    
    W25Qx_Disable();
}    

//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ������������ʱ��:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)   
{  
    //����falsh�������,������   
    //printf("fe:%x\r\n",Dst_Addr);   
    Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL    
    W25QXX_Wait_Busy();   
    /* ʹ��Ƭѡ */
    W25Qx_Enable();
    SPI1_ReadWriteByte(W25X_SectorErase);   //������������ָ�� 
    if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
    {
        SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>24)); 
    }
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //����24bit��ַ    
    SPI1_ReadWriteByte((uint8_t)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((uint8_t)Dst_Addr);  
    /* ȡ��Ƭѡ */
    W25Qx_Disable();      
    W25QXX_Wait_Busy();                     //�ȴ��������
} 

//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!   
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint16_t i;  
    W25QXX_Write_Enable();                  //SET WEL 
    /* ʹ��Ƭѡ */
    W25Qx_Enable();    
    SPI1_ReadWriteByte(W25X_PageProgram);   //����дҳ����   
    if(W25QXX_TYPE==W25Q256)                //�����W25Q256�Ļ���ַΪ4�ֽڵģ�Ҫ�������8λ
    {
        SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>24)); 
    }
    SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //����24bit��ַ    
    SPI1_ReadWriteByte((uint8_t)((WriteAddr)>>8));   
    SPI1_ReadWriteByte((uint8_t)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//ѭ��д�� 
    /* ȡ��Ƭѡ */    
    W25Qx_Disable();  
    W25QXX_Wait_Busy();                    //�ȴ�д�����
} 

//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{                    
    uint16_t pageremain;       
    pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���                
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
    while(1)
    {      
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//д�������
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;  

            NumByteToWrite-=pageremain;           //��ȥ�Ѿ�д���˵��ֽ���
            if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
            else pageremain=NumByteToWrite;       //����256���ֽ���
        }
    };      
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)                      
//NumByteToWrite:Ҫд����ֽ���(���65535)   
uint8_t W25QXX_BUFFER[4096];         
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;    
    uint16_t i;    
    uint8_t * W25QXX_BUF;     
    W25QXX_BUF=W25QXX_BUFFER;        
    secpos=WriteAddr/4096;//������ַ  
    secoff=WriteAddr%4096;//�������ڵ�ƫ��
    secremain=4096-secoff;//����ʣ��ռ��С   
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
    while(1) 
    {   
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//������������������
        for(i=0;i<secremain;i++)//У������
        {
            if(W25QXX_BUF[secoff+i]!=0XFF)break;//��Ҫ����        
        }
        if(i<secremain)//��Ҫ����
        {
            W25QXX_Erase_Sector(secpos);//�����������
            for(i=0;i<secremain;i++)       //����
            {
                W25QXX_BUF[i+secoff]=pBuffer[i];      
            }
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//д����������  

        }else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.                  
        if(NumByteToWrite==secremain)break;//д�������
        else//д��δ����
        {
            secpos++;//������ַ��1
            secoff=0;//ƫ��λ��Ϊ0    

            pBuffer+=secremain;  //ָ��ƫ��
            WriteAddr+=secremain;//д��ַƫ��       
            NumByteToWrite-=secremain;              //�ֽ����ݼ�
            if(NumByteToWrite>4096)secremain=4096;  //��һ����������д����
            else secremain=NumByteToWrite;          //��һ����������д����
        }    
    };   
}
