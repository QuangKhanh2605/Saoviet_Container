/*
 * myUart.c
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#include "user_uart.h"


/*==================var struct======================*/
uint8_t UartDebugBuff[1200] = {0};
sData 	sUartDebug = {&UartDebugBuff[0], 0};

/*==================Function======================*/
void Init_Uart_Module (void)
{   
    __HAL_UART_ENABLE_IT(&uart_sim, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&uart_debug, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}











