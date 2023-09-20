#include "bsp_kfifo.h"
#include "bsp_user_lib.h"
#include "bsp_timer.h"

#define RTU_MODE
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
} MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
	{
		{2400, 16000}, /* 波特率2400bps, 3.5字符延迟时间16000us */
		{4800, 8000},
		{9600, 4000},
		{19200, 2000},
		{38400, 1750},
		{115200, 1750},
		{128000, 1750},
		{230400, 1750},
};

#define _BaudRate 2u

/**
 *
 *	@brief Init FIFO
 *
 *	@return If global struct KFIFO init successful
 **/
#define RX_NUM_MAX 32u
uint8_t g_ringBuffer[RX_NUM_MAX];
enum
{
	UART1_RX,
	KFIFO_NUM_MAX
};
struct KFIFO g_kfio[UART1_RX];
uint8_t xRingBufferInit(void)
{
	return xKfifo_Init(&g_kfio, &g_ringBuffer, RX_NUM_MAX);
}

/**
 *	@brief 超过3.5个字符时间后执行本函数。 设置全局变量 g_mods_timeout = 1，通知主程序开始解码。
 *
 *	@return NULL
 **/
static uint32_t g_mods_timeout = 0; // 全局modbus帧超时变量

static void MODS_RxTimeOut(void)
{
	g_mods_timeout = 1;
}

void UART_IRQ(struct RingBuffer_T *rb)
{
	uint8_t ch;

	/* ----------------------------- if receive IRQ ----------------------------- */
	if (IRQ_FLAG == UART1_IRQ_FLAG) // TODO
		ch = READ_REG();			// TODO
	ClearFLAG();					// TODO

/* ch to data_deal_callback */
#ifdef RTU_MODE
	data_deal_callback(ch);
#endif
	/* ----------------------------- if transmit IRQ ---------------------------- */
}

/**
 *
 * @brief 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
 *
 **/
void data_deal_callback(uint8_t ch)
{
	ModBus_Deal(ch); // can change any callback you want
}

void ModBus_Deal(uint8_t ch)
{
#ifdef RTU_MODE
	/*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		详情看此C文件开头
	*/
	g_mods_timeout = 0;

	/* 硬件定时中断，定时精度us 硬件定时器1用于MODBUS从机, 定时器2用于MODBUS主机*/
	bsp_StartHardTimer(1, ModbusBaudRate[_BaudRate].usTimeOut, (void *)MODS_RxTimeOut);

	vKfifoPutchar(&g_kfio, ch);
#endif
}

/**
 * @brief 解析ModBus协议，在主程序中调用。
 *
 **/
#define SLAVE_ADDR	0x02
void vModBusPoll(struct KFIFO *kfio)
{
	uint16_t addr;
	uint16_t crc1;
	uint32_t rx_count = 0;

	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1; 通知主程序开始解码 */
	if (g_mods_timeout == 0)
	{
		return; /* 没有超时，继续接收。不要清零 g_tModS.RxCount */
	}

	g_mods_timeout = 0; /* 清标志 */
	rx_count = xKfifoGetDataLen(kfio);

	if (rx_count < 4) /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
	{
		goto err_ret;
	}

	/* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
	crc1 = CRC16_Modbus(kfio->buffer,rx_count);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* 站地址 (1字节） */
	addr = kfio->buffer[kfio->in-rx_count]; /* 第1字节 站号 */
	if (addr != SLAVE_ADDR)	 /* 判断主机发送的命令地址是否符合 */
	{
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODS_AnalyzeApp(kfio->buffer[kfio->in-rx_count+1]);

err_ret:
	vKfifoClear(&kfio); /* 必须清零计数器，方便下次帧同步 */
}

/**
 * @brief Analyze application layer protocols
*/
static void MODS_AnalyzeApp(uint8_t cmd)
{
	switch (cmd)				/* 第2个字节 功能码 */
	{
		case 0x01:							/* 读取线圈状态（此例程用led代替）*/
			MODS_01H();
			bsp_PutMsg(MSG_MODS_01H, 0);	/* 发送消息,主程序处理 */
			break;

		case 0x02:							/* 读取输入状态（按键状态）*/
			MODS_02H();
			bsp_PutMsg(MSG_MODS_02H, 0);
			break;
		
		case 0x03:							/* 读取保持寄存器（此例程存在g_tVar中）*/
			MODS_03H();
			bsp_PutMsg(MSG_MODS_03H, 0);
			break;
		
		case 0x04:							/* 读取输入寄存器（ADC的值）*/
			MODS_04H();
			bsp_PutMsg(MSG_MODS_04H, 0);
			break;
		
		case 0x05:							/* 强制单线圈（设置led）*/
			MODS_05H();
			bsp_PutMsg(MSG_MODS_05H, 0);
			break;
		
		case 0x06:							/* 写单个保存寄存器（此例程改写g_tVar中的参数）*/
			MODS_06H();	
			bsp_PutMsg(MSG_MODS_06H, 0);
			break;
			
		case 0x10:							/* 写多个保存寄存器（此例程存在g_tVar中的参数）*/
			MODS_10H();
			bsp_PutMsg(MSG_MODS_10H, 0);
			break;
		
		default:
			g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* 告诉主机命令错误 */
			break;
	}
}