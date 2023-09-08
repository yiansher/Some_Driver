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
#define OpenState 0x31    //��������
#define CloseState 0x32   //��������
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

#define   OUTPUT_CONTROL                            0X01                         //�������
#define   OUTPUT_STATE_CHECK                        0X02                         //�豸���״̬��ѯ
#define   SIMPLE_OUTPUT_STATE_FEEDBECK              0X03                         //�������״̬����
#define   HOLISTIC_OUTPUT_STATE_FEEDBECK            0X04                         //�������״̬����
#define   SENCE_CONFIG                              0X09                         //�龰����
#define   SENCE_CONFIG_FEEDBACK                     0X0A                         //�龰���÷���
#define   SENCE_CONFIG_DELETE                       0X0B                         //�龰����ɾ��
#define   SENCE_CONFIG_DELETE_FEEDBECK              0X0C                         //�龰����ɾ������
#define   SENCE_CONTROL                             0X0D                         //�龰����
#define   CONTACTOR_CONFIG                          0X11                         //��������
#define   CONTACTOR_CONFIG_FEEDBECK                 0X12                         //�������÷���
#define   JOIN_NETWORK                              0X19                         //����
#define   JOIN_NETWORK_FEEDBECK                     0X1A                         //��������
#define   EXIT_NETWORK                              0X1B                         //����
#define   EXIT_NETWORK_FEEDBECK                     0X1C                         //��������
#define   CHECK_NETWORK_STATE                       0X1D                         //��ѯ����״̬
#define   FEEDBECK_NETWORK_STATE                    0X1E                         //��������״̬
#define   SIMPLE_LED_CONTROL                        0X22                         //����LED����
#define   HOLISTIC_LED_CONTROL                      0X23                         //����LED����
#define   IR_RESET                                  0X24                         //�����ʼ��
#define   VERSION_REPORT                            0X26                         //�汾�ϱ�
#define   VERSION_CHECK                             0X27                         //�汾��ѯ

#define   Write_Address                             0x86                         //д��ַ
#define   Write_Lux                                 0x88                         //д�ն�

#endif
