
#include "user_log.h"

#include "user_modem.h"

#include "queue_p.h"

/*==================var struct======================*/
uint8_t aLOG_EEPROM[1280];    //20*64 byte = 1280 byte
uint8_t aLOG_RECORD[512];

Struct_Queue_Type   qModemEEPROM;

sQueryOldRecord    sOldRec =
{
    .strMess = {&aLOG_RECORD[0], 0},
};


/*==================Function======================*/
void LOG_Init_Queue (void)
{
    qQueue_Create (&qModemEEPROM, 20, 64, (uint8_t *) &aLOG_EEPROM);
}

void LOG_Send_Queue (sData *str)
{
    uint8_t aTEMP[64] = {0};
    uint8_t i = 0;

    if (str->Length_u16 >= 62)
        str->Length_u16 = 62;

    aTEMP[0] = 0xAA;
    aTEMP[1] = str->Length_u16;

    for (i = 0; i < str->Length_u16; i++)
        aTEMP[i + 2] = *(str->Data_a8 + i);

    qQueue_Send (&qModemEEPROM, (uint8_t *) &aTEMP[0], _TYPE_SEND_TO_END);
    //Active event Handler Save Log

}

void LOG_Recei_Queue (uint8_t *pData)
{
    qQueue_Receive (&qModemEEPROM, (uint8_t *) pData, 1);
}

/*
    Func: Luu vao flash noi
*/
uint8_t LOG_Save_Mess (uint8_t *pData, uint16_t Length)
{
    uint8_t aTEMP[64] = {0};
    uint8_t i = 0, Count = 0, TempCrc = 0;
    uint8_t aTEMP_NUM[10] = {0};
    sData  strNum = {&aTEMP_NUM[0], 0};

    if (Length >= 40)
        Length = 40;

    //Dong goi theo 1.ST.Log
    Convert_Uint64_To_StringDec(&strNum, sRecLog.IndexSave_u16, 0);
    for (i = 0; i < strNum.Length_u16; i++)
        aTEMP[Count++] = *(strNum.Data_a8 + i);

    aTEMP[Count++] = '.';
    aTEMP[Count++] = sRTC.year/10 + 0x30;
    aTEMP[Count++] = sRTC.year%10 + 0x30;
    aTEMP[Count++] = '/';
    aTEMP[Count++] = sRTC.month/10 + 0x30;
    aTEMP[Count++] = sRTC.month%10 + 0x30;
    aTEMP[Count++] = '/';
    aTEMP[Count++] = sRTC.date/10 + 0x30;
    aTEMP[Count++] = sRTC.date%10 + 0x30;
    aTEMP[Count++] = ' ';
    aTEMP[Count++] = sRTC.hour/10 + 0x30;
    aTEMP[Count++] = sRTC.hour%10 + 0x30;
    aTEMP[Count++] = ':';
    aTEMP[Count++] = sRTC.min/10 + 0x30;
    aTEMP[Count++] = sRTC.min%10 + 0x30;
    aTEMP[Count++] = ':';
    aTEMP[Count++] = sRTC.sec/10 + 0x30;
    aTEMP[Count++] = sRTC.sec%10 + 0x30;

    aTEMP[Count++] = '.';   //9 byte: 64 - (3 + 12 + 3)- 2 = 44

    for (i = 0; i < Length; i++)
        aTEMP[Count++] = *(pData + i);
    
    Count++;
	for (i = 0; i < (Count - 1); i++)
		TempCrc ^= aTEMP[i];

    aTEMP[Count-1] = TempCrc;

#ifdef MEMORY_ON_FLASH
    return Flash_Save_Record (&sRecLog, &aTEMP[0], Count);
#else
    return ExMem_Save_Record(&sRecLog, &aTEMP[0], Count);
#endif
}

/*
    Func: Read log mess from Flash
        + Kind: Read last index
                Read Last for index
        + Port: Server, serial
    Return:
            0: FALSE -> End Read
            1: TRUE
*/
uint8_t LOG_Read_Old_Record ( sQueryOldRecord *sReqRead )
{
    uint16_t IndexRead = 0;
    uint8_t ReadStatus = 0;
    StructManageRecordFlash *sRec = NULL;

    if (sReqRead->Type_u8 == _RQ_RECORD_TSVH)
        sRec = &sRecTSVH;
    else if (sReqRead->Type_u8 == _RQ_RECORD_EVENT)
        sRec = &sRecEvent;
    else
        sRec = &sRecLog;
    //Reset Buff Read again
    sReqRead->strMess.Length_u16 = 0;
    //Doc ra.
    switch (sReqRead->Kind_u8)
    {
        case 0:    //Read Log hien tai
            if (sRec->IndexSave_u16 == 0)
                IndexRead = sRec->MaxRecord_u16 - 1;
            else
                IndexRead = sRec->IndexSave_u16 - 1;
            //Doc Packet ra tu vi tri hien tai
            ReadStatus = LOG_Read_Data_Index (sRec, IndexRead, &sReqRead->strMess);
            break;
        case 1:     //Doc tu vi tri Start to End
            IndexRead = (sReqRead->StartIndex_u8 + sReqRead->Count_u8) % sRec->MaxRecord_u16;
            ReadStatus = LOG_Read_Data_Index(sRec, IndexRead, &sReqRead->strMess);
            sReqRead->Count_u8 ++;
            break;
        default:
            break;
    }

    //If Read OK -> Respond to Port
    if (ReadStatus == TRUE)
    {
    #ifdef USING_APP_SIM
        if ((sReqRead->Port_u8 == _AT_REQUEST_SERVER) && ((sModem.ModeSimPower_u8 == _SIM_MODE_SAVE_POWER) || (sSimVar.ConnSerStatus_u8 == FALSE)))
            return 0;
    #endif

        if (sReqRead->Type_u8 != _RQ_RECORD_LOG)    
            DCU_Respond(sReqRead->Port_u8, sReqRead->strMess.Data_a8, sReqRead->strMess.Length_u16, 0);
        else
            DCU_Respond(sReqRead->Port_u8, sReqRead->strMess.Data_a8, (sReqRead->strMess.Length_u16 - 1), 0);  //Bo byte crc

        if ((sReqRead->Kind_u8 == 0) || (IndexRead == sReqRead->EndIndex_u8))  //Ket thuc
            return 0;

        return 1;
    }
    //If Read Fail -> continue Read if case: start to end index
    if (sReqRead->Kind_u8 != 0)
    {
        if (IndexRead != sReqRead->EndIndex_u8)   //tiep tuc doc record tiep theo
            return 1;
    }

    return 0;
}


/*
    Func: Read Old Rec at index
*/

uint8_t LOG_Read_Data_Index (StructManageRecordFlash *sRec, uint16_t IndexRead, sData *pData)
{
#ifdef MEMORY_ON_FLASH
    return Flash_Read_Record_Without_Index (sRec->AddStart_u32 + IndexRead * sRec->SizeRecord_u16, pData);
#else
    return ExMem_Read_Record_Without_Index(sRec->AddStart_u32 + IndexRead * sRec->SizeRecord_u16, pData);
#endif
}











