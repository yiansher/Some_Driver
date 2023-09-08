#include "bsp_uart.h"

#define RINGBUF_NUM_MAX 3
static struct RingBuffer_T RingBuffer_t[RINGBUF_NUM_MAX];

void vRingBufInit(void)
{
    for (uint32_t i = 0; i < RINGBUF_NUM_MAX; i++)
    {
        RingBuffer_t[i].dataBuf = NULL;
        RingBuffer_t[i].status = RINGBUF_FREE;
        RingBuffer_t[i].dataBufSize = 0;
        RingBuffer_t[i].read_mirror = RingBuffer_t[i].read_index = 0;
        RingBuffer_t[i].write_mirror = RingBuffer_t[i].write_index = 0;
    }
}

/**
 * @brief Initialize the ring buffer object.
 *
 * @param pxNewRingBuf      A pointer to the ring buffer object.
 * @param pool              A pointer to the buffer.
 * @param size              The size of the buffer in bytes.
 */
uint8_t xRingBufRegister(struct RingBuffer_T *pxNewRingBuf, uint8_t *const iDataBuf, const uint32_t iDataBufSize)
{
    uint8_t i = 0;
    if (pxNewRingBuf == NULL || iDataBuf == NULL || iDataBufSize <= 0)
        return RINGBUF_RET_ERR_PARAMETER;

    if (pxNewRingBuf->status != RINGBUF_INUSE)
    {
        pxNewRingBuf->dataBuf = iDataBuf;
        pxNewRingBuf->status = RINGBUF_INUSE;
        pxNewRingBuf->dataBufSize = iDataBufSize;
        pxNewRingBuf->read_mirror = pxNewRingBuf->read_index = 0;
        pxNewRingBuf->write_mirror = pxNewRingBuf->write_index = 0;
        return RINGBUF_RET_SUCCESS;
    }

    return RINGBUF_RET_REGISTER_ABORT;
}

uint8_t xRingBufFree(struct RingBuffer_T *const pxRingBuf)
{
    if (pxRingBuf == NULL)
        return RINGBUF_RET_ERR_PARAMETER;
    pxRingBuf->dataBuf = NULL;
    pxRingBuf->status = RINGBUF_FREE;
    pxRingBuf->dataBufSize = 0;
    pxRingBuf->read_mirror = pxRingBuf->read_index = 0;
    pxRingBuf->write_mirror = pxRingBuf->write_index = 0;
    return RINGBUF_RET_SUCCESS;
}

inline uint8_t xCheckRingBufIsFull(struct RingBuffer_T *const pxRingBuf)
{
    if (pxRingBuf->read_index == pxRingBuf->write_index)
    {
        if (pxRingBuf->read_mirror == pxRingBuf->write_mirror)
            return RINGBUF_RET_BUF_EMPTY;
        else
            return RINGBUF_RET_BUF_FULL;
    }
    return RINGBUF_RET_BUF_NOT_FULL;
}

uint32_t xGetRingBufDataLen(struct RingBuffer_T *const pxRingBuf)
{
    switch (xCheckRingBufIsFull(pxRingBuf))
    {
    case RINGBUF_RET_BUF_EMPTY:
        return 0;
    case RINGBUF_RET_BUF_FULL:
        return pxRingBuf->dataBufSize;
    case RINGBUF_RET_BUF_NOT_FULL:
    default:
    {
        uint32_t wi = pxRingBuf->write_index, ri = pxRingBuf->read_index;
        if (wi > ri)
            return wi - ri;
        else
            return pxRingBuf->dataBufSize - (ri - wi);
    }
    }
}

/**
 * @brief Put a block of data into the ring buffer. If the capacity of ring buffer is insufficient,
 * it will discard out-of-range data.
 *
 * @param pxRingBuf            A pointer to the ring buffer object.
 * @param ptr           A pointer to the data buffer.
 * @param length        The size of data in bytes.
 *
 * @return Return the data size we put into the ring buffer.
 */
