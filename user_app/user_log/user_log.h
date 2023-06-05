
#ifndef USER_LOG_H_
#define USER_LOG_H_

#include "user_util.h"

#include "user_internal_mem.h"
#include "user_external_mem.h"

/*===========Var struct=============*/
typedef struct
{
    uint8_t     Status_u8;   //0 False 1: True : 2 Pending

    uint8_t     Type_u8;     //Loai Record: 1: TSVH, 2: Event; 3: LOG
    uint8_t     Kind_u8;     //Kieu request: 0: Rec gan nhat, 1: n Rec gan nhat
    uint8_t     Port_u8;     //1: Uart  3:server
    sData       strMess;     //str Data

    uint8_t     StartIndex_u8;
    uint8_t     EndIndex_u8;
    uint8_t     Count_u8;
}sQueryOldRecord;

typedef enum
{
    _RQ_RECORD_TSVH,
    _RQ_RECORD_EVENT,
    _RQ_RECORD_LOG,
}sTypeRequestReadOldRecord;

/*===========Extern Var struct=============*/
extern sQueryOldRecord    sOldRec;


/*==================Function==================*/
void    LOG_Init_Queue (void);
void    LOG_Send_Queue (sData *str);
void    LOG_Recei_Queue (uint8_t *pData);

uint8_t LOG_Save_Mess (uint8_t *pData, uint16_t Length);

uint8_t LOG_Read_Old_Record ( sQueryOldRecord *sReqRead );
uint8_t LOG_Read_Data_Index ( StructManageRecordFlash *sRec, uint16_t IndexRead, sData *pData);






#endif /* USER_UART_H_ */
