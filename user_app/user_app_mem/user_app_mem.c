

#include "user_app_mem.h"
#include "user_define.h"


/*
    - Quan ly Index: 
        + Luu Part A, Part B
        + Luu index vao flash noi        
*/
static uint8_t _Cb_mem_Power_On(uint8_t event);
static uint8_t _Cb_mem_Test_Write(uint8_t event);
static uint8_t _Cb_mem_Init_Pos_Rec(uint8_t event);
static uint8_t _Cb_mem_Check_New_Rec (uint8_t event);
static uint8_t _Cb_mem_Check_Request_At (uint8_t event);
static uint8_t _Cb_mem_Erase (uint8_t event);

/*================ Var struct =================*/  
sEvent_struct sEventAppMem[] = 
{
    { _EVENT_MEM_POWER_ON,  		    0, 0, 100,      _Cb_mem_Power_On },
    { _EVENT_MEM_TEST_WRITE,  		    0, 0, 5,        _Cb_mem_Test_Write }, 
    { _EVENT_MEM_ERASE,  	            0, 0, 0,        _Cb_mem_Erase },
    { _EVENT_MEM_INIT_POS_REC,  		0, 0, 5,        _Cb_mem_Init_Pos_Rec }, 
    { _EVENT_MEM_CHECK_NEW_REC,  		1, 0, 1000,     _Cb_mem_Check_New_Rec },   
    { _EVENT_MEM_CHECK_RQ_AT,  		    0, 0, 2000,     _Cb_mem_Check_Request_At },
};    

sAppMemVariable       sAppMem;

/*================= Function ======================*/
/*--------------Func Callback --------------*/

static uint8_t _Cb_mem_Power_On(uint8_t event)
{
    static uint8_t Step = 0;
    
    switch (Step)
    {
        case 0:
            sEventAppMem[_EVENT_MEM_POWER_ON].e_period = 500;
            AppMem_PowerOff_ExFlash();
            break;
        case 1:
            sEventAppMem[_EVENT_MEM_POWER_ON].e_period = 100;
            AppMem_PowerOn_ExFlash();
            break;
        case 2:
            Step = 0;
            //Set status: init value
            AppMem_Setup_Init();
            fevent_active( sEventAppMem, _EVENT_MEM_TEST_WRITE);
            return 1;
        default:
            Step = 0;
            break;     
    }
    
    Step++;
    fevent_enable( sEventAppMem, event);
    
	return 1;
}

static uint8_t _Cb_mem_Test_Write(uint8_t event)
{
    if (AppMem_Test_Write() == true)
    {
        if (AppMem_Status() == ERROR)
        {
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: mem write error!\r\n" );
            //Quay lai power on lai eflash sau 5s
            sEventAppMem[_EVENT_MEM_POWER_ON].e_period = 10000;
            fevent_enable( sEventAppMem, _EVENT_MEM_POWER_ON);
            return 1;
        }
        
        if (AppMem_Is_Write_To_PartAB() == true)
        {
            fevent_active( sEventAppMem, _EVENT_MEM_INIT_POS_REC);
        } else
        {
            sExFlash.Status_u8 = true;
        }
        
        return 1;
    }
    
    fevent_enable( sEventAppMem, event);
    
	return 1;
}

/*
    Func Init Pos Record in Flash
        + Check Write en: to determine Flash OK
        + Read Rec from start to find Pos

        + Tao 2 case dau:
            ++ HARD RESET
            ++ TEST WRITE
        + Cac case tiep theo Init Data: TSVHA, TSVHB, EventA, EventB, LOG
        
*/
static uint8_t _Cb_mem_Init_Pos_Rec(uint8_t event)
{
#ifdef EX_MEM_FLASH
    switch (sExFlashInit.StepInit_u8)
    {
        case _FLASH_TYPE_DATA_TSVH_A:
            if (eFlash_Check_Pos_PartA(&sRecTSVH) == true)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: check addr tsvh a ok!\r\n" );
                sExFlashInit.StepInit_u8++;
            }
            break;
        case _FLASH_TYPE_DATA_TSVH_B:
            if (eFlash_Check_Pos_PartB(&sRecTSVH) == true)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: check addr tsvh b ok!\r\n" );
                sExFlashInit.StepInit_u8++;
            }
            break;
        case _FLASH_TYPE_DATA_EVENT_A:
            if (eFlash_Check_Pos_PartA(&sRecEvent) == true)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: check addr event a ok!\r\n" );
                sExFlashInit.StepInit_u8++;
            }
            break;
        case _FLASH_TYPE_DATA_EVENT_B:
            if (eFlash_Check_Pos_PartB(&sRecEvent) == true)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: check addr event b ok!\r\n" );
                sExFlashInit.StepInit_u8++;
            }
            break;
        case _FLASH_TYPE_DATA_LOG:
            if (eFlash_Check_Pos_PartA(&sRecLog) == true)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: check addr log a ok!\r\n" );
                sExFlashInit.StepInit_u8++;
            }
            break;
        case _FLASH_TYPE_DATA_LOG + 1:
            sExFlash.Status_u8 = true;  //ket thuc
            return 1;
        default:
            
            break;
    }

    fevent_enable( sEventAppMem, event);

