/*
    8/2021
    Thu vien xu ly Uart
*/
#include "user_message.h"
#include "radio.h"

#include "user_define.h"
#include "user_internal_mem.h"



/*==================Func static ===================*/
static uint8_t _mDATA_TSVH (int Kind_Send);
static uint8_t _mDATA_INTAN(int Kind_Send);
static uint8_t _mDATA_RESP_AT(int Kind_Send);
static uint8_t _mDATA_EVENT(int Kind_Send);


/*======================== Structs var======================*/
const structMessTypeSend     sMessSend[] =
{
    { _MESS_CONNECT,			    NULL,           				{NULL}  },
    
    { _MESS_RESPONSE_AT,	        _mDATA_RESP_AT,				    {NULL}  }, 
    
    { _MESS_TSVH,				    _mDATA_TSVH,				    {NULL}  },
    { _MESS_INTAN,				    _mDATA_INTAN,				    {NULL}  }, 
    
    { _MESS_EVENT,				    _mDATA_EVENT,				    {NULL}  },
    { _MESS_RQ_STIME,				_mDATA_TSVH,				    {NULL}  }, 
      
    { _MESS_END,				    NULL,				            {NULL}  },
};

/* */
const structMessTypeReceive     sMessRecv[] =
{
    {_MESS_END,					NULL,						{NULL}},
};


uint8_t aPAYLOAD[MAX_LENGTH_PAYLOAD];

structMessVariable sMessVar = 
{
    .sPayload = {&aPAYLOAD[0], 0},
};


/*================ Func Callback ===============*/
/*
    Func: Read Packet TSVH from mem:
        + Ghep nhieu ban tin vao 1 packet gui di.
*/

static uint8_t _mDATA_TSVH(int Kind_Send)
{
    uint16_t    i = 0;
    uint8_t     IndexMess = 0;
    uint16_t    IndexRead = 0;
    uint8_t     TempCrC = 0;

    //Ðoc record tu Flash ra-> Dong goi du lieu vao sMessVar.sPayload
    sMessVar.sPayload.Data_a8 = aPAYLOAD;
    sMessVar.sPayload.Length_u16 = 0;

	sRecTSVH.CountMessPacket_u16 = 0;
    while (sRecTSVH.CountMessPacket_u16 < MAX_MESS_IN_PACKET)
    {
        IndexRead = (sRecTSVH.IndexSend_u16 + sRecTSVH.CountMessPacket_u16) % sRecTSVH.MaxRecord_u16;
        if (IndexRead == sRecTSVH.IndexSave_u16)
            break;
    #ifdef MEMORY_ON_FLASH
        if (Flash_Read_Record_Without_Index ((uint32_t)(sRecTSVH.AddStart_u32 + sRecTSVH.SizeRecord_u16 * IndexRead), &sMessVar.sPayload) == 1)
            IndexMess++;
    #else
        if (Flash_Read_Record_Without_Index((uint32_t)(sRecTSVH.AddStart_u32 + sRecTSVH.SizeRecord_u16 * IndexRead), &sMessVar.sPayload) == 1)
            IndexMess++;
    #endif
        sRecTSVH.CountMessPacket_u16++;
        
        break;  //chi dong goi 1 ban tin
    }

    if (IndexMess != 0)  //neu có ban tin duoc dong goi thi moi add crc vao
    {
        //crc tat ca cac byte.
        sMessVar.sPayload.Length_u16++;
        for (i = 0; i < (sMessVar.sPayload.Length_u16 - 1); i++)
            TempCrC ^= *(sMessVar.sPayload.Data_a8 + i);

        *(sMessVar.sPayload.Data_a8 + sMessVar.sPayload.Length_u16 - 1) = TempCrC;
        //Encryption AES128

        return 1;
    }
    //Neu cac ban tin doc ra loi -> Cong index send lên (De bo qua) -> Tiep tuc gưi tiep
    if (sRecTSVH.CountMessPacket_u16 != 0)
    {
        sRecTSVH.IndexSend_u16 = (sRecTSVH.IndexSend_u16 + sRecTSVH.CountMessPacket_u16) % sRecTSVH.MaxRecord_u16;

    #ifdef MEMORY_ON_FLASH
        Flash_Save_Index(sRecTSVH.AddIndexSend_u32, sRecTSVH.IndexSend_u16);
    #else
        ExMem_Save_Index(sRecTSVH.AddIndexSend_u32, sRecTSVH.IndexSend_u16);
    #endif

        sRecTSVH.CountMessPacket_u16 = 0;
    }

    return 0;
}


