


#include "user_external_eeprom.h"



static uint8_t _Cb_Ex_EEPROM_Write_Buff (uint8_t event);
static uint8_t _Cb_Ex_EEPROM_Read_Buff (uint8_t event);
static uint8_t _Cb_Ex_EEPROM_Check_Queue (uint8_t event);


/*======== struct ===============*/
sEvent_struct sEventExEEPROM[] =
{
	{ _EVENT_EX_EEPROM_WRITE_BUFF, 	    0, 0, 200,  			_Cb_Ex_EEPROM_Write_Buff  },
	{ _EVENT_EX_EEPROM_READ_BUFF, 	    0, 0, 200,  			_Cb_Ex_EEPROM_Read_Buff   },
    { _EVENT_EX_EEPROM_CHECK_QUEUE, 	0, 0, 10,               _Cb_Ex_EEPROM_Check_Queue },
};


static uint8_t aDATA_EEPROM_READ[256];

static sQueueMemWriteManager     sQExEEPROMWrite[5];
static sQueueMemReadManager      sQExEEPROMRead[20];

sMemoryManager        sExEEPROM = 
{
    .sHRead.aData = {&aDATA_EEPROM_READ[0], 0},
};

Struct_Queue_Type   qExEEPROMRead;
Struct_Queue_Type   qExEEPROMWrite;


/*======================= Function =======================*/


/*------------ Func Callback --------------*/
static uint8_t _Cb_Ex_EEPROM_Write_Buff (uint8_t event)
{
    uint16_t LenWrite = sExEEPROM.sHWrite.Length_u16;
    uint8_t  NumWrite = 0;
    uint32_t PosWrite = 0;
    static uint8_t CountRetry = 0;
    
    while (LenWrite > I2C_CAT24Mxx_MAX_BUFF)
    {
        PosWrite = NumWrite * I2C_CAT24Mxx_MAX_BUFF;
        if (CAT24Mxx_Write_Array (sExEEPROM.sHWrite.Addr_u32 + PosWrite, &sExEEPROM.sHWrite.aData[PosWrite], I2C_CAT24Mxx_MAX_BUFF) == 0) 
        {
            //Set EEPROM Loi and check again
            sExEEPROM.sHWrite.Pending_u8 = false;
            UTIL_Printf_Str( DBLEVEL_M, "u_ex_eeprom: write eeprom fail!\r\n" ); 
            
            CountRetry++;
            if (CountRetry >= EEPROM_MAX_RETRY_WRITE)
            {
                CountRetry = 0;
                qQueue_Receive(&qExEEPROMWrite, NULL, 1);   
                sExEEPROM.pMem_Forward_Data_To_Sim(sExEEPROM.sHWrite.TypeData_u8, &sExEEPROM.sHWrite.aData[0], 
                                                   sExEEPROM.sHWrite.Length_u16);
            }
            
            return 0;
        }
        
        NumWrite++;
        LenWrite -= I2C_CAT24Mxx_MAX_BUFF;
    }
    
    //Luu vao Flash
    if (LenWrite != 0)
    {
        if (CAT24Mxx_Write_Array (sExEEPROM.sHWrite.Addr_u32 + PosWrite,  &sExEEPROM.sHWrite.aData[PosWrite], LenWrite) == 0)  
        {
            //Set EEPROM Loi and check again
            sExEEPROM.sHWrite.Pending_u8 = false;
            UTIL_Printf_Str( DBLEVEL_M, "u_ex_eeprom: write eeprom fail!\r\n" ); 
            //Bo qua item neu qua retry
            CountRetry++;
            if (CountRetry >= EEPROM_MAX_RETRY_WRITE)
            {
                CountRetry = 0;
                qQueue_Receive(&qExEEPROMWrite, NULL, 1);
                sExEEPROM.pMem_Forward_Data_To_Sim(sExEEPROM.sHWrite.TypeData_u8, &sExEEPROM.sHWrite.aData[0], 
                                                   sExEEPROM.sHWrite.Length_u16);
            }
            
            return 0;
        }
    }
    
    CountRetry = 0;
    //Write OK: tang index save
    if (sExEEPROM.pMem_Write_OK != NULL)
        sExEEPROM.pMem_Write_OK(sExEEPROM.sHWrite.TypeData_u8);
    
    sExEEPROM.sHWrite.Pending_u8 = false;
    qQueue_Receive(&qExEEPROMWrite, NULL, 1);   
    
    return 1;
}