uint32_t xRingBufPut(struct RingBuffer_T *const pxRingBuf,
                     const uint8_t *ptr,
                     uint32_t length)
{
    MY_ASSERT(pxRingBuf);
    uint32_t size;
    /* whether has enough space */
    size = xGetRingBufSpaceLen(pxRingBuf);

    /* no space */
    if (size == 0)
        return RINGBUF_RET_BUF_FULL;

    /* drop some data */
    if (size < length)
        length = size;

    /* enough space to put */
    if (pxRingBuf->dataBufSize - pxRingBuf->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&pxRingBuf->dataBuf[pxRingBuf->write_index], ptr, length);
        /** this should not cause overflow because there is enough space for
         * length of data in current mirror **/
        pxRingBuf->write_index += length;
        return length;
    }

    memcpy(&pxRingBuf->dataBuf[pxRingBuf->write_index],
           &ptr[0],
           pxRingBuf->dataBufSize - pxRingBuf->write_index);
    memcpy(&pxRingBuf->dataBuf[0],
           &ptr[pxRingBuf->dataBufSize - pxRingBuf->write_index],
           length - (pxRingBuf->dataBufSize - pxRingBuf->write_index));

    /* we are going into the other side of the mirror */
    pxRingBuf->write_mirror = ~pxRingBuf->write_mirror;
    pxRingBuf->write_index = length - (pxRingBuf->dataBufSize - pxRingBuf->write_index);

    return length;
}

/**
 * @brief Put a block of data into the ring buffer. If the capacity of ring buffer is insufficient,
 * it will overwrite the existing data in the ring buffer.
 *
 * @param rb            A pointer to the ring buffer object.
 * @param ptr           A pointer to the data buffer.
 * @param length        The size of data in bytes.
 *
 * @return Return the data size we put into the ring buffer.
 */
uint32_t xRingBufPutForce(struct RingBuffer_T *const pxRingBuf,
                          const uint8_t *ptr,
                          uint32_t length)
{
    uint32_t space_length;

    MY_ASSERT(pxRingBuf != NULL);

    space_length = xGetRingBufSpaceLen(pxRingBuf);

    if (length > pxRingBuf->dataBufSize)
    {
        ptr = &ptr[length - pxRingBuf->dataBufSize];
        length = pxRingBuf->dataBufSize;
    }

    /* enough space */
    if (pxRingBuf->dataBufSize - pxRingBuf->write_index > length)
    {
        memcpy(&pxRingBuf->dataBuf[pxRingBuf->write_index], ptr, length);
        pxRingBuf->write_index += length;
        if (length > space_length)
            pxRingBuf->read_index = pxRingBuf->write_index;

        return length;
    }

    memcpy(&pxRingBuf->dataBuf[pxRingBuf->write_index],
           &ptr[0],
           pxRingBuf->dataBufSize - pxRingBuf->write_index);
    memcpy(&pxRingBuf->dataBuf[0],
           &ptr[pxRingBuf->dataBufSize - pxRingBuf->write_index],
           length - (pxRingBuf->dataBufSize - pxRingBuf->write_index));

    /* we are going into the other side of the mirror */
    pxRingBuf->write_mirror = ~pxRingBuf->write_mirror;
    pxRingBuf->write_index = length - (pxRingBuf->dataBufSize - pxRingBuf->write_index);

    if (length > space_length)
    {
        if (pxRingBuf->write_index <= pxRingBuf->read_index)
            pxRingBuf->read_mirror = ~pxRingBuf->read_mirror;
        pxRingBuf->read_index = pxRingBuf->write_index;
    }

    return length;
}

/**
 * @brief Get data from the ring buffer.
 *
 * @param rb            A pointer to the ring buffer.
 * @param ptr           A pointer to the data buffer.
 * @param length        The size of the data we want to read from the ring buffer.
 *
 * @return Return the data size we read from the ring buffer.
 */
