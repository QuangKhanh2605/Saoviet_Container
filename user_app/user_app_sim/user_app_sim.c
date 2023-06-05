

#include "user_app_sim.h"
#include "user_define.h"

#include "user_string.h"
#include "user_modem.h"


/*================ Var struct =================*/  
sEvent_struct sEventAppSim[] = 
{
    { _EVENT_SIM_SEND_MESS,  		0, 0, 2000,     _Cb_Event_Sim_Send_Mess },  
    { _EVENT_SIM_SEND_PING,  		0, 0, 30000,    _Cb_Event_Sim_Send_Ping },
    { _EVENT_SIM_REQ_GPS,  		    1, 0, 5000,     _Cb_Event_Sim_Req_GPS },
    { _EVENT_SIM_GET_GPS,  		    1, 0, 100,      _Cb_Event_Sim_Get_GPS },
};

  
sFuncCallBackModem  sModemCallBackToSimHandler = 
{
    .pSim_Get_Config_UpdateFw   = Modem_Get_Config_UpdateFw,
    .pSim_Reset_MCU             = Reset_Chip,
        
    .pSim_Handler_AT_Cmd        = AppSim_Process_AT_Event,
    .pSim_Handler_Downlink_Server = AppSim_Process_Downl_Mess, 
    .pSim_Process_Sms           = AppSim_Process_Sms,
    .pSim_Get_Fw_Version        = AppSim_Get_Firmware_Version,
};


uint8_t aDATA_FLASH_SIM[256];
uint8_t aDATA_GPS[256];

sAppSimVariable sAppSimVar = 
{
    .sDataFlashSim = {&aDATA_FLASH_SIM[0], 0},
    .sDataGPS      = {&aDATA_GPS[0], 0},
};




/*================ Func =================*/
uint8_t _Cb_Event_Sim_Send_Ping (uint8_t event)
{
    static uint8_t Count_Ping = 0;
    
    if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
    {
        sMQTT.aMARK_MESS_PENDING[DATA_PING] = TRUE;
        fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);   
    } else
    {
        Count_Ping++;
        
        if (Count_Ping < MAX_PING_TO_CONFIG)
        {
            sMQTT.aMARK_MESS_PENDING[DATA_PING] = TRUE;
            fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);            //Tiep tuc cho check Event PING sau 30s.
        } else
        {
            //Ket thuc ping ->Di ngu sau
            Count_Ping = 0;
            sModem.rExternIrq_u8 = FALSE;
        }
    }
    
    return 1;
}


/*
    Cu 5s vao day de push at get GPS
*/
uint8_t _Cb_Event_Sim_Req_GPS (uint8_t event)
{
    /*
        - Qua Init Module sim
        - Neu trong queue con nhieu step (Full) -> bo qua
    */
    
    if (sSimCommon.GroupStepID_u8 > _GR_SETTING)
    {
        if (qGet_Number_Items(&qSimStep) < (SIM_MAX_ITEM_QUEUE - 5))
        {
            sAppSimVar.IsGetGPS_u8 = false;
            //Default struct value GPS
            Sim_Defaul_Struct_GPS();  
            //Push lenh lay toa do
            fPushBlockSimStepToQueue(aSimStepBlockLocation, sizeof(aSimStepBlockLocation)); 
        }
    }
    
    fevent_enable(sEventAppSim, event);

    return 1;
}



