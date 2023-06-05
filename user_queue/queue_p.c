/*
 * queue_test.c
 *
 *  Created on: Feb 14, 2022
 *      Author: chien
 */


#include "queue_p.h"
#include <stdio.h>
#include <string.h>


static void qCopy_Data_To_Queue(Struct_Queue_Type * sQueue, void *qItem, uint8_t HeadOrEnd);
static void qCopy_Data_From_Queue(Struct_Queue_Type *sQueue, void *aBuffer, uint8_t Type);

/*====================Function =================*/
/*
    - Func: Create Queue
    - Output:
            Khoi tao cac bien 
            Khoi tao con tro tro toi mang struct. aBuffQueue
*/
void qQueue_Create (Struct_Queue_Type *qQueue, uint8_t Length, uint16_t Size, void *aBuffQueue)
{
	qQueue->status = 0;
	qQueue->Head_u8 = aBuffQueue;
	qQueue->Tail_u8 = qQueue->Head_u8 + Length * Size;

	qQueue->pcReadFrom = qQueue->Head_u8;
	qQueue->pcWriteTo = qQueue->Head_u8;

	qQueue->NumWaiting_u8 = 0;
	qQueue->LenQueue_u8 = Length;
	qQueue->SizeItem_u8 = Size;
}

/*
    Clear all item in queue
*/

void qQueue_Clear (Struct_Queue_Type *qQueue)
{
    qQueue->pcReadFrom = qQueue->Head_u8;
	qQueue->pcWriteTo = qQueue->Head_u8;
	qQueue->NumWaiting_u8 = 0;
}

/*
    Func: Send Item to Queue
    Input:
            qQueue: Queue chua item
            Item: uint8_t or typedef struct. Ep kieu
            HeadOrEnd: Push Head hoac End
    Output:
            TRUE: Push OK
            FALSE: Queue overwrite
*/
uint8_t qQueue_Send (Struct_Queue_Type *qQueue, void *Item, uint8_t HeadOrEnd)
{
    uint8_t Result = 0;
    
    if (qQueue->NumWaiting_u8 < (qQueue->LenQueue_u8 - 1))   //Chua tran buff queue
    {
        qCopy_Data_To_Queue(qQueue, Item, HeadOrEnd);
        Result = 1;
    } else
    {
        //OverWrite
        Result = 0;
    }
    
    return Result;
}

/*
    Func:  Get Item from queue
    Output:
        TRUE: Get OK
        FALSE: Fail
*/

uint8_t qQueue_Receive (Struct_Queue_Type *qQueue, void *Item, uint8_t Type)
{
    if (qQueue->NumWaiting_u8 > 0)
    {
        qCopy_Data_From_Queue(qQueue, Item, Type);
        return 1;
    }
    
    return 0;
}

/*
    Func: Copy Item to queue
    Input: 
            +Item
            + Type: Head or End
    Output: 
            TRUE
            FALSE
*/


static void qCopy_Data_To_Queue(Struct_Queue_Type * sQueue, void *qItem, uint8_t HeadOrEnd)
{
	if (sQueue->SizeItem_u8 == 0)
	{
		//Size Zero
	} else if(HeadOrEnd == _TYPE_SEND_TO_END)
	{
		(void) memcpy((void *) sQueue->pcWriteTo, qItem, sQueue->SizeItem_u8); 
        
		sQueue->pcWriteTo += sQueue->SizeItem_u8; 
		if(sQueue->pcWriteTo >= sQueue->Tail_u8) 
			sQueue->pcWriteTo = sQueue->Head_u8;
	} else
	{
        sQueue->pcReadFrom -= sQueue->SizeItem_u8;
		(void) memcpy((void *) sQueue->pcReadFrom, qItem, sQueue->SizeItem_u8);
		
		if (sQueue->pcReadFrom < sQueue->Head_u8) 
			sQueue->pcReadFrom = (sQueue->Tail_u8 - sQueue->SizeItem_u8);
	}
    //Increase Numwating excute
	sQueue->NumWaiting_u8 += 1;
}

/*
    Func:  Copy Item from Queue
    Type: 0: Không next item;  1: next Item
    Output: Item
*/
static void qCopy_Data_From_Queue(Struct_Queue_Type *sQueue, void *aBuffer, uint8_t Type)
{
    if (Type == 0)
        (void) memcpy((void *) aBuffer, (void *) sQueue->pcReadFrom, sQueue->SizeItem_u8 ); 
    else
    {        
        if (sQueue->SizeItem_u8 != 0)
        {
            if (aBuffer != NULL)
                (void) memcpy((void *) aBuffer, (void *) sQueue->pcReadFrom, sQueue->SizeItem_u8 ); 
            
            sQueue->pcReadFrom += sQueue->SizeItem_u8;
            
            if (sQueue->pcReadFrom >= sQueue->Tail_u8) 
                sQueue->pcReadFrom = sQueue->Head_u8;
        }
        //decrease Numwating excute
        sQueue->NumWaiting_u8 -= 1;
    }
}


uint8_t qGet_Number_Items(Struct_Queue_Type *sQueue)
{
	return sQueue->NumWaiting_u8;
}

uint8_t qQueue_Back_Item (Struct_Queue_Type *qQueue, uint16_t NumItem)
{
    uint16_t i = 0;
    
    //If NumItem back and Numwaiting > Max: -> False
    if ((NumItem + qQueue->NumWaiting_u8) >= (qQueue->LenQueue_u8 - 1))
        return 0;
        
    for (i = 0; i < NumItem; i++)
    {
        qQueue->pcReadFrom -= qQueue->SizeItem_u8;
		
		if (qQueue->pcReadFrom < qQueue->Head_u8) 
			qQueue->pcReadFrom = (qQueue->Tail_u8 - qQueue->SizeItem_u8);
        
        qQueue->NumWaiting_u8++;
    }
    
    return 1;
}



