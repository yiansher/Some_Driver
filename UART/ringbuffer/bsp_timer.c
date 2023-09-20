#include "bsp_timer.h"

void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;

    cnt_now = TIMx->CNT; //TODO, get TIMx CNT reg
    cnt_tar = cnt_now + _uiTimeOut;

    s_TIM_CallBack1 = (void(*) void)_pCallBack;
    TIMx->CCR1 = cnt_tar; 			    /* 设置捕获比较计数器CC1 */
    TIMx->SR = (uint16_t)~TIM_IT_CC1;   /* 清除CC1中断标志 */
	TIMx->DIER |= TIM_IT_CC1;			/* 使能CC1中断 */
}
