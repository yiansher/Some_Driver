#ifndef _DEAL_H
#define _DEAL_H

#include "stm32g0xx_hal.h"

void ClearAll_Deal(void);
void NetIN_Feedback_Deal(void);
void NetOUT_Feedback_Deal(void);

void Net_deal(void);
void NetInDeal(void);
void NetOutDeal(void);
void sence_config_deal(void);
void sence_config_delect(void) ;
void Contact_Input_Config(void);
void BodySence_deal(void);
void Write_Address_deal(void);
void Write_Lux_deal(void);


#endif



