# My_Driver

> use stm32f103RET6, 72M, 64k ram, 512k flash

## UART

1. use rt_thread's rt_buffer_t, create a ring buffer, tried the transmit and receive test, and success, but i wasn't content with that, the way it deal data block is still do in each interrupt, so this is not taking advantage of the ring buffer, the ideal approach would be an idle interrupt, receive a whole block, then deal it.

## SPI

1. w25qxx write and read
1. SD and microSD write/read test, noted that microSD should be low speed when initialize, change spi speed.

## GPIO

1. KEY: MultiButton的作者是0x1abin, github地址: https://github.com/0x1abin/MultiButton. 

## I2C

1. lux_sensor,include BH1730, LTR303

