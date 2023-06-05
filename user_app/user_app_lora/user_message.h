/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include "user_util.h"


/*======================== Define ======================*/
#define MAX_LENGTH_PAYLOAD      256
/*======================== Structs var======================*/
typedef enum {
    _MESS_CONNECT,
    
    _MESS_RESPONSE_AT,
    
	_MESS_TSVH,
    _MESS_INTAN,
    _MESS_EVENT,

	_MESS_RQ_STIME,   

	_MESS_END,
} eKindMessLora;


typedef uint8_t (*pFunCallBackSend)(int Kind_Send);
typedef struct {
	int 				idMark;
	pFunCallBackSend 	CallBack;
	sData		        sKind;
} structMessTypeSend;


//
typedef void (*pFunCallBackRecv)(sData *str_Receiv, int16_t Pos);
typedef struct {
	int 			    idMark;
	pFunCallBackRecv	CallBack;
	sData	            sKind;
} structMessTypeReceive;

typedef struct
{
    uint8_t aMARK[32];
        
    sData   sPayload;
    int8_t  DataType_u8;
    uint8_t Retry_u8[10];
    uint8_t MessType_u8[10];
}structMessVariable;





typedef enum eDataDeviceType
{
    _DATA_NONE      = 0x00,
    _DATA_TSVH      = 0x01,
    _DATA_INTAN     = 0x02,
    _DATA_EVENT     = 0x03,
    _DATA_RESPONSE  = 0x04,
    
    _DATA_CONFIG    = 0x05,
    _DATA_AT_CMD    = 0x06,
}DataDeviceype_t;



typedef struct
{
    uint8_t TypeResp_u8;
    
}StructMasterTx;


/*======================== External Var struct ======================*/

extern const structMessTypeSend sMessSend[];
extern const structMessTypeReceive sMessRecv[];

extern structMessVariable sMessVar;

extern uint8_t aPAYLOAD[MAX_LENGTH_PAYLOAD];


/*======================== Function ======================*/

/*---------------Function Callback send-----------------------*/
uint8_t     _mCB_MESS_CONNECT(int Kind_Send);
uint8_t 	_mCB_MESS_TSVH(int Kind_Send);

int8_t      mCheck_Mess_Mark (void);
void        mCallback_Send_OK (void);

void        mMess_Handle_Reset (uint8_t DataType);
uint8_t     mCb_Send_Fail (void);


#endif