static uint8_t _Cb_Ex_EEPROM_Read_Buff (uint8_t event)
{
    static uint8_t RetryRead = 0;
    //Doc Packet from Index
    if (CAT24Mxx_Read_Array(sExEEPROM.sHRead.Addr_u32, sExEEPROM.sHRead.aData.Data_a8, sExEEPROM.sHRead.aData.Length_u16) != 1)
    {
        //Set EEPROM Loi and check again
        sExEEPROM.sHRead.Pending_u8 = false;
        UTIL_Printf_Str( DBLEVEL_M, "u_ex_eeprom: read eeprom fail!\r\n" );  
        
        RetryRead++;
        if (RetryRead >= EEPROM_MAX_RETRY_WRITE)
        {
            RetryRead = 0;
            qQueue_Receive(&qExEEPROMRead, NULL, 1);  
        }
        
        return 0;
    }
    
    RetryRead = 0;
    
    if (sExEEPROM.pMem_Read_OK != NULL)
        sExEEPROM.pMem_Read_OK(sExEEPROM.sHRead.TypeData_u8, sExEEPROM.sHRead.Addr_u32, sExEEPROM.sHRead.TypeRq_u8);
    
    sExEEPROM.sHRead.Pending_u8 = false;
    qQueue_Receive(&qExEEPROMRead, NULL, 1);  
    
    return 1;
}


static uint8_t _Cb_Ex_EEPROM_Check_Queue (uint8_t event)
{
    sQueueMemWriteManager qFlashWriteTemp;  
    sQueueMemReadManager  qFlashReadTemp; 
        
    //Waiting Init Flash)
    if (sExEEPROM.Status_u8 != false)
    {             
        //Check Queue write
        if (sExEEPROM.sHWrite.Pending_u8 != true)
        {
            if (qGet_Number_Items (&qExEEPROMWrite) != 0)
            {
                UTIL_Printf_Str( DBLEVEL_H, "u_ex_eeprom: have item write to flash\r\n");
                //Get Item
                qQueue_Receive(&qExEEPROMWrite, (sQueueMemWriteManager *) &qFlashWriteTemp, 0);
                
                if (sExEEPROM.Status_u8 == true)
                {
                    //Copy Data and Push step write
                    sExEEPROM.sHWrite.Pending_u8         = true;
                    
                    for (uint16_t  i = 0; i < qFlashWriteTemp.Length_u16; i++)         
                        sExEEPROM.sHWrite.aData[i] = qFlashWriteTemp.aData[i];
                            
                    sExEEPROM.sHWrite.Length_u16   = qFlashWriteTemp.Length_u16;
                    sExEEPROM.sHWrite.TypeData_u8  = qFlashWriteTemp.TypeData_u8;
                    
                    if ( sExEEPROM.pMem_Get_Addr_Fr_Type != NULL )
                    {
                        sExEEPROM.sHWrite.Addr_u32 = sExEEPROM.pMem_Get_Addr_Fr_Type(qFlashWriteTemp.TypeData_u8, __FLASH_WRITE);
                        //Neu la 1 queue ghi vao 1 dia chi cu the: k phai ban tin data: lay dia chi trong queue
                        if (sExEEPROM.sHWrite.Addr_u32 == 0xFFFFFFFF)
                        {
                            sExEEPROM.sHWrite.Addr_u32 = qFlashWriteTemp.Addr_u32;
                        }
                        
                        fevent_active( sEventExEEPROM, _EVENT_EX_EEPROM_WRITE_BUFF);
                    } else
                    {
                        UTIL_Printf_Str( DBLEVEL_M, "u_ex_eeprom: unregisted cb get addr\r\n");
                    }
                } 
            }
        }
        
        //Check Queue Read
        if (sExEEPROM.sHRead.Pending_u8 != true)
        {
            if (qGet_Number_Items (&qExEEPROMRead) != 0)
            {   
                UTIL_Printf_Str( DBLEVEL_H, "u_ex_eeprom: have item read to flash\r\n");
                
                qQueue_Receive(&qExEEPROMRead, (sQueueMemReadManager *) &qFlashReadTemp, 0);
                
                if (sExEEPROM.Status_u8 == true)
                {
                    UTIL_MEM_set(aDATA_EEPROM_READ, 0, sizeof (aDATA_EEPROM_READ) );
                    
                    sExEEPROM.sHRead.Pending_u8         = true;
                    sExEEPROM.sHRead.aData.Data_a8      = aDATA_EEPROM_READ;
                    sExEEPROM.sHRead.aData.Length_u16   = qFlashReadTemp.Length_u8;
                    
                    sExEEPROM.sHRead.TypeData_u8        = qFlashReadTemp.TypeData_u8;
                    sExEEPROM.sHRead.TypeRq_u8          = qFlashReadTemp.TypeRq_u8;
                    
                    if ( sExEEPROM.pMem_Get_Addr_Fr_Type != NULL )
                    {
                        sExEEPROM.sHRead.Addr_u32 = sExEEPROM.pMem_Get_Addr_Fr_Type(qFlashReadTemp.TypeData_u8, __FLASH_READ);
                        //Neu la 1 queue ghi vao 1 dia chi cu the: k phai ban tin data: lay dia chi trong queue
                        if (sExEEPROM.sHRead.Addr_u32 == 0xFFFFFFFF)
                        {
                            sExEEPROM.sHRead.Addr_u32 = qFlashReadTemp.Addr_u32;
                        }
                    } else
                    {
                        UTIL_Printf_Str( DBLEVEL_M, "u_ex_eeprom: unregisted cb get addr\r\n");
                        sExEEPROM.sHRead.Addr_u32 = qFlashReadTemp.Addr_u32;
                    }
                    
                    fevent_active( sEventExEEPROM, _EVENT_EX_EEPROM_READ_BUFF);
                } 
            }
        }
    }
            
    if ( (qGet_Number_Items (&qExEEPROMWrite) != 0) || (qGet_Number_Items (&qExEEPROMRead) != 0) )
        fevent_enable( sEventExEEPROM, event);
    
    return 1;
}




