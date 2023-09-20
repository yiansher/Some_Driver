#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__

#include <stdint.h>

void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void *_pCallBack);

#endif