/*
    Func: Packet Intan
*/
static uint8_t _mDATA_INTAN(int Kind_Send)
{
    //Ðoc record tu Flash ra-> Dong goi du lieu vao sMessVar.sPayload
    sMessVar.sPayload.Data_a8 = aPAYLOAD;
    sMessVar.sPayload.Length_u16 = 0;
        
    #ifdef USING_APP_WM
        sMessVar.sPayload.Length_u16 = Modem_Packet_TSVH (sMessVar.sPayload.Data_a8);
    #endif
    //Encryption AES128 
    
    return 1;
}


static uint8_t _mDATA_EVENT(int Kind_Send)
{
    uint16_t    i = 0;
    uint8_t     IndexMess = 0;
    uint16_t    IndexRead = 0;
    uint8_t     TempCrC = 0;

    //Ðoc record tu Flash ra-> Dong goi du lieu vao sMessVar.sPayload
    sMessVar.sPayload.Data_a8 = aPAYLOAD;
    sMessVar.sPayload.Length_u16 = 0;

	sRecEvent.CountMessPacket_u16 = 0;
    while (sRecEvent.CountMessPacket_u16 < MAX_MESS_IN_PACKET)
    {
        IndexRead = (sRecEvent.IndexSend_u16 + sRecEvent.CountMessPacket_u16) % sRecEvent.MaxRecord_u16;
        if (IndexRead == sRecEvent.IndexSave_u16)
            break;
    #ifdef MEMORY_ON_FLASH
        if (Flash_Read_Record_Without_Index ((uint32_t)(sRecEvent.AddStart_u32 + sRecEvent.SizeRecord_u16 * IndexRead), &sMessVar.sPayload) == 1)
            IndexMess++;
    #else
        if (Flash_Read_Record_Without_Index((uint32_t)(sRecEvent.AddStart_u32 + sRecEvent.SizeRecord_u16 * IndexRead), &sMessVar.sPayload) == 1)
            IndexMess++;
    #endif
        sRecEvent.CountMessPacket_u16++;
        
        break;  //chi dong goi 1 ban tin
    }

    if (IndexMess != 0)  //neu có ban tin duoc dong goi thi moi add crc vao
    {
        //crc tat ca cac byte.
        sMessVar.sPayload.Length_u16++;
        for (i = 0; i < (sMessVar.sPayload.Length_u16 - 1); i++)
            TempCrC ^= *(sMessVar.sPayload.Data_a8 + i);

        *(sMessVar.sPayload.Data_a8 + sMessVar.sPayload.Length_u16 - 1) = TempCrC;
        //Encryption AES128

        return 1;
    }
    //Neu cac ban tin doc ra loi -> Cong index send lên (De bo qua) -> Tiep tuc gưi tiep
    if (sRecEvent.CountMessPacket_u16 != 0)
    {
        sRecEvent.IndexSend_u16 = (sRecEvent.IndexSend_u16 + sRecEvent.CountMessPacket_u16) % sRecEvent.MaxRecord_u16;

    #ifdef MEMORY_ON_FLASH
        Flash_Save_Index(sRecEvent.AddIndexSend_u32, sRecEvent.IndexSend_u16);
    #else
        ExMem_Save_Index(sRecEvent.AddIndexSend_u32, sRecEvent.IndexSend_u16);
    #endif

        sRecEvent.CountMessPacket_u16 = 0;
    }

    return 0;
}