#endif
    
	return 1;
}




static uint8_t _Cb_mem_Check_New_Rec (uint8_t event)
{      
    static uint8_t CountPending = 0;
    static uint16_t MAX_COUNT_PENDING = 0;
    
    if (AppMem_Status() == true)
    {            
        MAX_COUNT_PENDING = MAX_TIME_RETRY_READ / sEventAppMem[_EVENT_MEM_CHECK_NEW_REC].e_period;
        
        /*  + K co item nao trong queue set pending ve false   */
        if ( ( AppMem_Queue_Read_Empty() == true) && (sAppMem.PendingNewMess_u8 == true) )
        {
            CountPending++;
            if (CountPending >= MAX_COUNT_PENDING ) 
            {
                CountPending = 0;
                sAppMem.PendingNewMess_u8 = false;
            }  
        }
        
        if ( sRecGPS.IndexSend_u16 != sRecGPS.IndexSave_u16 )
        {
            if (sAppMem.PendingNewMess_u8 == false)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: pending new mess GPS\r\n" );
                CountPending = 0;
                sAppMem.PendingNewMess_u8 = true;
                
                AppSim_Unmark_Mess(DATA_GPS); //Xoa ban tin da danh dau. Doc lai
                AppMem_Push_To_Read_Queue(_FLASH_TYPE_DATA_GPS_B);
            } 
        }
        
        if ( sRecTSVH.IndexSend_u16 != sRecTSVH.IndexSave_u16 )
        {
            if (sAppMem.PendingNewMess_u8 == false)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: pending new mess TSVH\r\n" );
                CountPending = 0;
                sAppMem.PendingNewMess_u8 = true;
                AppSim_Unmark_Mess(DATA_TSVH_OPERA); //Xoa ban tin da danh dau. Doc lai
                AppMem_Push_To_Read_Queue(_FLASH_TYPE_DATA_TSVH_B);
            } 
        }
        
        if ( sRecEvent.IndexSend_u16 != sRecEvent.IndexSave_u16 )
        {
            if (sAppMem.PendingNewMess_u8 == false)
            {
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: pending new mess EVENT\r\n" );
                CountPending = 0;
                sAppMem.PendingNewMess_u8 = true;
                AppSim_Unmark_Mess(DATA_EVENT);   //Xoa ban tin da danh dau. Doc lai
                AppMem_Push_To_Read_Queue(_FLASH_TYPE_DATA_EVENT_B);
            } 
        }
    }
    
    fevent_enable( sEventAppMem, event);

    return 1;
}




static uint8_t _Cb_mem_Check_Request_At (uint8_t event)
{
    sQueueMemReadManager  qSimFlashReadTemp;
    
    if ( (AppMem_Status() == true) && (sAppMem.sReqATcmd.Status_u8 == true) )
    {
        if (sAppMem.sReqATcmd.IndexStart_u16 != sAppMem.sReqATcmd.IndexStop_u16)
        {
            //Read Record at IndexStart
            switch (sAppMem.sReqATcmd.TypeData_u8)   
            {
                case _FLASH_TYPE_DATA_TSVH_A: 
                    qSimFlashReadTemp.Length_u8    = sRecTSVH.SizeRecord_u16;
                    qSimFlashReadTemp.Addr_u32     = sRecTSVH.AddAStart_u32 + sAppMem.sReqATcmd.IndexStart_u16 * sRecTSVH.SizeRecord_u16; 
                    break;
                case _FLASH_TYPE_DATA_EVENT_A:
                    qSimFlashReadTemp.Length_u8    = sRecEvent.SizeRecord_u16;
                    qSimFlashReadTemp.Addr_u32     = sRecEvent.AddAStart_u32 + sAppMem.sReqATcmd.IndexStart_u16 * sRecEvent.SizeRecord_u16;   
                    break; 
                case _FLASH_TYPE_DATA_LOG:
                    qSimFlashReadTemp.Length_u8    = sRecLog.SizeRecord_u16;
                    qSimFlashReadTemp.Addr_u32     = sRecLog.AddAStart_u32 + sAppMem.sReqATcmd.IndexStart_u16 * sRecLog.SizeRecord_u16;      
                    break; 
                case _FLASH_TYPE_DATA_GPS_A:
                    qSimFlashReadTemp.Length_u8    = sRecGPS.SizeRecord_u16;
                    qSimFlashReadTemp.Addr_u32     = sRecGPS.AddAStart_u32 + sAppMem.sReqATcmd.IndexStart_u16 * sRecGPS.SizeRecord_u16;      
                    break; 
                default:
                    break;
            
            }
            
            qSimFlashReadTemp.TypeData_u8   = _FLASH_TYPE_DATA_OTHER;
            qSimFlashReadTemp.TypeRq_u8     = _FLASH_READ_RESP_AT;
            
            AppMem_Send_To_Queue_Read(&qSimFlashReadTemp);
            
            //Tang index 
            sAppMem.sReqATcmd.IndexStart_u16++;
            if (sAppMem.sReqATcmd.IndexStart_u16 >= sAppMem.sReqATcmd.MaxIndex_u16)
            {
                sAppMem.sReqATcmd.IndexStart_u16 = 0;
            }
        } else
        {
            //Ket thuc
            sAppMem.sReqATcmd.Status_u8 = false;
            return 1;
        }
    }
    
    fevent_enable( sEventAppMem, event);
    
	return 1;
}




