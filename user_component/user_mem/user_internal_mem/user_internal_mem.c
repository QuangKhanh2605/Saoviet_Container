/*
    8/2021
    Thu vien xu ly Uart
*/
#include "user_internal_mem.h"


static uint8_t _Cb_On_Flash_Write_Buff (uint8_t event);
static uint8_t _Cb_On_Flash_Read_Buff (uint8_t event);
static uint8_t _Cb_On_Flash_Check_Queue (uint8_t event);


/*======== struct ===============*/
sEvent_struct sEventOnChipFlash[] =
{
	{ _EVENT_ON_FLASH_WRITE_BUFF, 	    0, 0, 500,  			_Cb_On_Flash_Write_Buff  },
	{ _EVENT_ON_FLASH_READ_BUFF, 	    0, 0, 500,  			_Cb_On_Flash_Read_Buff   },
    { _EVENT_ON_FLASH_CHECK_QUEUE, 	    0, 0, 10,               _Cb_On_Flash_Check_Queue   },
};


static uint8_t aDATA_READ[256];

static sQueueMemWriteManager     sQOnFlashWrite[5];
static sQueueMemReadManager      sQOnFlashRead[20];

sMemoryManager        sOnFlash = 
{
    .sHRead.aData = {&aDATA_READ[0], 0},
};

Struct_Queue_Type   qOnFlashRead;
Struct_Queue_Type   qOnFlashWrite;



/*======================== Function ======================*/
/*------------ Func Callback --------------*/
static uint8_t _Cb_On_Flash_Write_Buff (uint8_t event)
{
    if (_ON_FLASH_IS_NEW_PAGE (sOnFlash.sHWrite.Addr_u32) == 0)
        Erase_Firmware(sOnFlash.sHWrite.Addr_u32, 1);
      
    OnchipFlash_Write_Buff (sOnFlash.sHWrite.Addr_u32, sOnFlash.sHWrite.aData, sOnFlash.sHWrite.Length_u16); 

    //Write OK: tang index save
    if (sOnFlash.pMem_Write_OK != NULL)
        sOnFlash.pMem_Write_OK(sOnFlash.sHWrite.TypeData_u8);
    
    sOnFlash.sHWrite.Pending_u8 = false;
    qQueue_Receive(&qOnFlashWrite, NULL, 1);   
    
    return 1;
}


static uint8_t _Cb_On_Flash_Read_Buff (uint8_t event)
{
    OnchipFlashReadData(sOnFlash.sHRead.Addr_u32, sOnFlash.sHRead.aData.Data_a8, sOnFlash.sHRead.aData.Length_u16);

    if (sOnFlash.pMem_Read_OK != NULL)
        sOnFlash.pMem_Read_OK(sOnFlash.sHRead.TypeData_u8, sOnFlash.sHRead.Addr_u32, sOnFlash.sHRead.TypeRq_u8);
    
    sOnFlash.sHRead.Pending_u8 = false;
    qQueue_Receive(&qOnFlashRead, NULL, 1);   
    
    return 1;
}




static uint8_t _Cb_On_Flash_Check_Queue (uint8_t event)
{
    sQueueMemWriteManager qFlashWriteTemp;  
    sQueueMemReadManager  qFlashReadTemp; 
        
    //Waiting Init Flash)
    if (sOnFlash.Status_u8 != false)
    {             
        //Check Queue write
        if (sOnFlash.sHWrite.Pending_u8 != true)
        {
            if (qGet_Number_Items (&qOnFlashWrite) != 0)
            {
                UTIL_Printf_Str( DBLEVEL_H, "u_on_flash: have item write to flash\r\n");
                //Get Item
                qQueue_Receive(&qOnFlashWrite, (sQueueMemWriteManager *) &qFlashWriteTemp, 0);
                
                if (sOnFlash.Status_u8 == true)
                {
                    //Copy Data and Push step write
                    sOnFlash.sHWrite.Pending_u8         = true;
                    
                    for (uint16_t  i = 0; i < qFlashWriteTemp.Length_u16; i++)         
                        sOnFlash.sHWrite.aData[i] = qFlashWriteTemp.aData[i];
                            
                    sOnFlash.sHWrite.Length_u16   = qFlashWriteTemp.Length_u16;
                    sOnFlash.sHWrite.TypeData_u8  = qFlashWriteTemp.TypeData_u8;
                    
                    if ( sOnFlash.pMem_Get_Addr_Fr_Type != NULL )
                    {
                        sOnFlash.sHWrite.Addr_u32 = sOnFlash.pMem_Get_Addr_Fr_Type(qFlashWriteTemp.TypeData_u8, __FLASH_WRITE);
                        //Neu la 1 queue ghi vao 1 dia chi cu the: k phai ban tin data: lay dia chi trong queue
                        if (sOnFlash.sHWrite.Addr_u32 == 0xFFFFFFFF)
                        {
                            sOnFlash.sHWrite.Addr_u32 = qFlashWriteTemp.Addr_u32;
                        }
                        
                        fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_WRITE_BUFF);
                    } else
                    {
                        UTIL_Printf_Str( DBLEVEL_M, "u_on_flash: unregisted cb get addr\r\n");
                    }
                } 
            }
        }
        
        //Check Queue Read
        if (sOnFlash.sHRead.Pending_u8 != true)
        {
            if (qGet_Number_Items (&qOnFlashRead) != 0)
            {   
                UTIL_Printf_Str( DBLEVEL_H, "u_on_flash: have item read to flash\r\n");
                
                qQueue_Receive(&qOnFlashRead, (sQueueMemReadManager *) &qFlashReadTemp, 0);
                
                if (sOnFlash.Status_u8 == true)
                {
                    UTIL_MEM_set(aDATA_READ, 0, sizeof (aDATA_READ) );
                    
                    sOnFlash.sHRead.Pending_u8         = true;
                    sOnFlash.sHRead.aData.Data_a8      = aDATA_READ;
                    sOnFlash.sHRead.aData.Length_u16   = qFlashReadTemp.Length_u8;
                    
                    sOnFlash.sHRead.TypeData_u8        = qFlashReadTemp.TypeData_u8;
                    sOnFlash.sHRead.TypeRq_u8          = qFlashReadTemp.TypeRq_u8;
                    
                    if ( sOnFlash.pMem_Get_Addr_Fr_Type != NULL )
                    {
                        sOnFlash.sHRead.Addr_u32 = sOnFlash.pMem_Get_Addr_Fr_Type(qFlashReadTemp.TypeData_u8, __FLASH_READ);
                        //Neu la 1 queue ghi vao 1 dia chi cu the: k phai ban tin data: lay dia chi trong queue
                        if (sOnFlash.sHRead.Addr_u32 == 0xFFFFFFFF)
                        {
                            sOnFlash.sHRead.Addr_u32 = qFlashReadTemp.Addr_u32;
                        }
                    } else
                    {
                        UTIL_Printf_Str( DBLEVEL_M, "u_on_flash: unregisted cb get addr\r\n");
                        sOnFlash.sHRead.Addr_u32 = qFlashReadTemp.Addr_u32;
                    }
                    
                    fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_READ_BUFF);
                } 
            }
        }
    }
    
    if ( (qGet_Number_Items (&qOnFlashWrite) != 0) || (qGet_Number_Items (&qOnFlashRead) != 0) )
        fevent_enable( sEventOnChipFlash, event);
    
    return 1;
}








