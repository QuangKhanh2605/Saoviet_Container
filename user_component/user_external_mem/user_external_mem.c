


#include "user_external_mem.h"


/*======================= Function =======================*/

void ExMem_Save_Index (uint32_t Add, uint16_t Value)
{
    uint8_t aTemp[4] = {0};
    
    aTemp[0] = 0xAA;
    aTemp[1] = 0x02;
    aTemp[2] = (uint8_t) (Value >> 8);
    aTemp[3] = (uint8_t) Value;
       
    CAT24Mxx_Write_Array (Add, aTemp, 4);
}

/*
    Func: Init Index Record From ExMem
        + Index Send, Save
*/

void ExMem_Init_Record_Index (StructManageRecordFlash *sRecord)
{
    uint8_t Buff_temp[20] = {0};
    
     //Doc vi tri send va luu ra
    UTIL_MEM_set(Buff_temp, CAT_BYTE_EMPTY, sizeof(Buff_temp));
    //Read Cat24
    CAT24Mxx_Read_Array(sRecord->AddIndexSend_u32, Buff_temp, 4);
    
    if ((Buff_temp[0] != CAT_BYTE_EMPTY) && (Buff_temp[1] != CAT_BYTE_EMPTY))
	{
        sRecord->IndexSend_u16 = (Buff_temp[2] << 8) | Buff_temp[3];
        //Clear Buff and Read Data IndexSave
        UTIL_MEM_set(Buff_temp, CAT_BYTE_EMPTY, sizeof(Buff_temp));
        CAT24Mxx_Read_Array(sRecord->AddIndexSave_u32, Buff_temp, 4);
        
        sRecord->IndexSave_u16 = (Buff_temp[2] << 8) | Buff_temp[3];
        //kiem tra dieu kien gioi han InDex
        if (sRecord->IndexSend_u16 >= sRecord->MaxRecord_u16)
        	sRecord->IndexSend_u16 = 0;
        
        if (sRecord->IndexSave_u16 >= sRecord->MaxRecord_u16)
        	sRecord->IndexSave_u16 = 0;
    } else
    {
        ExMem_Save_Index(sRecord->AddIndexSend_u32, sRecord->IndexSend_u16);
    	ExMem_Save_Index(sRecord->AddIndexSave_u32, sRecord->IndexSave_u16);
    }
}



/*
    Func: Read Last Record From exmem
        + Cut Pulse Value and stime
*/


uint8_t ExMem_Read_Last_Record (StructManageRecordFlash sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime)
{
    uint32_t        Pulse = 0;
    uint8_t         aTEMP[CAT_MAX_SIZE_RECORD] = {0};
    uint16_t		IndexRead = 0;
    
    if (sRecord.IndexSave_u16 < 1)                                //Vi tri save la 0 thi doc truoc do 1 index moi co data
        IndexRead = sRecord.IndexSave_u16 + sRecord.MaxRecord_u16 - 1;
    else
        IndexRead = sRecord.IndexSave_u16 - 1;
    
    //Doc Packet from Index
    if (CAT24Mxx_Read_Array(sRecord.AddStart_u32 + (IndexRead * 64), &aTEMP[0], CAT_MAX_SIZE_RECORD) != 1)
    {
        UTIL_Printf ( (uint8_t *) "=Read EEPROM Fail 2!=\r\n", 23 ); 
        return 0;
    }
    //Check if ((aTEMP[0] != FLASH_BYTE_EMPTY) && (aTEMP[1] != FLASH_BYTE_EMPTY))
    if ((aTEMP[0] == 0xAA) && (aTEMP[1] != CAT_BYTE_EMPTY))
    {
        //Thoi gian tu byte 0 den byte 7 bao gom ca obis va length
        LastSTime->year  = aTEMP[4];
        LastSTime->month = aTEMP[5];
        LastSTime->date  = aTEMP[6];
        LastSTime->hour  = aTEMP[7];
        LastSTime->min   = aTEMP[8];
        LastSTime->sec   = aTEMP[9];
        
        //Pulse tu byte thu 8 den 13 ke ca obis va length
        Pulse = ((aTEMP[12] << 24) | (aTEMP[13] << 16) |(aTEMP[14] << 8) | aTEMP[15]);  
        
        *LastPulse = Pulse;
        return 1;
    }   
        
    return 0;
}


/*
    Func: Save Record to ExMem
*/

