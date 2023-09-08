#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "hello.h"

// #define UART_DEBUG

#define MY_ASSERT(x)

#define ALIGN_SIZE 8
#define AlIGN_DOWN(size, align) ((size) & ~((align)-1))
#define ALIGN(size, align) (((size) + (align)-1) & ~((align)-1))

enum RINGBUF_GROUP
{
    RB_UART1,
    RB_UART2,
    RB_UART3
};

struct RingBuffer_T
{
    uint8_t *dataBuf;
    uint8_t status;
    /* as we use msb of index as mirror bit, the size should be signed and could only be positive. */
    int32_t dataBufSize;
    /**
     * use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuffer is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     **/
    uint32_t read_mirror : 1;
    uint32_t read_index : 31;
    uint32_t write_mirror : 1;
    uint32_t write_index : 31;
};

enum
{
    RINGBUF_FREE,
    RINGBUF_INUSE
}; /* RingBuffer_T->Status */

enum
{
    RINGBUF_RET_SUCCESS = 0,        /*对队列操作成功 */
    RINGBUF_RET_NOT_OPEN,           /*没有申请队列句柄 */
    RINGBUF_RET_REGISTER_ABORT,     /*申请队列句柄异常 */
    RINGBUF_RET_BUF_OUT_OF_MAX,     /*申请队列句柄个数到达 */
    RINGBUF_RET_ERR_PARAMETER,      /*传入队列参数出错 */
    RINGBUF_RET_BUF_FULL,           /*队列满 */
    RINGBUF_RET_BUF_EMPTY,          /*队列空 */
    RINGBUF_RET_BUF_NOT_FULL,       /*队列非满 */
    RINGBUF_RET_BUF_NOT_EMPTY,      /*队列非空 */
    RINGBUF_RET_BUF_NO_ENOUGH_BUF,  /*队列缓存区不足 */
    RINGBUF_RET_BUF_NO_ENOUGH_DATA, /*队列缓存区没有足够的数据 */
    RINGBUF_RET_ERR_UNEXPECTED
};

void vRingBufInit(void);
uint8_t xRingBufRegister(struct RingBuffer_T *pxNewRingBuf, uint8_t *const iDataBuf, const uint32_t iDataBufSize);
uint8_t xRingBufFree(struct RingBuffer_T *const pxRingBuf);
inline uint8_t xCheckRingBufIsFull(struct RingBuffer_T *const pxRingBuf);
uint32_t xGetRingBufDataLen(struct RingBuffer_T *const pxRingBuf);
uint32_t xRingBufPut(struct RingBuffer_T *const pxRingBuf,
                     const uint8_t *ptr,
                     uint32_t length);
uint32_t xRingBufPutForce(struct RingBuffer_T *const pxRingBuf,
                          const uint8_t *ptr,
                          uint32_t length);
uint32_t xRingBufGet(struct RingBuffer_T *pxRingBuf,
                     uint8_t *ptr,
                     uint32_t length);
uint32_t rt_ringbuffer_peek(struct RingBuffer_T *rb, uint8_t **ptr);
uint32_t rt_ringbuffer_putchar(struct RingBuffer_T *rb, const uint8_t ch);
uint32_t rt_ringbuffer_putchar_force(struct RingBuffer_T *rb, const uint8_t ch);
uint32_t rt_ringbuffer_getchar(struct RingBuffer_T *rb, uint8_t *ch);

/** return the size of empty space in rb */
#define xGetRingBufSpaceLen(rb) ((rb)->dataBufSize - xGetRingBufDataLen(rb))

#endif