/*------------- Func Handle----------------*/

void OnFlash_Init (void)
{
    //Init Queue Sim Step
    qQueue_Create (&qOnFlashWrite, 5, sizeof (sQueueMemWriteManager), (sQueueMemWriteManager *) &sQOnFlashWrite);  
    qQueue_Create (&qOnFlashRead, 20, sizeof (sQueueMemReadManager), (sQueueMemReadManager *) &sQOnFlashRead);    
}



uint8_t OnFlash_Task (void)
{
    uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_ON_FLASH_END; i++)
	{
		if (sEventOnChipFlash[i].e_status == 1)
		{
            Result = true;
            
			if ((sEventOnChipFlash[i].e_systick == 0) ||
					((HAL_GetTick() - sEventOnChipFlash[i].e_systick)  >=  sEventOnChipFlash[i].e_period))
			{
                sEventOnChipFlash[i].e_status = 0; 
				sEventOnChipFlash[i].e_systick = HAL_GetTick();
				sEventOnChipFlash[i].e_function_handler(i);
			}
		}
	}

	return Result;
}

/*
    Func: Read last Record: TSVH, Event...
        + Return LastPulse, sTime
*/


uint8_t Flash_Read_Last_Record (sRecordMemoryManager sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime)
{
    
    return 0;
}


/*============= Func Init ========================*/
/*
    Func: Init Infor from Flash
*/
void Flash_Get_Infor (uint32_t AddStart, uint8_t *pData, uint16_t *Length, uint8_t MAX_LEGNTH_INFOR)
{
    uint8_t LenTemp = 0;
    
    LenTemp = *(__IO uint8_t*) (AddStart);

    if (LenTemp > MAX_LEGNTH_INFOR)
        LenTemp = MAX_LEGNTH_INFOR;
    //clear buff
    UTIL_MEM_set (pData, 0, MAX_LEGNTH_INFOR);
    //Read flash
    OnchipFlashReadData ((AddStart + 1), pData, LenTemp);   
    *Length = LenTemp;
}



/*
    Func: check status memory
*/
uint8_t OnFlash_Status (void)
{
    return sOnFlash.Status_u8;
}


void OnFlash_Setup_Init (void)
{
    sOnFlash.Status_u8 = false;
}


uint8_t OnFlash_Test_Write (void)
{
    return true;
}


/*
    Func: check queue empty
*/
uint8_t OnFlash_Is_Queue_Read_Empty (void)
{
    if (qGet_Number_Items (&qOnFlashRead) == 0)
        return true;
            
    return false;
}


uint8_t OnFlash_Is_Queue_Write_Empty (void)
{
    if (qGet_Number_Items (&qOnFlashWrite) == 0)
        return true;
            
    return false;
}



void OnFlash_Send_To_Queue_Read (sQueueMemReadManager *qRead)
{
    qQueue_Send(&qOnFlashRead, (sQueueMemReadManager *) qRead, _TYPE_SEND_TO_END); 
            
    fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_CHECK_QUEUE);
}


void OnFlash_Send_To_Queue_Write (sQueueMemWriteManager *qWrite)
{
    qQueue_Send(&qOnFlashWrite, (sQueueMemWriteManager *) qWrite, _TYPE_SEND_TO_END);
    
    fevent_active( sEventOnChipFlash, _EVENT_ON_FLASH_CHECK_QUEUE);
}


