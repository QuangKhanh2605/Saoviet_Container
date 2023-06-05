/*
 * myUart.h
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#ifndef USER_UART_H_
#define USER_UART_H_

#include "user_util.h"
#include "usart.h"

/*======Define Uart ==============*/
#define uart_debug	    huart3
#define uart_sim	    huart2

/*===========Var struct=============*/
extern uint8_t UartDebugBuff[1200];
extern sData sUartDebug;

/*==================Function==================*/
void Init_Uart_Module (void);




#endif /* USER_UART_H_ */