static uint8_t _Cb_mem_Erase (uint8_t event)
{
#ifdef EX_MEM_FLASH
    eFlash_Push_Step_Erase_Chip();
#endif
    
#ifdef EX_MEM_EEPROM
    if (CAT24Mxx_Erase() == 0)
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: erase eeprom ok. reset mcu!\r\n" );
        
        if (sExEEPROM.pMem_Erase_Chip_OK != NULL)
            sExEEPROM.pMem_Erase_Chip_OK();
        
        Reset_Chip();
    }
#endif
    
	return 1;
}


/*--------------Func handle --------------*/

uint8_t AppMem_Task(void)
{
    uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_END_MEM; i++)
	{
		if (sEventAppMem[i].e_status == 1)
		{
            if (i != _EVENT_MEM_CHECK_NEW_REC)
                Result = true;

			if ((sEventAppMem[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppMem[i].e_systick)  >=  sEventAppMem[i].e_period))
			{
                sEventAppMem[i].e_status = 0;  //Disable event
				sEventAppMem[i].e_systick = HAL_GetTick();
				sEventAppMem[i].e_function_handler(i);
			}
		}
	}
    
    Result |= OnFlash_Task(); 
    Result |= ExEEPROM_Task(); 
    Result |= eFlash_Task(); 
    
	return Result;
    
}




void AppMem_Init(void)
{
#ifdef MEMORY_ON_FLASH
	Init_Index_Packet();
    //Init queue
    OnFlash_Init();
    sOnFlash.Status_u8 = true;
     
    sOnFlash.pMem_Write_OK = AppMem_Cb_Write_OK;
    sOnFlash.pMem_Read_OK = AppMem_Cb_Read_OK;
    sOnFlash.pMem_Get_Addr_Fr_Type = AppMem_Cb_Get_Addr_From_TypeData;
#endif


#ifdef EX_MEM_EEPROM
    Init_Index_Packet();
    
    ExEEPROM_Init();
    sExEEPROM.Status_u8 = true;
    
    sExEEPROM.pMem_Write_OK = AppMem_Cb_Write_OK;
    sExEEPROM.pMem_Read_OK = AppMem_Cb_Read_OK;
    sExEEPROM.pMem_Get_Addr_Fr_Type = AppMem_Cb_Get_Addr_From_TypeData;
    
    sExEEPROM.pMem_Erase_Chip_OK = AppMem_Cb_Erase_Chip_OK;
    sExEEPROM.pMem_Forward_Data_To_Sim = AppMem_Forward_Data_To_Sim;
#endif

#ifdef EX_MEM_FLASH
    eFlash_Init();
    
    #if (FLASH_MANAGE_TYPE == TYPE_INDEX)
        Init_Index_Packet();
    #endif
            
    fevent_enable( sEventAppMem, _EVENT_MEM_POWER_ON);
    
    sExFlash.pMem_Write_OK = AppMem_Cb_Write_OK;
    sExFlash.pMem_Read_OK  = AppMem_Cb_Read_OK;   
    sExFlash.pMem_Get_Addr_Fr_Type = AppMem_Cb_Get_Addr_From_TypeData;
    
    sExFlash.pMem_Wrtie_ERROR = AppMem_Cb_Wrtie_ERROR;
    sExFlash.pMem_Erase_Chip_OK = AppMem_Cb_Erase_Chip_OK;
    sExFlash.pMem_Erase_Chip_Fail = AppMem_Cb_Erase_Chip_Fail ; 
    sExFlash.pMem_Forward_Data_To_Sim = AppMem_Forward_Data_To_Sim;
#endif

}


