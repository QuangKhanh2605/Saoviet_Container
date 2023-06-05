/*
 * queue_test.h
 *
 *  Created on: Feb 14, 2022
 *      Author: chien
 */

#ifndef QUEUE_QUEUE_H_
#define QUEUE_QUEUE_H_

#include <stdint.h>
#include "user_util.h"


/*=============== Var struct ================*/
typedef struct
{
	uint8_t 	status;   			 // 0 - ready, 1 - pending
	uint8_t		*Head_u8;            // Start addr queue
	uint8_t		*Tail_u8;            // Overflow addr queue

	uint8_t		*pcWriteTo;          //Vi tri ghi tiep theo
	uint8_t		*pcReadFrom;		 //Vi tri doc ra

	uint8_t 	NumWaiting_u8;       //So phan tu dang cho xu ly
	uint8_t 	LenQueue_u8;         //Tong so Max phan tu trong queue
	uint16_t 	SizeItem_u8;         //Size 1 item cua queue
} Struct_Queue_Type;

//Struct Queue Message
typedef struct
{
	uint8_t 	Status_u8;
	uint8_t 	Type_u8;
	sData	    strData;
	uint8_t 	Retry_u8;
}Struct_Queue_Message;


typedef enum
{
    _TYPE_SEND_TO_END,
    _TYPE_SEND_TO_HEAD,
    _TYPE_SEND_END,
}Type_Send_Queue;

/*==============Function ======================*/
void        qQueue_Create (Struct_Queue_Type *qQueue, uint8_t Length, uint16_t Size, void *aBuffQueue);
uint8_t     qQueue_Send (Struct_Queue_Type *qQueue, void *Item, uint8_t HeadOrEnd);
uint8_t     qQueue_Receive (Struct_Queue_Type *qQueue, void *Item, uint8_t Type);
uint8_t     qGet_Number_Items(Struct_Queue_Type *sQueue);
void        qQueue_Clear (Struct_Queue_Type *qQueue);
uint8_t     qQueue_Back_Item (Struct_Queue_Type *qQueue, uint16_t NumItem);

#endif /* QUEUE_QUEUE_TEST_H_ */