uint8_t _Cb_Event_Sim_Get_GPS (uint8_t event)
{
    static uint8_t MarkGPSError_u8 = false;
    
    //At gps ok + Chua lay du lieu
    if ( (sAppSimVar.IsGetGPS_u8 == false) && (sSimCommon.sGPS.Status_u8 != false) && (sRTC.year > 20) )
    {
        sAppSimVar.IsGetGPS_u8 = true;  
        
        //neu tiep tuc gps error bo qua dong goi
        if ( (sSimCommon.sGPS.Status_u8 == error) && (MarkGPSError_u8 == true) )
        {
            UTIL_Printf_Str( DBLEVEL_M, "u_app_sim: gps error continue!\r\n" );
            
            fevent_enable(sEventAppSim, event);
            return 1;
        }
        
        //Copy to Buffer data
        if (sAppSimVar.sDataGPS.Length_u16 == 0)
        {
            //Them Stime vao truoc
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = OBIS_TIME_DEVICE;   // sTime
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = 0x06;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.year;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.month;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.date;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.hour;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.min;
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sRTC.sec;
        } else
        {
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = ';';
        }

        //Neu status = true, packet data or status = error packet 0,0 util have data again
        if (sSimCommon.sGPS.Status_u8 == true)
        {
            MarkGPSError_u8 = false;
            
            for (uint16_t i = 0; i < sSimCommon.sGPS.LengData_u8; i++)
            {
                *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = sSimCommon.sGPS.aPOS_INFOR[i];  
                
                if (sAppSimVar.sDataGPS.Length_u16 >= 250)
                    break;
            }
        } else if (MarkGPSError_u8 == false)
        {
            MarkGPSError_u8 = true;
            
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = '0';
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = ',';
            *(sAppSimVar.sDataGPS.Data_a8 + sAppSimVar.sDataGPS.Length_u16++) = '0';
        }
        
        //Kiem tra xem da day buff chua: -> Luu vao -> gui di
        if (sAppSimVar.sDataGPS.Length_u16 > 200)
        {
            //Send to queue write 
            AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_GPS_A, sAppSimVar.sDataGPS.Data_a8, sAppSimVar.sDataGPS.Length_u16);
            //Memset data
            sAppSimVar.sDataGPS.Length_u16 = 0;
        }
    }
  
    fevent_enable(sEventAppSim, event);

    return 1;
}




/*
    
*/
uint8_t _Cb_Event_Sim_Send_Mess (uint8_t event)
{
    switch (sSimCommon.PowerStatus_u8)
    {
        case _POWER_START:
            Sim_Disable_All_Event();
            fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
            break;
        case _POWER_INIT:   
            break;
        case _POWER_CONN_MQTT:
            if (AppSim_Send_Mess () == 1)  //Co ban tin
            {
                //Have Mess need Send: Enable even again + Unmark Send PING
                if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
                    sEventAppSim[_EVENT_SIM_SEND_PING].e_status = 0;
                
                sEventAppSim[event].e_period = 2000;
                fevent_enable(sEventAppSim, event);
                
            } else
            {
                sEventAppSim[event].e_period = 10;
                //Neu Online: enable Ping  | Enable event Power SIM
                if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
                {
                    //Mark Send PING
                    if (sEventAppSim[_EVENT_SIM_SEND_PING].e_status == 0)
                        fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);
                    
                    fevent_enable(sEventAppSim, event);
                } else
                {
                    //Irq External to config by server
                    if (sModem.rExternIrq_u8 == TRUE)
                    {
                        //Mark Send PING
                        if (sEventAppSim[_EVENT_SIM_SEND_PING].e_status == 0)
                            fevent_enable(sEventAppSim, _EVENT_SIM_SEND_PING);
                        
                        fevent_enable(sEventAppSim, event);
                    } else
                    {
                        //POW off module SIM -> go to lowpower
                        fevent_active(sEventSim, _EVENT_SIM_POWER_OFF);
                    }
                }
            }
            
            break;
        case _POWER_PSM:
            //Active Event Power ON SIM
            fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
            break;
        case _POWER_POWER_OFF:
            //Active Event Power ON SIM
            Sim_Disable_All_Event();
            fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
            break;
        default:
            break;
    }
          
    return 1;
}


/*============== Function Handler ====================*/   

/*
    Func Init App SIM
*/