/*
    Func: Call back external flash write
        + Increase Index and Addr: Active Event Check New Mess Modem
*/
void AppMem_Cb_Write_OK (uint8_t TypeData)
{
    switch ( TypeData )
    {
        case _FLASH_TYPE_DATA_TSVH_A:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr tsvh a\r\n");
            //Increase AddA_u32 TSVH: If using sim....
            if (AppMem_Inc_Index_Save(&sRecTSVH) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, (uint8_t *) "Full Data Save!\r\n", 17);
            //Active Event check new Mess
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        case _FLASH_TYPE_DATA_TSVH_B:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr tsvh b\r\n");
            //Increase AddB_u32 TSVH: If using sim....
            AppMem_Inc_Index_Send_1(&sRecTSVH, 1);
            //eable Event check new Mess: to period 5s send next
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        case _FLASH_TYPE_DATA_EVENT_A:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr event a\r\n");
            //Increase AddA_u32 Event: If using sim....
            if (AppMem_Inc_Index_Save(&sRecEvent) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_EVENT_B, (uint8_t *) "Full Data Save!\r\n", 17);
            //Active Event check new Mess
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        case _FLASH_TYPE_DATA_EVENT_B:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr event b\r\n");
            //Increase AddB_u32 Event: If using sim....
            AppMem_Inc_Index_Send_1(&sRecEvent, 1);
            //eable Event check new Mess: to period 5s send next
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        case _FLASH_TYPE_DATA_LOG:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr log a\r\n");
            AppMem_Inc_Index_Save(&sRecLog);
            break;
        case _FLASH_TYPE_DATA_GPS_A:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr gps a\r\n");
            if (AppMem_Inc_Index_Save(&sRecGPS) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_GPS_B, (uint8_t *) "Full Data Save!\r\n", 17);
            
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        case _FLASH_TYPE_DATA_GPS_B:
            UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: increase addr gps b\r\n");
            //Increase AddB_u32 Event: If using sim....
            AppMem_Inc_Index_Send_1(&sRecGPS, 1);
            //eable Event check new Mess: to period 5s send next
            fevent_enable( sEventAppMem, _EVENT_MEM_CHECK_NEW_REC);
            break;
        default:
            break;
    }
}


/*
    Func: Call back external flash read
        + Push Record to qFlashSim and active event Check new mess
*/

void AppMem_Cb_Read_OK (uint8_t TypeData, uint32_t Addr, uint8_t TypeReq)
{
    uint8_t MessType = DATA_TSVH_OPERA;   
    uint16_t i = 0;
    uint16_t RealLength = 0;  
    sData   sDataReal;
    uint8_t TempCrc = 0;
    uint8_t IsValidData = true;
    sData   sDataRead;
    
    //Tro vao data read
    sDataRead.Data_a8 = AppMem_sData_Read.Data_a8;
    sDataRead.Length_u16 = AppMem_sData_Read.Length_u16;
        
    RealLength = (*(sDataRead.Data_a8 + 1) << 8) + *(sDataRead.Data_a8 + 2);

    switch (TypeReq)
    {
        case _FLASH_READ_NEW_MESS:      
            if (TypeData == _FLASH_TYPE_DATA_TSVH_B)
            {
                MessType = DATA_TSVH_OPERA;
                if (RealLength >= sRecTSVH.SizeRecord_u16)
                    IsValidData = false;
            }
            
            if (TypeData == _FLASH_TYPE_DATA_EVENT_B)
            {
                MessType = DATA_EVENT;
                if (RealLength >= sRecEvent.SizeRecord_u16)
                    IsValidData = false;
            }
            
            if (TypeData == _FLASH_TYPE_DATA_GPS_B)
            {
                MessType = DATA_GPS;
                if (RealLength >= sRecGPS.SizeRecord_u16)
                    IsValidData = false;
            }

            //Check Crc
            if (IsValidData == true)
            {
                //Tro vao data real
                sDataReal.Data_a8 = sDataRead.Data_a8 + 3;  //1byte 0xAA + 2byte length
                sDataReal.Length_u16 = RealLength;
                
                for (i = 0; i < (RealLength + 3); i++)
                {
                    TempCrc ^= *(sDataRead.Data_a8 + i);
                }
                
                if ( TempCrc != *(sDataRead.Data_a8 + RealLength + 3) )
                    IsValidData = false;
            }
            
            //Copy sang buff App sim Data
            if (IsValidData == true)
            {
                UTIL_Printf_Str( DBLEVEL_H, "u_app_mem: data read from eflash\r\n" );
                UTIL_Printf_Hex( DBLEVEL_H, (uint8_t *) sDataReal.Data_a8, sDataReal.Length_u16);
                UTIL_Printf_Str( DBLEVEL_H, "\r\n" );
                
                AppSim_Get_Data_From_Flash(MessType, sDataReal.Data_a8, sDataReal.Length_u16);
            } else
            {
                UTIL_Log_Str( DBLEVEL_M, "u_app_mem: check invalid read record!\r\n" );
                AppMem_Cb_Check_Mess_Invalid(TypeData);
            }
            break;
        case _FLASH_READ_RESP_AT:
            //Check crc (byte cuoi)
            if ( RealLength >= 256 )
            {
                RealLength = 252;
            }
            //Tro vao data real
            sDataReal.Data_a8 = sDataRead.Data_a8 + 3;
            sDataReal.Length_u16 = RealLength;
            
            AppMem_Process_Resp_AT (sDataReal.Data_a8, sDataReal.Length_u16);
            break;
        case _FLASH_READ_INIT:
            Appmem_Check_Data_Read(Addr, sDataRead.Data_a8, sDataRead.Length_u16);
            break;
        default:
            break;
    }
}