/*------------ Func Handle ------------*/

void ExEEPROM_Init (void)
{
   //Init Queue Sim Step
    qQueue_Create (&qExEEPROMWrite, 5, sizeof (sQueueMemWriteManager), (sQueueMemWriteManager *) &sQExEEPROMWrite);  
    qQueue_Create (&qExEEPROMRead, 20, sizeof (sQueueMemReadManager), (sQueueMemReadManager *) &sQExEEPROMRead);    
}



uint8_t ExEEPROM_Task (void)
{
    uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_EX_EEPROM_END; i++)
	{
		if (sEventExEEPROM[i].e_status == 1)
		{
            Result = true;
            
			if ((sEventExEEPROM[i].e_systick == 0) ||
					((HAL_GetTick() - sEventExEEPROM[i].e_systick)  >=  sEventExEEPROM[i].e_period))
			{
                sEventExEEPROM[i].e_status = 0; 
				sEventExEEPROM[i].e_systick = HAL_GetTick();
				sEventExEEPROM[i].e_function_handler(i);
			}
		}
	}

	return Result;
}



/*
    Func: Read Last Record From exmem
        + Cut Pulse Value and stime
*/
uint8_t ExEEPROM_Read_Last_Record (sRecordMemoryManager sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime)
{
//    //Check if ((aTEMP[0] != FLASH_BYTE_EMPTY) && (aTEMP[1] != FLASH_BYTE_EMPTY))
//    if ((aTEMP[0] == 0xAA) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        //Thoi gian tu byte 0 den byte 7 bao gom ca obis va length
//        LastSTime->year  = aTEMP[4];
//        LastSTime->month = aTEMP[5];
//        LastSTime->date  = aTEMP[6];
//        LastSTime->hour  = aTEMP[7];
//        LastSTime->min   = aTEMP[8];
//        LastSTime->sec   = aTEMP[9];
//        
//        //Pulse tu byte thu 8 den 13 ke ca obis va length
//        Pulse = ((aTEMP[12] << 24) | (aTEMP[13] << 16) |(aTEMP[14] << 8) | aTEMP[15]);  
//        
//        *LastPulse = Pulse;
//        return 1;
//    }   
        
    return 0;
}




/*
    Func: check status memory
*/
uint8_t ExEEPROM_Status (void)
{
    return sExEEPROM.Status_u8;
}

void ExEEPROM_Setup_Init (void)
{
    sExEEPROM.Status_u8 = false;
}

uint8_t ExEEPROM_Test_Write (void)
{
    return true;
}



/*
    Func: check queue empty
*/
uint8_t ExEEPROM_Is_Queue_Read_Empty (void)
{
    if (qGet_Number_Items (&qExEEPROMRead) == 0)
        return true;
            
    return false;
}


/*
    Func: check queue empty
*/
uint8_t ExEEPROM_Is_Queue_Write_Empty (void)
{
    if (qGet_Number_Items (&qExEEPROMWrite) == 0)
        return true;
            
    return false;
}


void ExEEPROM_Send_To_Queue_Read (sQueueMemReadManager *qRead)
{
    qQueue_Send(&qExEEPROMRead, (sQueueMemReadManager *) qRead, _TYPE_SEND_TO_END); 
            
    fevent_active( sEventExEEPROM, _EVENT_EX_EEPROM_CHECK_QUEUE);
}


void ExEEPROM_Send_To_Queue_Write (sQueueMemWriteManager *qWrite)
{
    qQueue_Send(&qExEEPROMWrite, (sQueueMemWriteManager *) qWrite, _TYPE_SEND_TO_END); 
            
    fevent_active( sEventExEEPROM, _EVENT_EX_EEPROM_CHECK_QUEUE);
}

 