void AppSim_Init (void)
{
    //Func Pointer Lib Sim
    sFuncCBModem = &sModemCallBackToSimHandler;
    //Init Module Sim
    Sim_Init();
    //Mode power of modem
    sSimVar.ModePower_u8 = &sModem.ModeSimPower_u8; 
    
    sMQTT.aMARK_MESS_PENDING[DATA_HANDSHAKE] = TRUE;
    sMQTT.aMARK_MESS_PENDING[DATA_INTAN_TSVH] = TRUE;
}



uint8_t AppSim_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;

	for (i = 0; i < _EVENT_END_SIM; i++)
	{
		if (sEventAppSim[i].e_status == 1)
		{
            Result = 1;
            
			if ((sEventAppSim[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppSim[i].e_systick)  >=  sEventAppSim[i].e_period))
			{
                sEventAppSim[i].e_status = 0; 
				sEventAppSim[i].e_systick = HAL_GetTick();
				sEventAppSim[i].e_function_handler(i);
			}
		}
	}

	return Result;
}



/*
    Func: Check New Mess
*/

uint8_t AppSim_Send_Mess (void)
{
    uint16_t i = 0;
    uint8_t Result = FALSE;
    
    //Kiem tra xem co ban tin nao can gui di khong
    for (i = TOPIC_NOTIF; i < END_MQTT_SEND; i++)
    {
        if (sMQTT.aMARK_MESS_PENDING[i] == TRUE)
        {
            Result = TRUE;
            //
            if (sMQTT.Status_u8 != PENDING)
            {
                //Danh dau Mess type hiên tai ->neu OK clear di
                sMQTT.MessType_u8 = i;
                //Dong goi ban tin vao buff aPAYLOAD bang cach thuc hien callback
                if (sMark_MessageSend_Type[i].CallBack(i) == TRUE)
                {
                    //Day 2 step Publish vao Queue send AT: 2 option wait ACK and No wait ACK
                    if (i < SEND_RESPOND_SERVER)
                        Sim_Push_AT_Publish_Fb ();
                    else
                        Sim_Push_AT_Publish (); 
                     //Set flag status= pending
                    sMQTT.Status_u8 = PENDING;
                } else
                    sMQTT.aMARK_MESS_PENDING[i] = FALSE;
            } 
        }
    }
    
    return Result;
}



/*
    Func: Func pointer Handler AT cmd
        + Input: Type: Get ID SIM,....
        + Ouput: Handler
*/

uint8_t AppSim_Process_AT_Event (uint8_t Type)
{    
    switch (Type)
    { 
        case _SIM_COMM_EVENT_GET_STIME:  
            _CbAppSim_Recv_sTime (sSimInfor.sTime);
            break;
        case _SIM_COMM_EVENT_GPS_OK:
//            fPushBlockSimStepToQueue(aSimStepBlockDelLocation, sizeof(aSimStepBlockDelLocation));  
            break;
        case _SIM_COMM_EVENT_TCP_SEND_1:  
            _CbAppSim_TCP_Send_1(&sMQTT.str);
            break;
        case _SIM_COMM_EVENT_TCP_SEND_2:  
            _CbAppSim_TCP_Send_2(&sMQTT.str);
            break;
        case _SIM_COMM_EVENT_CONN_MQTT_1:
            mConnect_MQTT();
            break;
        case _SIM_COMM_EVENT_SUB_MQTT_1:
            //Truyen chuoi Subcribe
            mSubcribe_MQTT ();
            break;
        case _SIM_COMM_EVENT_SUB_MQTT_2:
            mSet_default_MQTT(); 
            fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);
            UTIL_Printf_Str( DBLEVEL_M, "u_app_sim: mqtt connect ok!\r\n" );
            break;
        case _SIM_COMM_EVENT_PUB_MQTT_1:  
            mPublish_MQTT();
            break;
        case _SIM_COMM_EVENT_PUB_MQTT_2:
            _CbAppSim_Recv_PUBACK ();
            break;
        case _SIM_COMM_EVENT_HTTP_UPDATE_OK: 
            Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
            //ghi Flag update va Size firm vao Inflash
            HAL_FLASH_Unlock();
            HAL_Delay(10);
            
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ADDR_FLAG_HAVE_NEW_FW, 0xAA);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ADDR_FLAG_HAVE_NEW_FW + FLASH_BYTE_WRTIE, sSimFwUpdate.CountByteTotal_u32);
              
            HAL_Delay(10);
            HAL_FLASH_Lock();
            
            sMQTT.aMARK_MESS_PENDING[SEND_UPDATE_FIRM_OK] = TRUE;
            //Init Sim again
            sSimVar.ModeConnectNow_u8 = sSimVar.ModeConnectLast_u8;
            Sim_Disable_All_Event();
            fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);
            break;
        case _SIM_COMM_EVENT_HTTP_UPDATE_FAIL:
            sMQTT.aMARK_MESS_PENDING[SEND_UPDATE_FIRM_FAIL] = TRUE;
            break;
        default: 
            break;
    }
    
    return 1;
}

   