/*
    Func: App sim push data to write to extenal flash
        + Type Data: 0: TSVH ,1: Event  2: LOG
        + PartAorB: Xac dinh dia chi A hay B
        + pData
        + Length
*/
void AppMem_Push_Mess_To_Queue_Write (uint8_t TypeData, uint8_t *pData, uint16_t Length)
{
    sQueueMemWriteManager  qFlashTemp;
    uint16_t Count = 0, i = 0;
    uint8_t TempCrc = 0;
    uint16_t MAX_SIZE_REC = 0;
    
    qFlashTemp.Addr_u32 = 0;   //ban tin du lieu k can truyen vao dia chi
    qFlashTemp.TypeData_u8 = TypeData;
    
    qFlashTemp.aData[Count++] = 0xAA;
    qFlashTemp.aData[Count++] = (Length >> 8) & 0xFF;
    qFlashTemp.aData[Count++] = Length & 0xFF;
        
    for (i = 0; i < Length; i++)
        qFlashTemp.aData[Count++] =  pData[i];
    
    switch (TypeData)   
    {
        case _FLASH_TYPE_DATA_TSVH_A:
        case _FLASH_TYPE_DATA_TSVH_B: 
            MAX_SIZE_REC = sRecTSVH.SizeRecord_u16;
            break;
        case _FLASH_TYPE_DATA_EVENT_A:
        case _FLASH_TYPE_DATA_EVENT_B:
            MAX_SIZE_REC = sRecEvent.SizeRecord_u16;
            break; 
        case _FLASH_TYPE_DATA_LOG:
            MAX_SIZE_REC = sRecLog.SizeRecord_u16;
            break; 
        case _FLASH_TYPE_DATA_GPS_A:
        case _FLASH_TYPE_DATA_GPS_B:
            MAX_SIZE_REC = sRecGPS.SizeRecord_u16;
            break; 
        default:
            break;
    
    }
    
    //Gioi han size luu vao
    if (Count > (MAX_SIZE_REC - 1) )
        Count = MAX_SIZE_REC - 1;
    
    // caculator crc
    Count++;
	for (i = 0; i < (Count - 1); i++)
		TempCrc ^= qFlashTemp.aData[i];

    qFlashTemp.aData[Count-1] = TempCrc;   
    qFlashTemp.Length_u16 = Count;
    
    //Send to queue fwrite
    AppMem_Send_To_Queue_Write(&qFlashTemp);
}




void AppMem_Push_To_Read_Queue (uint8_t TypeData)
{
    sQueueMemReadManager  qSimFlashReadTemp;
       
    //Set Read Flash
    switch (TypeData)   
    {
        case _FLASH_TYPE_DATA_TSVH_B: 
            qSimFlashReadTemp.Length_u8    = sRecTSVH.SizeRecord_u16;
            break;
        case _FLASH_TYPE_DATA_EVENT_B:
            qSimFlashReadTemp.Length_u8    = sRecEvent.SizeRecord_u16;
            break; 
        case _FLASH_TYPE_DATA_LOG:
            qSimFlashReadTemp.Length_u8    = sRecLog.SizeRecord_u16;
            break; 
        case _FLASH_TYPE_DATA_GPS_B:
            qSimFlashReadTemp.Length_u8    = sRecGPS.SizeRecord_u16;
            break; 
        default:
            break;
    
    }
    
    qSimFlashReadTemp.TypeData_u8   = TypeData;
    qSimFlashReadTemp.Addr_u32      = 0;        //Req doc du lieu: k can truyen dia chi vao
    qSimFlashReadTemp.TypeRq_u8     = _FLASH_READ_NEW_MESS;
    
    AppMem_Send_To_Queue_Read(&qSimFlashReadTemp);
}



/*
    Func: check data read in Init flash
        
*/
void Appmem_Check_Data_Read (uint32_t Addr, uint8_t *pData, uint16_t Length)
{  
    switch (sExFlashInit.StepInit_u8)
    {
        case _FLASH_TYPE_DATA_TSVH_A:
        case _FLASH_TYPE_DATA_EVENT_A:
        case _FLASH_TYPE_DATA_LOG:
            eFlash_Check_Data_Init_Pos(Addr, pData, Length, 0);
            break;
        case _FLASH_TYPE_DATA_TSVH_B:
        case _FLASH_TYPE_DATA_EVENT_B:
            eFlash_Check_Data_Init_Pos(Addr, pData, Length, 1);
            break;
        default:
            break;
    }
    
    fevent_active( sEventAppMem, _EVENT_MEM_INIT_POS_REC);
}



