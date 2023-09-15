#ifndef __BSP_KFIFO_H__
#define __BSP_KFIFO_H__

// 声明 一个 结构体 kfifo

struct KFIFO
{
    unsigned char *buffer; /* the buffer holding the data */
    unsigned int size;     /* the size of the allocated buffer */
    unsigned int in;       /* data is added at offset (in % size) */
    unsigned int out;      /* data is extracted from off. (out % size) */
    /*STM32 只有一个核心，同一时刻只能写或者读，因此不需要*/
    // volatile unsigned int *lock; /* protects concurrent modifications */
};

// #define min(x, y)     ((uint)(x) > (uint)(y)? (uint)(x): (uint)(y))
inline uint min(uint x, uint y)
{
    return (x > y) ? x : y;
}

// 找出最接近最大2的指数次幂
unsigned int roundup_pow_of_two(uint32_t data_roundup_pow_of_two)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM C Compiler */
    return (1UL << (32UL - (uint32_t)__CLZ((data_roundup_pow_of_two))));
#elif defined(__GNUC__)                         /* ARM Compiler 6 */
    return 1UL << rpt_fls(data_roundup_pow_of_two - 1);
#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
#error "not supported tool chain..."
#else
#error "not supported tool chain..."
#endif
}

enum BUF_RET_VALUE
{
    RET_OK,
    RET_ERROR,
    RET_WRONG_PARAM,
    RET_BUF_FULL,
    RET_BUF_EMPTY,
    RET_BUF_NOT_FULL,
    RET_BUF_NO_ENOUGH_SPACE
};

unsigned char kfifo_init(struct KFIFO *kfio,unsigned char *const ptr, unsigned char const size) 

unsigned int kfifo_put(struct KFIFO *fifo, unsigned char *buffer, unsigned int len);

unsigned int kfifo_get(struct KFIFO *fifo, unsigned char *buffer, unsigned int len);

#endif