/*
    Func: _Cb Get sTime From AT cmd
        + Input: sTime
        + Active Log TSVH for the first Get sTime
*/

void _CbAppSim_Recv_sTime (ST_TIME_FORMAT sTimeSet)
{
    static uint8_t  MarkFirstGetTime = 0;
    
    sRTCSet.year   = sTimeSet.year;
    sRTCSet.month  = sTimeSet.month;
    sRTCSet.date   = sTimeSet.date;
    sRTCSet.hour   = sTimeSet.hour;
    sRTCSet.min    = sTimeSet.min;
    sRTCSet.sec    = sTimeSet.sec;
    sRTCSet.day    = sTimeSet.day;

#ifdef SIM_EC200U_LIB
    Convert_sTime_ToGMT(&sRTCSet, 7); 
    //Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
    sRTCSet.day = ((HW_RTC_GetCalendarValue_Second (sRTCSet, 1) / SECONDS_IN_1DAY) + 6) % 7 + 1;
#endif
    
    fevent_active(sEventAppComm, _EVENT_SET_RTC);
    //active event log mess first
    if (MarkFirstGetTime == 0)
    {
        fevent_active(sEventAppComm, _EVENT_IDLE);
    #ifdef USING_APP_WM
        fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
    #endif
        MarkFirstGetTime = 1;
    }
}

/*
    Func: _cb Publish AT cmd
        + Set Status Send OK
        + Increase Index send
        + Active Event Send Mess
*/

