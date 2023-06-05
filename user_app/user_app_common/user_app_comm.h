/*
 * user_app.h
 *
 *  Created on: Dec 14, 2021
 *      Author: Chien
 */

#ifndef INC_USER_APP_COMM_H_
#define INC_USER_APP_COMM_H_


#include "user_util.h"

#include "event_driven.h"

typedef enum
{
	_EVENT_TIMMER_IRQ = 0,       //0
	_EVENT_PROCESS_UART_DEBUG,   //1
    _EVENT_SET_RTC,              //2
	_EVENT_IDLE,                 //3
    _EVENT_TX_TIMER,             //4
    
    _EVENT_SAVE_BOX,             //6
    
	_EVENT_END_COMM,             //7
}eKindEventAppCommon;


extern sData   sFirmVersion;
extern sEvent_struct sEventAppComm[];


/*=============Function=======================*/
void        SysApp_Init (void);
void        SysApp_Setting (void);
void        Main_Task (void);

uint8_t     AppComm_Task (void);
void        AppComm_Init (void);

void        AppComm_IRQ_Timer_CallBack (void);
uint32_t    AppComm_Get_Random_Follow_ID (void);

void        AppComm_Set_Next_TxTimer (void);







#endif /* INC_USER_APP_H_ */