/*
    Func: Cb write fail: Do chua dc xoa vung nho. Active Check Pos Init
*/
void AppMem_Cb_Wrtie_ERROR (uint8_t TypeData)
{
    sExFlashInit.StepInit_u8 = 0;
    //Neu quan ly Index -> Van tang vi ti write
    
    if (AppMem_Is_Write_To_PartAB() != true)
    {
        switch ( TypeData )
        {
            case _FLASH_TYPE_DATA_TSVH_A:
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: w fail -> inc tsvh a\r\n");
                AppMem_Inc_Index_Save(&sRecTSVH);
                break;
            case _FLASH_TYPE_DATA_EVENT_A:
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: w fail -> inc event a\r\n");
                AppMem_Inc_Index_Save(&sRecEvent);
                break;
            case _FLASH_TYPE_DATA_LOG:
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: w fail -> inc log a\r\n");
                AppMem_Inc_Index_Save(&sRecLog);
                break;
            case _FLASH_TYPE_DATA_GPS_A:
                UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: w fail -> inc gps a\r\n");
                AppMem_Inc_Index_Save(&sRecGPS);
                break;
            default:
                break;
        }
    }
    
    fevent_active( sEventAppMem, _EVENT_MEM_POWER_ON);
}

/*
    Func: Cb write fail: Do chua dc xoa vung nho. Active Check Pos Init
*/
void AppMem_Cb_Erase_Chip_OK(void)
{
    //Luu chi so ve 0;
    mSave_Index_Record(sRecTSVH.AddIndexSend_u32, 0);
    mSave_Index_Record(sRecTSVH.AddIndexSave_u32, 0);
    
    mSave_Index_Record(sRecEvent.AddIndexSend_u32, 0);
    mSave_Index_Record(sRecEvent.AddIndexSave_u32, 0);
    
    mSave_Index_Record(sRecLog.AddIndexSend_u32, 0);
    mSave_Index_Record(sRecLog.AddIndexSave_u32, 0);
    
    mSave_Index_Record(sRecGPS.AddIndexSend_u32, 0);
    mSave_Index_Record(sRecGPS.AddIndexSave_u32, 0);
    
    //Reset MCU
    Reset_Chip();
}

/*
    Func: Cb write fail: Do chua dc xoa vung nho. Active Check Pos Init
*/
void AppMem_Cb_Erase_Chip_Fail (void)
{
    //Try again?
    UTIL_Log_Str( DBLEVEL_M, "u_app_mem: cb erase chip fail!\r\n" );
}



void AppMem_Erase_New_Sector (uint32_t Addr)
{
#ifdef EX_MEM_FLASH
    if (AppMem_Is_Write_To_PartAB() == true)
    {
        if ( (Addr % S25FL_SECTOR_SIZE) == 0 )
        {
            sExFlash.sHErase.Status_u8 = false;
            sExFlash.sHErase.Addr_u32 = Addr;
            eFlash_Push_Step_Erase_Sector();
        }
    }
#endif 
}



/*
    Func: Callback Get Addr write follow typedataa
*/
uint32_t AppMem_Cb_Get_Addr_From_TypeData (uint8_t TypeData, uint8_t RW)
{
    uint32_t AddrGet_u32 = 0;
    
    switch (TypeData)   
    {
        case _FLASH_TYPE_DATA_TSVH_A: 
            AddrGet_u32 = sRecTSVH.AddA_u32;
            break;
        case _FLASH_TYPE_DATA_TSVH_B: 
            if (RW == __FLASH_READ)
                AddrGet_u32 = sRecTSVH.AddB_u32 - sRecTSVH.Offset_u32;
            else
                AddrGet_u32 = sRecTSVH.AddB_u32;
            break;
        case _FLASH_TYPE_DATA_EVENT_A:
            AddrGet_u32 = sRecEvent.AddA_u32;
            break; 
        case _FLASH_TYPE_DATA_EVENT_B:
            if (RW == __FLASH_READ)
                AddrGet_u32 = sRecEvent.AddB_u32 - sRecEvent.Offset_u32;
            else
                AddrGet_u32 = sRecEvent.AddB_u32;
            break; 
        case _FLASH_TYPE_DATA_LOG:
            AddrGet_u32 = sRecLog.AddA_u32;
            break; 
        case _FLASH_TYPE_DATA_GPS_A:
            AddrGet_u32 = sRecGPS.AddA_u32;
            break; 
        case _FLASH_TYPE_DATA_GPS_B:
            if (RW == __FLASH_READ)
                AddrGet_u32 = sRecGPS.AddB_u32 - sRecGPS.Offset_u32;
            else
                AddrGet_u32 = sRecGPS.AddB_u32;
            break; 
        case _FLASH_TYPE_DATA_OTHER:
            AddrGet_u32 = 0xFFFFFFFF;
            break; 
        default:
            break;
    }
    
    return AddrGet_u32;
}