void _CbAppSim_Recv_PUBACK (void)
{
    sMQTT.Status_u8 = TRUE;   //Set status ve true
        
    switch (sMQTT.MessType_u8)
    {
        case DATA_TSVH:
            if (AppMem_Inc_Index_Send_2(&sRecTSVH, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, sAppSimVar.sDataFlashSim.Data_a8, sAppSimVar.sDataFlashSim.Length_u16);
            
            //Unmark mess 
            AppSim_Unmark_Mess(sMQTT.MessType_u8);
            
            break;
        case DATA_TSVH_OPERA:
            if (AppMem_Inc_Index_Send_2(&sRecTSVH, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_B, sAppSimVar.sDataFlashSim.Data_a8, sAppSimVar.sDataFlashSim.Length_u16);
            
            //Unmark mess 
            AppSim_Unmark_Mess(sMQTT.MessType_u8);

            break;
        case DATA_EVENT:
            if (AppMem_Inc_Index_Send_2(&sRecEvent, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_EVENT_B, sAppSimVar.sDataFlashSim.Data_a8, sAppSimVar.sDataFlashSim.Length_u16);
           
            //Unmark mess 
            AppSim_Unmark_Mess(sMQTT.MessType_u8);
            
            break;
        case DATA_GPS:
            if (AppMem_Inc_Index_Send_2(&sRecGPS, 1) == false)
                AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_GPS_B, sAppSimVar.sDataFlashSim.Data_a8, sAppSimVar.sDataFlashSim.Length_u16);
           
            //Unmark mess 
            AppSim_Unmark_Mess(sMQTT.MessType_u8);
            
            break;
        case SEND_SAVE_BOX_OK:
            sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE; 
            fevent_active(sEventAppComm, _EVENT_SAVE_BOX);
            break;
        case SEND_SHUTTING_DOWN:
        	Reset_Chip_Immediately();
            break;
        case SEND_UPDATE_FIRM_OK:
        case SEND_UPDATE_FIRM_FAIL:
            sSimVar.IsUpdateFinish_u8 = TRUE;          
            sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE; 
            Reset_Chip();
            break;
        default:
            sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE;  
            break;
    }
            
    sSimVar.ConnSerStatus_u8 = TRUE;  //Connected to server
    sSimVar.CountHardReset_u8 = 0;
    sSimVar.CountSoftReset_u8 = 0;
}
    


void AppSim_Process_Downl_Mess (sData *sUartSim)
{
    uint8_t var = 0;
    int PosFind = 0;
    
    for (var = REQUEST_RESET; var < END_MQTT_RECEI; ++var)
    {
        PosFind = Find_String_V2 ((sData*) &sMark_MessageRecei_Type[var].sKind, sUartSim);
        
        if ((PosFind >= 0) && (sMark_MessageRecei_Type[var].CallBack != NULL))
            sMark_MessageRecei_Type[var].CallBack(sUartSim, PosFind);
    }
}

void AppSim_Process_Sms (sData *sUartSim)
{
    UTIL_Printf_Str( DBLEVEL_M, "Check AT Request by SMS\r\n" );
    Check_AT_User(sUartSim, _AT_REQUEST_SERVER);  //check cac lenh AT cmd
}




/*
    Func: Process at: AT+CIPSEND=cid,length...
        + Send: Cid + length Data
*/

void _CbAppSim_TCP_Send_1 (sData *pData)
{
    uint8_t aDATA_TEMP[40] = {0};
    sData   strDataTemp = {&aDATA_TEMP[0], 0};
    
    *(strDataTemp.Data_a8 + strDataTemp.Length_u16++) = CID_SERVER; 
    *(strDataTemp.Data_a8 + strDataTemp.Length_u16++) = ',';
    Convert_Uint64_To_StringDec(&strDataTemp, pData->Length_u16, 0);
    
    Sim_Common_Send_AT_Cmd(&uart_sim, strDataTemp.Data_a8, strDataTemp.Length_u16, 1000);
}


/*
    Func: Process at: AT+CIPSEND=cid,length...
        + Send: Data after check '>'
*/
void _CbAppSim_TCP_Send_2 (sData *pData)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, pData->Data_a8, pData->Length_u16, 1000);
}



sData * (AppSim_Get_Firmware_Version) (void)
{
    return &sFirmVersion;
}


/*
    Func: Get buffer data from Flash
        + Check crc and format again: 
            + False: -> Luu vao flash part B and skip 
            + true: Copy data to buff sim mqtt
                    mark new mess to send now
*/


void AppSim_Get_Data_From_Flash (uint8_t MessType, uint8_t *pData, uint16_t Length)
{ 
    //Memset buff 
    Reset_Buff (&sAppSimVar.sDataFlashSim);
          
    if (Length > sizeof (aDATA_FLASH_SIM) )
        Length = sizeof (aDATA_FLASH_SIM);
    
    for (uint16_t i = 0; i < Length; i++)
        *(sAppSimVar.sDataFlashSim.Data_a8 + sAppSimVar.sDataFlashSim.Length_u16++) = *(pData + i);
    
    //active event send mess mqtt
    sMQTT.aMARK_MESS_PENDING[MessType] = TRUE;  
    //Event
    fevent_active( sEventAppSim, _EVENT_SIM_SEND_MESS);
}



void AppSim_Unmark_Mess (uint8_t TypeMess)
{
    sMQTT.aMARK_MESS_PENDING[TypeMess] = FALSE; 
    Reset_Buff(&sAppSimVar.sDataFlashSim);
}






