#ifndef __EXTERN_H__
#define __EXTERN_H__

#include "stdint.h"
#include "stdbool.h"


extern volatile uint8_t tim16_Delay_flag;
extern volatile uint8_t tim16_1s_flag;
extern volatile uint8_t tim16_100ms_flag;
extern volatile uint8_t tim16_10ms_flag;
extern volatile uint8_t tim16_200ms_flag;
extern volatile uint8_t tim16_20s_flag;
extern volatile uint8_t tim16_delay_flag;
extern volatile uint8_t tim16_delay_flag1;

extern volatile uint8_t receive_data_time;

extern volatile uint8_t IfScene;
extern volatile uint8_t IfAddress;

extern uint8_t u8RxData[1];
extern volatile uint8_t bodyflag2;
extern uint8_t My_Send_Data[20];

extern uint16_t My_Delay_Time;
extern uint8_t MyAddressH;
extern uint8_t MyAddressL;

extern uint8_t DialNum[1];
extern uint8_t DevAddress[1];
extern uint8_t OCAddr[2];
#define OpenState 0x31    //红外有人
#define CloseState 0x32   //红外无人
#define SubAddress1   0x61
#define SubAddress2   0x62

extern uint8_t NetTodoFlag;
#define Net_Addr    0x90
#define Net_In      0x91
#define Net_Out     0x99
#define ClearAll    0x95

extern uint8_t ringbuf[60];
extern uint8_t readcnt ;
extern uint8_t translate_flage;
extern uint8_t sumtranslate_flage;
extern uint8_t state;

extern uint8_t recvbufOK;
extern uint8_t  RxData[41];

extern uint16_t LuxToUse;
extern uint32_t LuxFromBH;

#define   OUTPUT_CONTROL                            0X01                         //输出控制
#define   OUTPUT_STATE_CHECK                        0X02                         //设备输出状态查询
#define   SIMPLE_OUTPUT_STATE_FEEDBECK              0X03                         //单点输出状态反馈
#define   HOLISTIC_OUTPUT_STATE_FEEDBECK            0X04                         //整体输出状态反馈
#define   SENCE_CONFIG                              0X09                         //情景配置
#define   SENCE_CONFIG_FEEDBACK                     0X0A                         //情景配置反馈
#define   SENCE_CONFIG_DELETE                       0X0B                         //情景配置删除
#define   SENCE_CONFIG_DELETE_FEEDBECK              0X0C                         //情景配置删除反馈
#define   SENCE_CONTROL                             0X0D                         //情景控制
#define   CONTACTOR_CONFIG                          0X11                         //触点配置
#define   CONTACTOR_CONFIG_FEEDBECK                 0X12                         //触点配置反馈
#define   JOIN_NETWORK                              0X19                         //入网
#define   JOIN_NETWORK_FEEDBECK                     0X1A                         //入网反馈
#define   EXIT_NETWORK                              0X1B                         //退网
#define   EXIT_NETWORK_FEEDBECK                     0X1C                         //退网反馈
#define   CHECK_NETWORK_STATE                       0X1D                         //查询网络状态
#define   FEEDBECK_NETWORK_STATE                    0X1E                         //反馈网络状态
#define   SIMPLE_LED_CONTROL                        0X22                         //单个LED控制
#define   HOLISTIC_LED_CONTROL                      0X23                         //整体LED控制
#define   IR_RESET                                  0X24                         //红外初始化
#define   VERSION_REPORT                            0X26                         //版本上报
#define   VERSION_CHECK                             0X27                         //版本查询

#define   Write_Address                             0x86                         //写地址
#define   Write_Lux                                 0x88                         //写照度

#endif