/*
    Func: Procses RespAT
        + Check Req Port: Server |Serial
            ++ Server: -> Copy to buff at resp server
            ++ Terminal: Print ra uart
*/
void AppMem_Process_Resp_AT (uint8_t *pData, uint16_t Length)
{       
    DCU_Respond(sAppMem.sReqATcmd.Port_u8, pData, Length, 0);
}


/*
    Func: Set Infor index read parameter:
        + Read n rec
*/

uint8_t AppMem_Set_Read_Resp_AT (uint8_t TypeData, uint16_t NumRec)
{
    uint16_t MaxIndex = 0;
    uint8_t TypeTemp = 0;
    uint16_t IndexSaveTemp = 0;
    
    if ( ( NumRec != 0) && (AppMem_Status() == true) )
    {
        switch (TypeData)
        {
            case _RQ_RECORD_TSVH:
                IndexSaveTemp = sRecTSVH.IndexSave_u16;
                TypeTemp = _FLASH_TYPE_DATA_TSVH_A;
                MaxIndex = sRecTSVH.MaxRecord_u16;
                break;
            case _RQ_RECORD_EVENT:
                IndexSaveTemp = sRecEvent.IndexSave_u16;
                TypeTemp = _FLASH_TYPE_DATA_EVENT_A;
                MaxIndex = sRecEvent.MaxRecord_u16;
                break;
            case _RQ_RECORD_LOG:
                IndexSaveTemp = sRecLog.IndexSave_u16;
                TypeTemp = _FLASH_TYPE_DATA_LOG;
                MaxIndex = sRecLog.MaxRecord_u16;
                break;
            case _RQ_RECORD_GPS:
                IndexSaveTemp = sRecGPS.IndexSave_u16;
                TypeTemp = _FLASH_TYPE_DATA_GPS_A;
                MaxIndex = sRecGPS.MaxRecord_u16;
                break;
            default:
                break;
        }
        
        if (NumRec < MaxIndex)
        {
            sAppMem.sReqATcmd.Status_u8 = true;
            sAppMem.sReqATcmd.Port_u8 = PortConfig;
            sAppMem.sReqATcmd.TypeData_u8 = TypeTemp;
            sAppMem.sReqATcmd.IndexStop_u16 = IndexSaveTemp;
            sAppMem.sReqATcmd.MaxIndex_u16 = MaxIndex;
                
            if (sAppMem.sReqATcmd.IndexStop_u16 >= NumRec)
            {
                sAppMem.sReqATcmd.IndexStart_u16 = sAppMem.sReqATcmd.IndexStop_u16 - NumRec;
            } else
            {
                sAppMem.sReqATcmd.IndexStart_u16 = (MaxIndex + sAppMem.sReqATcmd.IndexStop_u16 - NumRec);
            }
            
            //active event check read resp at
            fevent_active( sEventAppMem, _EVENT_MEM_CHECK_RQ_AT);
            UTIL_Printf_Str( DBLEVEL_M, "\r\nOK\r\n" );
            
            return 1;
        } else
        {
            
        }
    }
    
    UTIL_Printf_Str( DBLEVEL_M, "u_app_mem: set read resp at fail!\r\n" );
    
    return 0;
}

