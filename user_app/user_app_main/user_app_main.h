/*
 * user_app.h
 *
 *  Created on: Dec 14, 2021
 *      Author: Chien
 */

#ifndef INC_USER_APP_MAIN_H_
#define INC_USER_APP_MAIN_H_


#include "user_util.h"

#include "event_driven.h"

typedef enum
{
	_EVENT_TIMMER_IRQ = 0,       //0
	_EVENT_PROCESS_UART_DEBUG,   //1
    _EVENT_SET_RTC,              //2
	_EVENT_IDLE,                 //3
    _EVENT_TX_TIMER,             //4
    
    _EVENT_READ_OLD_RECORD,      //5
    _EVENT_SAVE_BOX,
    
	_EVENT_END_MAIN,
}eKindEventAppMain;


extern sData   sFirmVersion;
extern sEvent_struct sEventAppMain[];


/*=============Function=======================*/
void        SysApp_Init (void);
void        SysApp_Setting (void);

uint8_t     AppCom_Task (void);
void        AppCom_Init (void);

void        AppCom_IRQ_Timer_CallBack (void);

void        Main_Task (void);
void        AppCom_Set_Next_TxTimer (void);
void        qQueue_Init (void);

#endif /* INC_USER_APP_H_ */
