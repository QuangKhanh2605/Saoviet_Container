/*
 * myUart.c
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#include "user_uart.h"
#include "user_define.h"

/*==================var struct======================*/
uint8_t UartDebugBuff[1200] = {0};
sData 	sUartDebug = {&UartDebugBuff[0], 0};

uint8_t UartSimRxByte = 0;
uint8_t UartDebugRxByte = 0;
uint8_t Uart485RxByte = 0;


/*==================Function======================*/

void Init_Uart_Sim_Rx_IT (void)
{
//    __HAL_UART_ENABLE_IT(&uart_sim, UART_IT_RXNE);
    
    HAL_UART_Receive_IT(&uart_sim, &UartSimRxByte, 1);
}

void Init_Uart_Debug_Rx_IT (void)
{
//    __HAL_UART_ENABLE_IT(&uart_debug, UART_IT_RXNE);
    
    HAL_UART_Receive_IT(&uart_debug, &UartDebugRxByte, 1);
}


void Init_Uart_485_Rx_IT (void)
{
//    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    
    HAL_UART_Receive_IT(&huart1, &Uart485RxByte, 1);
}


void DeInit_Uart_Sim_Rx_IT (void)
{
    //__HAL_UART_DISABLE_IT(&uart_sim, UART_IT_RXNE);   
}


void Init_Uart_Module (void)
{   
    Init_Uart_Sim_Rx_IT();
    Init_Uart_Debug_Rx_IT();
    Init_Uart_485_Rx_IT();
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == uart_sim.Instance)
	{
        *(sUartSim.Data_a8 + sUartSim.Length_u16++) = UartSimRxByte;

        if (sUartSim.Length_u16 >= (sizeof(uartSimBuffReceive) - 1))
            sUartSim.Length_u16 = 0;

        HAL_UART_Receive_IT(&uart_sim, &UartSimRxByte, 1);
        
        fevent_enable(sEventSim, _EVENT_SIM_UART_RECEIVE);        
	} 

	if (huart->Instance == uart_debug.Instance)
	{
        *(sUartDebug.Data_a8 + sUartDebug.Length_u16++) = UartDebugRxByte;

        if (sUartDebug.Length_u16 > (sizeof (UartDebugBuff) - 1))
            sUartDebug.Length_u16 = 0;

        HAL_UART_Receive_IT(&uart_debug, &UartDebugRxByte, 1);
        
        fevent_enable(sEventAppComm, _EVENT_PROCESS_UART_DEBUG);
	}
    
    if (huart->Instance == huart1.Instance)
	{
        *(sUart485.Data_a8 + sUart485.Length_u16++) = Uart485RxByte;

        if (sUart485.Length_u16 >= (sizeof(aUART_485_DATA) - 1))
            sUart485.Length_u16 = 0;
        
        HAL_UART_Receive_IT(&huart1, &Uart485RxByte, 1);
        
       sTempHumi.ModBusStatus_u8 = TRUE;
    }
}