static uint8_t _mDATA_RESP_AT(int Kind_Send)
{
    uint16_t i = 0;
    
    sMessVar.sPayload.Data_a8 = aPAYLOAD;
    sMessVar.sPayload.Length_u16 = 0;

    for (i = 0; i < sModem.strATResp.Length_u16; i++)
      *(sMessVar.sPayload.Data_a8 +  sMessVar.sPayload.Length_u16++) = *(sModem.strATResp.Data_a8 + i);

    //Encryption AES128 
    return 1;
}


/*======================= Function Handler =======================*/
/*
    Func: Check Mess mark
        return: 0: No mess
                1: Check and Read OK
                2: Check fail Format
*/

int8_t mCheck_Mess_Mark (void)
{
    int8_t i = 0;
    
    for (i = _MESS_CONNECT; i < _MESS_END; i++)
    {
        if (sMessVar.aMARK[i] == TRUE)
        {         
            if (sMessSend[i].CallBack != NULL)
            {
                if (sMessSend[i].CallBack(i) == TRUE)
                {
                    sMessVar.aMARK[i] = FALSE;
                    //Send via Lora
                    return i;
                } 
            } else
            {
                sMessVar.aMARK[i] = FALSE;
            }
        }
    }
    
    return _MESS_END;
}



void mCallback_Send_OK (void)
{
    switch (sMessVar.DataType_u8)
    {
        case _MESS_TSVH:
            sRecTSVH.IndexSend_u16 = (sRecTSVH.IndexSend_u16 + sRecTSVH.CountMessPacket_u16) % sRecTSVH.MaxRecord_u16;
            #ifdef MEMORY_ON_FLASH
                Flash_Save_Index(sRecTSVH.AddIndexSend_u32, sRecTSVH.IndexSend_u16);
            #else
                ExMem_Save_Index(sRecTSVH.AddIndexSend_u32, sRecTSVH.IndexSend_u16);
            #endif
            break;
        case _MESS_EVENT:
            sRecEvent.IndexSend_u16 = (sRecEvent.IndexSend_u16 + sRecEvent.CountMessPacket_u16) % sRecEvent.MaxRecord_u16;
            #ifdef MEMORY_ON_FLASH
                Flash_Save_Index(sRecEvent.AddIndexSend_u32, sRecEvent.IndexSend_u16);
            #else
                ExMem_Save_Index(sRecEvent.AddIndexSend_u32, sRecEvent.IndexSend_u16);
            #endif
            break;
        default:
            break;
    } 
}





void mMess_Handle_Reset (uint8_t DataType)
{
    sMessVar.Retry_u8[DataType] = 0;
    sMessVar.DataType_u8 = 0;
}

/*
    Func: handle send fail
        + Increase Index
    return 
        If Retru > MAX:  return 0: to retry
        Not:        return 1: to ignore Mess.

        Pending: retry util send ok 
*/

uint8_t mCb_Send_Fail (void)
{
    if (MAX_RETRY_SEND_LORA == 0xFF)
    {
        sMessVar.Retry_u8[sMessVar.DataType_u8]++;
        if (sMessVar.Retry_u8[sMessVar.DataType_u8] < MAX_RETRY_WITHOUT_SKIP)
        {
            return FALSE;
        } else
        {
            sMessVar.Retry_u8[sMessVar.DataType_u8] = 0;
            return PENDING;
        }
    } else
    {
        sMessVar.Retry_u8[sMessVar.DataType_u8]++;
        
        if (sMessVar.Retry_u8[sMessVar.DataType_u8] >= MAX_RETRY_SEND_LORA)
        {
            return TRUE;
        }
    }
        
    return FALSE;
}




