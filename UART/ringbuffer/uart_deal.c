#include "my_ringbuffer.h"
#include "bsp_user_lib.h"

/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) 后面固定都为1750us
*/
typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
}MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
{	
    {2400,	16000}, /* 波特率2400bps, 3.5字符延迟时间16000us */
	{4800,	 8000}, 
	{9600,	 4000},
	{19200,	 2000},
	{38400,	 1750},
	{115200, 1750},
	{128000, 1750},
	{230400, 1750},
};

void UART_IRQ(struct RingBuffer_T *rb)
{
    uint8_t ch;

    /* ----------------------------- if receive IRQ ----------------------------- */
    if(IRQ_FLAG==UART1_IRQ_FLAG)    //TODO
    ch = READ_REG();    //TODO
    xRingBufPutChar(rb, ch);
    ClearFLAG();    //TODO

    /* ch to data_deal_callback */
    data_deal_callback(ch); //TODO

    /* ----------------------------- if transmit IRQ ---------------------------- */

}

/**
 * 
 * @brief 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
 *
**/
data_deal_callback(ch)
{
    ModBus_Deal(ch);
}

ModBus_Deal(ch)
{
    /*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		详情看此C文件开头
	*/
	uint8_t i;
	
	/* 根据波特率，获取需要延迟的时间 */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(SBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	g_mods_timeout = 0;
	
	/* 硬件定时中断，定时精度us 硬件定时器1用于MODBUS从机, 定时器2用于MODBUS主机*/
	bsp_StartHardTimer(1, ModbusBaudRate[i].usTimeOut, (void *)MODS_RxTimeOut);

	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = ch;
	}
}

//TODO, All on top of here

static uint32_t g_mods_timeout = 0; //全局modbus帧超时变量
/**
 * @brief 解析ModBus协议，在主程序中调用。
 *
 **/
void vModBusPoll(struct RingBuffer_T *rb)
{
	uint16_t addr;
	uint16_t crc1;
	
	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1; 通知主程序开始解码 */
	if (g_mods_timeout == 0)	
	{
		return;								/* 没有超时，继续接收。不要清零 g_tModS.RxCount */
	}
	
	g_mods_timeout = 0;	 					/* 清标志 */

	if (xGetRingBufDataLen(rb) < 4)				/* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
	{
		goto err_ret;
	}

	/* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
	crc1 = CRC16_Modbus(rb->dataBuf, g_tModS.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* 站地址 (1字节） */
	addr = g_tModS.RxBuf[0];				/* 第1字节 站号 */
	if (addr != SADDR485)		 			/* 判断主机发送的命令地址是否符合 */
	{
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODS_AnalyzeApp();						
	
err_ret:
	rb->read_index = 0;					/* 必须清零计数器，方便下次帧同步 */
}