uint8_t ExMem_Save_Record (StructManageRecordFlash *sRecord, uint8_t *pData, uint8_t Length)
{	
    uint8_t     aMessData[CAT_MAX_SIZE_RECORD] = {0};
    uint8_t     i = 0;
    uint8_t     NumWrite = 0;
    uint8_t     LenWrite = 0;
    uint8_t     PosWrite = 0;
    
    aMessData[0] = 0xAA;
    aMessData[1] = Length;
    
    for (i = 0; i < Length; i++)
        aMessData[i + 2] = *(pData + i);
    
    LenWrite = Length + 2;
    
    while (LenWrite > I2C_CAT24Mxx_MAX_BUFF)
    {
        PosWrite = NumWrite * I2C_CAT24Mxx_MAX_BUFF;
        if (CAT24Mxx_Write_Array (sRecord->AddStart_u32 + sRecord->IndexSave_u16 * sRecord->SizeRecord_u16 + PosWrite, &aMessData[PosWrite], I2C_CAT24Mxx_MAX_BUFF) == 0) 
            return 0;
        
        NumWrite++;
        LenWrite -= I2C_CAT24Mxx_MAX_BUFF;
    }
    
    //Luu vao Flash
    if (LenWrite != 0)
        if (CAT24Mxx_Write_Array (sRecord->AddStart_u32 + sRecord->IndexSave_u16 * sRecord->SizeRecord_u16 + PosWrite, &aMessData[PosWrite], LenWrite) == 0)  
            return 0;
   
    //kiem tra xem ban ghi moi nay da vuot qua max chua.
    sRecord->IndexSave_u16++;
    sRecord->IndexSave_u16 = sRecord->IndexSave_u16 % sRecord->MaxRecord_u16;
    //luu lai vi tri Save
    ExMem_Save_Index(sRecord->AddIndexSave_u32, sRecord->IndexSave_u16);    
    //kiem tra xem ban ghi moi nay da vuot qua Indexsend. thi cung day 2 th len 1
    if (sRecord->IndexSave_u16 == sRecord->IndexSend_u16)
    {
        sRecord->IndexSend_u16 = (sRecord->IndexSend_u16 + 1) % sRecord->MaxRecord_u16;
        //luu IndexSend vao Flash
        ExMem_Save_Index(sRecord->AddIndexSend_u32, sRecord->IndexSend_u16);  
    }
    
    return 1;
}


/*
    Func: Read packet from flash
    Input: Addr
           *str: save data
           IndexMess in many packet: 1, 2,3...helpfull decode in server
    
*/
uint8_t ExMem_Read_Record (uint32_t andress, sData *str, uint8_t IndexMess)
{
    uint8_t     i = 0, Length = 0;
    uint8_t		ChecksumByte=0;
    uint8_t     FirstByte = 0;
    uint8_t     aTEMP_DATA[CAT_MAX_SIZE_RECORD] = {0};
    
    //Doc Packet from Index
    if (CAT24Mxx_Read_Array(andress, &aTEMP_DATA[0], CAT_MAX_SIZE_RECORD) != 1)
    {
        UTIL_Printf( (uint8_t *) "=Read EEPROM Fail 2!=\r\n", 23 ); 
        return 0;
    }
         
    FirstByte = aTEMP_DATA[0];
    Length = aTEMP_DATA[1]; 
    
    if ( (Length >= (CAT_MAX_SIZE_RECORD - 2)) || (FirstByte != 0xAA) )
        return 0;
    //check crc
	for(i = 0; i < (Length - 1); i++)
		ChecksumByte ^= aTEMP_DATA[i + 2];
    
    if (ChecksumByte == aTEMP_DATA[Length + 1])
    {
        *(str->Data_a8 + str->Length_u16++) = IndexMess;  
        *(str->Data_a8 + str->Length_u16++) = Length;
        
        for(i = 0; i < Length; i++)
            *(str->Data_a8 + str->Length_u16++) = aTEMP_DATA[i + 2];
        
        return 1;
    }
    
    return 0;
}



/*
    Func: Read packet from flash
    Input: Addr
           *str: save data
           IndexMess in many packet: 1, 2,3...helpfull decode in server
    
*/
uint8_t ExMem_Read_Record_Without_Index (uint32_t andress, sData *str)
{
    uint8_t     i = 0, Length = 0;
    uint8_t		ChecksumByte=0;
    uint8_t     FirstByte = 0;
    uint8_t     aTEMP_DATA[CAT_MAX_SIZE_RECORD] = {0};
    
    //Doc Packet from Index
    if (CAT24Mxx_Read_Array(andress, &aTEMP_DATA[0], CAT_MAX_SIZE_RECORD) != 1)
    {
        UTIL_Printf( (uint8_t *) "=Read EEPROM Fail 2!=\r\n", 23 ); 
        return 0;
    }
         
    FirstByte = aTEMP_DATA[0];
    Length = aTEMP_DATA[1]; 
    
    if ((Length >= (CAT_MAX_SIZE_RECORD - 2)) || (FirstByte != 0xAA))
        return 0;
    //check crc
	for(i = 0; i < (Length - 1); i++)
		ChecksumByte ^= aTEMP_DATA[i + 2];
    
    if (ChecksumByte == aTEMP_DATA[Length + 1])
    {        
        for(i = 0; i < Length; i++)
            *(str->Data_a8 + str->Length_u16++) = aTEMP_DATA[i + 2];
        
        return 1;
    }
    
    return 0;
}


/*
    Func: Init Infor from Flash
*/
void Exmem_Get_Infor (uint8_t *pSource, uint8_t *pData, uint16_t *Length, uint8_t MAX_LEGNTH_INFOR)
{
    uint8_t LenTemp = 0;
    uint8_t i = 0;
    
    LenTemp = *pSource;

    if (LenTemp > MAX_LEGNTH_INFOR)
        LenTemp = MAX_LEGNTH_INFOR;
    //clear buff
    UTIL_MEM_set (pData, 0, MAX_LEGNTH_INFOR);
    //Read flash
    for (i = 0; i < LenTemp; i++)
         pData[i] = pSource[i + 1];
  
    *Length = LenTemp;
}



void Exmem_Read_Array (uint32_t Addr, uint8_t *pData, uint16_t length)
{
    CAT24Mxx_Read_Array(Addr, pData, length);
}













