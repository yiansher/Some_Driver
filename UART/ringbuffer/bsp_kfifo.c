#include "bsp_kfifo.h"

#define FIO_NUM_MAX 3
struct KFIFO kfio_buffer_t[FIO_NUM_MAX];

/**
 * @brief Initialize the ring buffer object.
 *
 * @param kfio      A pointer to the ring buffer object.
 * @param ptr              A pointer to the buffer.
 * @param size              The size of the buffer in bytes.
 */
unsigned char xKfifo_Init(struct KFIFO *kfio, unsigned char *const ptr, unsigned int size)
{
    if (kfio == NULL || ptr == NULL)
        return RET_WRONG_PARAM;
    /*
     * round up to the next power of 2, since our 'let the indices
     * wrap' tachnique works only in this case.
     * 如果size 是2的 次幂圆整，则 size & (size - 1)  =0
     */
    if (size & (size - 1))
    {
        // 如果你要申请的buffer 不是 2的 次幂圆整，就要把 size 变成 2的次幂圆整 ，方便下面计算
        size = roundup_pow_of_two(size);
    }
    kfio->buffer = ptr;
    kfio->size = size;
    kfio->mask = size - 1;
    kfio->in = 0;
    kfio->out = 0;
    return RET_OK;
}

/**
 * @brief Put a block of data into the ring buffer.
 *
 * @param fifo      A pointer to the ring buffer object.
 * @param buffer    A pointer to the data buffer.
 * @param len       The size of data in bytes.
 *
 * @return Return the data size we put into the ring buffer.
 */
unsigned int xKfifoPut(struct KFIFO *fifo, unsigned char *const buffer, unsigned int len)
{
    unsigned int L;

    // 环形缓冲区的剩余容量为fifo->size - fifo->in + fifo->out，让写入的长度取len和剩余容量中较小的，避免写越界；
    len = min(len, fifo->size - fifo->in + fifo->out);

    /*
     * Ensure that we sample the fifo->out index -before- we
     * start putting bytes into the kfifo.
     */
    // 多处理器 处理内存 的 屏障，STM32不需要这个
    //     smp_mb();

    /* first put the data starting from fifo->in to buffer end */
    /* 首先将数据从fifo.in 所在的位置开始写，写之前，首先要看一下fifo->in到 buffer 末尾的大小 是不是 比 len 大*/

    /*
     * 前面讲到fifo->size已经2的次幂圆整，主要是方便这里计算，提升效率
     * 在对10进行求余的时候，我们发现，余数总是整数中的个位上的数字，而不用管其他位是什么；
     * 所以,kfifo->in % kfifo->size 可以转化为 kfifo->in & (kfifo->size – 1)，效率会提升
     * 所以fifo->size - (fifo->in & (fifo->size - L)) 即位 fifo->in 到 buffer末尾所剩余的长度，
     * L取len和剩余长度的最小值，即为需要拷贝L 字节到fifo->buffer + fifo->in的位置上。
     */
    L = min(len, fifo->size - (fifo->in & fifo->mask));

    memcpy(fifo->buffer + (fifo->in & fifo->mask), buffer, L);

    /* then put the rest (if any) at the beginning of the buffer */

    memcpy(fifo->buffer, buffer + L, len - L);

    /*
     * Ensure that we add the bytes to the kfifo -before-
     * we update the fifo->in index.
     */

    // smp_wmb();   //多处理器 处理内存 的 屏障，STM32不需要这个

    /*
     * 注意这里 只是用了 fifo->in +=  len而未取模，
     * 这就是kfifo的设计精妙之处，这里用到了unsigned int的溢出性质，
     * 当in 持续增加到溢出时又会被置为0，这样就节省了每次in向前增加都要取模的性能，
     * 锱铢必较，精益求精，让人不得不佩服。
     */

    fifo->in += len;

    /* 返回值代表写入数据的个数，这样就可以根据返回值，判断缓冲区是否写满 */
    return len;
}

/**
 * @brief Get data from the ring buffer.
 *
 * @param fifo          A pointer to the ring buffer.
 * @param buffer        A pointer to the data buffer.
 * @param len           The size of the data we want to read from the ring buffer.
 *
 * @return Return the data size we read from the ring buffer.
 */
unsigned int xKfifoGet(struct KFIFO *fifo, unsigned char *const buffer, unsigned int len)
{

    if(xKfifoGetSpace(fifo)==0)
        return 0;

    unsigned int L;

    len = min(len, fifo->in - fifo->out);

    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */

    // smp_rmb();    //多处理器 处理内存 的 屏障，STM32不需要这个

    /* first get the data from fifo->out until the end of the buffer */
    L = min(len, fifo->size - (fifo->out & fifo->mask));
    memcpy(buffer, fifo->buffer + (fifo->out & fifo->mask), L);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + L, fifo->buffer, len - L);

    /*
     * Ensure that we remove the bytes from the kfifo -before-
     * we update the fifo->out index.
     */

    // smp_mb();   //多处理器 处理内存 的 屏障，STM32不需要这个

    /*
     * 注意这里 只是用了 fifo->out +=  len 也未取模运算，
     * 同样unsigned int的溢出性质，当out 持续增加到溢出时又会被置为0，
     * 如果in先溢出，出现 in  < out 的情况，那么 in – out 为负数（又将溢出），
     * in – out 的值还是为buffer中数据的长度。
     */

    fifo->out += len;

    return len;
}

void vKfifoPutchar(struct KFIFO *fifo, unsigned char dataIn)
{
    fifo->buffer[fifo->in & fifo->mask] = dataIn;
    fifo->in++;
}

unsigned char xKfifoGetchar(struct KFIFO *fifo)
{
    unsigned char temp;
    temp = fifo->buffer[fifo->out & fifo->mask];
    fifo->out++;
    return temp;
}

void vKfifoClear(struct KFIFO *fifo)
{
    fifo->in = 0;
	fifo->out = 0;
	fifo->size = 0;
	fifo->buffer = NULL;
	fifo->mask = 0;
}

int16_t uart1_send_by_int(const uint8_t *data, uint16_t len)
{
    if (xKfifoGetSpace(&uart1TxFifo) >= len) // 只有空闲区>len，才执行发送程序
    {
        gfifo_put(&uart1TxFifo, data, len);
    }

    else
    {
        log_i("uart1 SendFifo has no spacern"); // 程序走到这里，意味着FIFO缓冲不足，会出现发送丢失
        return 0;
    }

    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    return 1;
}

//执行完上述发送函数后，硬件发现DR寄存器中没有数据，会立即进入TXE中断，接下来我们写TXE中断的服务函数：

void USART1_IRQHandler(void) // 串口1中断服务程序
{
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) // 数据寄存器DR空中断TXE
    {
        if (get_fifo_used_size(&uart1TxFifo) > 0) // main调用链中操作uart1TxFifo的地方必须禁掉本中断(或全局中断)
        {
            uint8_t sendCh;
            // 从FIFO中取出一个字节并发送，这个字节一旦被从DR移入移位寄存器，就会再次进入本中断
            xKfifoGet(&uart1TxFifo, &sendCh, 1);
            USART1->DR = sendCh;
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE); // FIFO中的所有数据都已发完，关中断
        }
    }
}