uint32_t xRingBufGet(struct RingBuffer_T *pxRingBuf,
                     uint8_t *ptr,
                     uint32_t length)
{
    uint32_t size;

    RT_ASSERT(pxRingBuf != NULL);

    /* whether has enough data  */
    size = xGetRingBufDataLen(pxRingBuf);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (pxRingBuf->dataBufSize - pxRingBuf->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &pxRingBuf->dataBuf[pxRingBuf->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        pxRingBuf->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &pxRingBuf->dataBuf[pxRingBuf->read_index],
           pxRingBuf->dataBufSize - pxRingBuf->read_index);
    memcpy(&ptr[pxRingBuf->dataBufSize - pxRingBuf->read_index],
           &pxRingBuf->dataBuf[0],
           length - (pxRingBuf->dataBufSize - pxRingBuf->read_index));

    /* we are going into the other side of the mirror */
    pxRingBuf->read_mirror = ~pxRingBuf->read_mirror;
    pxRingBuf->read_index = length - (pxRingBuf->dataBufSize - pxRingBuf->read_index);

    return length;
}

/**
 * @brief Get the first readable byte of the ring buffer.
 *
 * @param rb        A pointer to the ringbuffer.
 * @param ptr       When this function return, *ptr is a pointer to the first readable byte of the ring buffer.
 *
 * @note It is recommended to read only one byte, otherwise it may cause buffer overflow.
 *
 * @return Return the size of the ring buffer.
 */
uint32_t rt_ringbuffer_peek(struct RingBuffer_T *rb, uint8_t **ptr)
{
    uint32_t size;

    MY_ASSERT(rb != NULL);

    *ptr = NULL;

    /* whether has enough data  */
    size = xGetRingBufDataLen(rb);

    /* no data */
    if (size == 0)
        return 0;

    *ptr = &rb->dataBuf[rb->read_index];

    if ((uint32_t)(rb->dataBufSize - rb->read_index) > size)
    {
        rb->read_index += size;
        return size;
    }

    size = rb->dataBufSize - rb->read_index;

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = 0;

    return size;
}

/**
 * @brief Put a byte into the ring buffer. If ring buffer is full, this operation will fail.
 *
 * @param rb        A pointer to the ring buffer object.
 * @param ch        A byte put into the ring buffer.
 *
 * @return Return the data size we put into the ring buffer. The ring buffer is full if returns 0. Otherwise, it will return 1.
 */
uint32_t rt_ringbuffer_putchar(struct RingBuffer_T *rb, const uint8_t ch)
{
    RT_ASSERT(rb != NULL);

    /* whether has enough space */
    if (!rt_ringbuffer_space_len(rb))
        return 0;

    rb->dataBuf[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->dataBufSize - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * @brief Put a byte into the ring buffer. If ring buffer is full, it will discard an old data and put into a new data.
 *
 * @param rb        A pointer to the ring buffer object.
 * @param ch        A byte put into the ring buffer.
 *
 * @return Return the data size we put into the ring buffer. Always return 1.
 */
uint32_t rt_ringbuffer_putchar_force(struct RingBuffer_T *rb, const uint8_t ch)
{
    uint8_t old_state;

    RT_ASSERT(rb != NULL);

    old_state = xCheckRingBufIsFull(rb);

    rb->dataBuf[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->dataBufSize - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == RINGBUF_RET_BUF_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == RINGBUF_RET_BUF_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

/**
 * @brief Get a byte from the ring buffer.
 *
 * @param rb        The pointer to the ring buffer object.
 * @param ch        A pointer to the buffer, used to store one byte.
 *
 * @return 0    The ring buffer is empty.
 * @return 1    Success
 */
uint32_t rt_ringbuffer_getchar(struct RingBuffer_T *rb, uint8_t *ch)
{
    RT_ASSERT(rb != NULL);

    /* ringbuffer is empty */
    if (!xGetRingBufDataLen(rb))
        return 0;

    /* put byte */
    *ch = rb->dataBuf[rb->read_index];

    if (rb->read_index == rb->dataBufSize - 1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}