/*
    Func: Check Mess read from flash invalid
        + Increase Index send
        + Or EX_MEM_FLASH: Push data to write to part B
    +note: Neu trong queue ton tai nhieu read: sau do invalid thi
            Can check xem bang indexsave truoc khi++
*/
void AppMem_Cb_Check_Mess_Invalid (uint8_t TypeData)
{
    switch (TypeData)
    {
        case _FLASH_TYPE_DATA_TSVH_B:
            if (AppMem_Inc_Index_Send_2(&sRecTSVH, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, (uint8_t *) "Invalid Data\r\n", 14);
            break;
        case _FLASH_TYPE_DATA_EVENT_B:
            if (AppMem_Inc_Index_Send_2(&sRecEvent, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, (uint8_t *) "Invalid Data\r\n", 14);
            break;
        case _FLASH_TYPE_DATA_GPS_B:
            if (AppMem_Inc_Index_Send_2(&sRecGPS, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, (uint8_t *) "Invalid Data\r\n", 14);
            break;
        default:
            break;
    }
}





uint8_t AppMem_Inc_Index_Save (sRecordMemoryManager *sRec)
{
    uint8_t Result = true;
    
    sRec->IndexSave_u16 = (sRec->IndexSave_u16 + 1) % sRec->MaxRecord_u16;
    sRec->AddA_u32 = sRec->AddAStart_u32 + sRec->IndexSave_u16 * sRec->SizeRecord_u16;
    
    if (AppMem_Is_Write_To_PartAB() == true)
    {
        //Xoa sector tiep theo: Neu nhu dung quan ly AB
        AppMem_Erase_New_Sector(sRec->AddA_u32);
    } else
    {
        mSave_Index_Record(sRec->AddIndexSave_u32, sRec->IndexSave_u16);
    }
    
    if (sRec->IndexSave_u16 == sRec->IndexSend_u16)
    {
        if (AppMem_Is_Write_To_PartAB() == true)
        {
            Result = false; 
        } else
        {
            //Tang index send
            sRec->IndexSend_u16 = (sRec->IndexSend_u16 + 1) % sRec->MaxRecord_u16;
            
            sRec->AddB_u32 = sRec->AddBStart_u32 + sRec->IndexSend_u16 * sRec->SizeRecord_u16;
            mSave_Index_Record(sRec->AddIndexSend_u32, sRec->IndexSend_u16);
        }
    }
    
    return Result;
}




uint8_t AppMem_Inc_Index_Send_1 (sRecordMemoryManager *sRec, uint8_t Num)
{       
    sAppMem.PendingNewMess_u8 = false; 
    
    if (sRec->IndexSend_u16 != sRec->IndexSave_u16)
    {
        sRec->IndexSend_u16 = (sRec->IndexSend_u16 + Num) % sRec->MaxRecord_u16;
        sRec->AddB_u32 = sRec->AddBStart_u32 + sRec->IndexSend_u16 * sRec->SizeRecord_u16;
        
        if (AppMem_Is_Write_To_PartAB() == true)
        {
            AppMem_Erase_New_Sector(sRec->AddB_u32);
        } else
        {
            mSave_Index_Record(sRec->AddIndexSend_u32, sRec->IndexSend_u16);
        }
    }
    
    return true;
}
                
                
                
uint8_t AppMem_Inc_Index_Send_2 (sRecordMemoryManager *sRec, uint8_t Num)
{       
    uint8_t Result = true;
    
    sAppMem.PendingNewMess_u8 = false; 
    
    if (sRec->IndexSend_u16 != sRec->IndexSave_u16)
    {
        if (AppMem_Is_Write_To_PartAB() == true)
        {
            Result = false;  
        } else
        {
            sRec->IndexSend_u16 = (sRec->IndexSend_u16 + Num) % sRec->MaxRecord_u16;
            sRec->AddB_u32 = sRec->AddBStart_u32 + sRec->IndexSend_u16 * sRec->SizeRecord_u16;
            
            mSave_Index_Record(sRec->AddIndexSend_u32, sRec->IndexSend_u16);
        } 
    }
    
    return Result;
}


/*
    Func: Chuyen tiep du lieu ghi vao khi mem error
*/
void AppMem_Forward_Data_To_Sim (uint8_t TypeData, uint8_t *pData, uint16_t Length)
{
    uint8_t MessType = DATA_TSVH_OPERA;
    
    switch (TypeData)   
    {
        case _FLASH_TYPE_DATA_TSVH_A:
            MessType = DATA_TSVH_OPERA;
            break;
        case _FLASH_TYPE_DATA_EVENT_A:
            MessType = DATA_EVENT;
            break; 
        case _FLASH_TYPE_DATA_GPS_A:
            MessType = DATA_GPS;
            break; 
        default:
            return;
    
    }
    
    UTIL_Printf_Str( DBLEVEL_H, "u_app_mem: mem forward data to sim\r\n" );
    
    //Copy vao buff sim: Bo qua 3 byte dau va byte cuoi crc     
    AppSim_Get_Data_From_Flash(MessType, (pData + 3), Length - 4);
}


/*
    Func: ngat han nguon cua flash ngoai
        + Power Off power
        + Cau hinh chan SPI ve output: xuat muc 0 
*/

void AppMem_PowerOff_ExFlash (void)
{    
    FLASH_POWER_OFF;
    
    HAL_GPIO_WritePin(GPIOC, FLASH_RESET_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, FLASH_CS_Pin, GPIO_PIN_RESET);
}


/*
    Func: Power on again ex flash
*/

void AppMem_PowerOn_ExFlash (void)
{
    HAL_GPIO_WritePin(GPIOC, FLASH_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, FLASH_RESET_Pin, GPIO_PIN_SET);
    FLASH_POWER_ON;
    MX_SPI2_Init();
}




/*
    Func: Kiem tra xem con record can gui di k
*/

uint8_t AppMem_Check_New_Record (void)
{
    if ( ( sRecTSVH.IndexSend_u16 != sRecTSVH.IndexSave_u16 ) ||
        ( sRecEvent.IndexSend_u16 != sRecEvent.IndexSave_u16 ) )
        return true;
    
    return false;
}

















              