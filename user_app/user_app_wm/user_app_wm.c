


#include "user_define.h"

#include "user_app_wm.h"
#include "user_time.h"

#include "user_modem.h"
#include "user_message.h"

#include "user_adc.h"
#include "adc_if.h"

/*=================Static func ======================*/
static uint8_t _Cb_Log_TSVH (uint8_t event);
static uint8_t _Cb_Meas_Pressure(uint8_t event);
static uint8_t _Cb_Entry_Data_WM (uint8_t event);
static uint8_t _Cb_Intan_IRQ(uint8_t event);
static uint8_t _Cb_Wating_Config (uint8_t event);

/*=================Var struct======================*/
    
Struct_Battery_Status    sBattery;
Struct_Battery_Status    sVout;
Struct_Battery_Status    sPressure;
Struct_Pulse			 sPulse;

struct_ThresholdConfig   sMeterThreshold =
{
    .FlowHigh = 0xFFFF,
    .FlowLow = 0,
    .PeakHigh = 0xFFFF,
    .PeakLow = 0,
    .LowBatery = 10,
};

sLastRecordInfor    sLastPackInfor = { {0}, 0};

uint8_t aMARK_ALARM[10];
uint8_t aMARK_ALARM_PENDING[10];



sEvent_struct sEventAppWM[] =
{
    { _EVENT_ENTRY_WM, 		        0, 0, 0, 	    _Cb_Entry_Data_WM }, 
	{ _EVENT_LOG_TSVH, 		        0, 0, 0, 	    _Cb_Log_TSVH },
    { _EVENT_MEAS_PRESSURE,		    0, 0, 3000,     _Cb_Meas_Pressure }, 
    
    { _EVENT_INTAN_IRQ,             0, 0, 100, 	    _Cb_Intan_IRQ },     //Ngat de 100ms chong nhieu 
    { _EVENT_WATING_CONFIG,         0, 0, 5000,	    _Cb_Wating_Config }, 
};


SWaterMeterVariable   sWmVar;


/*
    Func: Entry get data WM
        + Moi ung ung wm co nhieu thanh phan khac nhau
             Level.. Presss..
        + Nen can 1 diem vao chung: Sau nay chi can sua o day
*/
static uint8_t _Cb_Entry_Data_WM (uint8_t event)
{
    fevent_active(sEventAppWM, _EVENT_MEAS_PRESSURE);

    return 1;
}


/*
    Func: Log TSVH
        + Get Infor Device: To check Alarm
            ++ if have alarm: Active Send TX immediately
        + If count > Max_Count_Packet_Data: 
            ++ Packet Data
            ++ Save to Mem
            ++ active send Tx immediately
*/
static uint8_t _Cb_Log_TSVH (uint8_t event)
{
    static uint8_t TestCountStimeRq = 0;
    uint8_t MarkSendTx = false;
    
    UTIL_Printf((uint8_t*) "Log TSVH data\r\n", 15);
#ifndef  DEVICE_TYPE_GATEWAY
    
#ifdef LPTIM_ENCODER_MODE 
    LPTIME_Encoder_Get_Pulse(&sPulse);
#else   
    LPTIM_Counter_Get_Pulse(&sPulse);
    Modem_Check_Add_5Pulse(&sPulse);
#endif
    Get_RTC();
    /*
        Cacul and check alarm: Vbat, Threshold flow, peak, Dir change
        If have Alarm mess -> Send immediately
    */
    if (Modem_Check_Alarm() == 1)   
    {
        //Dong goi Event Mess
        Modem_Log_Data_Event (&sRecEvent);
        MarkSendTx = true;
    }
    
    //Get Last Packet Infor
    Modem_Get_Last_Packet_Infor();  
    //Packet TSVH and Save to Mem
    Modem_Log_Data_TSVH(&sRecTSVH);    
    //Increase Count To Send Mess
    sModem.CountSleepToSend_u8++;
	if (sModem.CountSleepToSend_u8 >= sFreqInfor.NumWakeup_u8)
	{
        sModem.CountSleepToSend_u8 = 0;
        MarkSendTx = true;
	}
    
    if (MarkSendTx == true)
    {
        //Active Event Lora TX
    #ifdef USING_APP_LORA
        sMessVar.aMARK[_MESS_TSVH] = TRUE;
        sMessVar.aMARK[_MESS_EVENT] = TRUE;
        
        //Set timer send tx theo ID
        AppLora_Set_Timer_Tx(sModem.TimeDelayTx_u32);
    #endif
    }
    //Gui rq Stime sau 2 lan TSVH
    TestCountStimeRq++;
    if ( TestCountStimeRq >= (*(sModem.sDCU_id.Data_a8 + sModem.sDCU_id.Length_u16 - 1) - 0x30 + 5) )
    {
        TestCountStimeRq = 0;
        sCtrlLora.CtrlStime_u8 = TRUE;
    }
#endif
    
    return 1;
}


static uint8_t _Cb_Meas_Pressure(uint8_t event)
{
//    if (Get_Press_Val(&sModem.PressureValue_u32) == FALSE)
//    {
//        fevent_enable(sEventAppWM, _EVENT_MEAS_PRESSURE);
//        sModem.rGetCSQagain_u8 = TRUE;   //Get CSQ each Get TSVH
//        //Disable mess send
//        sModem.IsPendingMeasPress_u8 = TRUE;
//    } else
//    {
//    	//neu co req Intan -> send intan | Log TSVH de gui len server
//    	if (sModem.IsReqIntanMess_u8 == TRUE)
//    	{
//    		sModem.IsReqIntanMess_u8 = FALSE;
//    		sMQTT.aMARK_MESS_PENDING[DATA_INTAN_TSVH] = TRUE;
//    	} else
//    	{
//    		fevent_active(sEventAppWM, _EVENT_LOG_TSVH);
//    	}
//
//        sModem.IsPendingMeasPress_u8 = FALSE;
//    }
    
    if ( sWmVar.IsRqIntan_u8 == true )
    {
        //Active Intan
        sWmVar.IsRqIntan_u8 = false;
        sMessVar.aMARK[_MESS_INTAN] = TRUE;
        fevent_active(sEventAppLora, _EVENT_LORA_TX);
    } else
    {
        fevent_active(sEventAppWM, _EVENT_LOG_TSVH);
    }
   
    return 1;
}

static uint8_t _Cb_Intan_IRQ(uint8_t event)
{
    UTIL_Printf((uint8_t*) "=Event Irq Send Intan=\r\n", 24 );
    /*
        - Module wakeup: va gui bai tin intan
    */
    #ifdef USING_APP_WM
        //Active Event Check Alarm
        fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
        sWmVar.IsRqIntan_u8 = true;
    #endif
    
    #ifdef USING_APP_SIM
        sMQTT.aMARK_MESS_PENDING[DATA_INTAN_TSVH] = TRUE;
        //Send TSVH if have new mess
        if (Flash_Check_Have_Packet (sRecTSVH.IndexSave_u16, sRecTSVH.IndexSend_u16) == TRUE)
            sMQTT.aMARK_MESS_PENDING[DATA_TSVH_OPERA] = TRUE;

        fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);  
    #endif
    
    //active event waiting config
    fevent_active(sEventAppWM, _EVENT_WATING_CONFIG);
        
    return 1;
}


static uint8_t _Cb_Wating_Config (uint8_t event)
{
    UTIL_Printf((uint8_t*) "Nhap lenh AT: \r\n", 16 );

    if (sModem.CountEventWaitConfg_u8 < 5)
    {
        sModem.CountEventWaitConfg_u8++;
        fevent_enable(sEventAppWM, _EVENT_WATING_CONFIG);
    } else
    {
        sModem.CountEventWaitConfg_u8 = 0;
    }
    return 1;
}

uint8_t AppWm_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;

	for (i = 0; i < _EVENT_END_WM; i++)
	{
		if (sEventAppWM[i].e_status == 1)
		{
            Result = 1;
            
			if ((sEventAppWM[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppWM[i].e_systick)  >=  sEventAppWM[i].e_period))
			{
                sEventAppWM[i].e_status = 0;  //Disable event
				sEventAppWM[i].e_systick = HAL_GetTick();
				sEventAppWM[i].e_function_handler(i);
			}
		}
	}

	return Result;
}



void Modem_Get_Last_Packet_Infor (void)
{
    //Last Pulse
#ifdef LPTIM_ENCODER_MODE
	sLastPackInfor.Pulse_u32 = sPulse.Number_u32 / 4;
#else
	sLastPackInfor.Pulse_u32 = sPulse.Number_u32;
#endif

    //Last Time
    sLastPackInfor.sTime.year   = sRTC.year;
    sLastPackInfor.sTime.month  = sRTC.month;
    sLastPackInfor.sTime.date   = sRTC.date;
    
    sLastPackInfor.sTime.hour   = sRTC.hour;
    sLastPackInfor.sTime.min    = sRTC.min;
    sLastPackInfor.sTime.sec    = sRTC.sec;
}



//check xem c? phai chip moi khoi dong lai khong. De c?ng 5 xung cua dong ho
void Modem_Check_Add_5Pulse (Struct_Pulse *sPulse)
{
    if (sModem.IsOverFivePulse_u8 == 0)
    {
        if (sPulse->Number_u32 != sPulse->NumberStartInit_u32)
        {
            sPulse->Number_u32  += 5;
            sModem.IsOverFivePulse_u8 = 1;
        }
    }
}



uint32_t TempPulse = 0;

uint8_t Modem_Packet_TSVH (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;   
    //----------------------- start send data below --------------------- //
    
    pData[length++] = OBIS_SERI_SENSOR; 
    pData[length++] = sModem.sDCU_id.Length_u16;
    
    for (i = 0; i < sModem.sDCU_id.Length_u16; i++)
        pData[length++] = *(sModem.sDCU_id.Data_a8 + i);
    
    //----------sTime--------------------
    pData[length++] = OBIS_TIME_DEVICE;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
    
    
    //----------chi so xung--------------------
    pData[length++] = OBIS_WM_PULSE;   // Chi so hien tai
    pData[length++] = 0x04;
#ifdef LPTIM_ENCODER_MODE 
//    uint32_t TempPulse = sPulse.Number_u32 / 4;
    
    TempPulse++;
    
	pData[length++] = (TempPulse)>>24;   
    pData[length++] = (TempPulse)>>16;
    pData[length++] = (TempPulse)>>8;
    pData[length++] = (TempPulse);
#else   
    pData[length++] = (sPulse.Number_u32)>>24;   
    pData[length++] = (sPulse.Number_u32)>>16;
    pData[length++] = (sPulse.Number_u32)>>8;
    pData[length++] = (sPulse.Number_u32);
#endif
    pData[length++] = 0x00;   //scale   
    
    //----------luu luong --------------------
    pData[length++] = OBIS_WM_FLOW;  // Luu luong
    pData[length++] = 0x02;
    pData[length++] = 0;
    pData[length++] = 0;
    pData[length++] = 0x00;    
    
    //----------Ap suat--------------------
    pData[length++] = OBIS_WM_PRESSURE; // Ap suat
    pData[length++] = 0x04;
    pData[length++] = (sModem.PressureValue_u32)>>24;
    pData[length++] = (sModem.PressureValue_u32)>>16;
    pData[length++] = (sModem.PressureValue_u32)>>8;
    pData[length++] = sModem.PressureValue_u32;
    pData[length++] = 0xFD;   //scale   
    
    //----------Dien ap Pin--------------------
    pData[length++] = OBIS_DEV_VOL1; // Dien ap pin
    pData[length++] = 0x02;
    pData[length++] = (sBattery.mVol_u32)>>8;
    pData[length++] = (sBattery.mVol_u32);
    pData[length++] = 0xFD;
    
    //----------Cuong do song--------------------
    #ifdef USING_APP_SIM
        pData[length++] = OBIS_RSSI_1; 
        pData[length++] = 0x01;
        pData[length++] = sSimInfor.RSSI_u8;
        pData[length++] = 0x00;   
    #endif
        
    #ifdef USING_APP_LORA
        pData[length++] = OBIS_RSSI_2; 
        pData[length++] = 0x01;
        pData[length++] = sLoraVar.Rssi_u8;
        pData[length++] = 0x00;   
    #endif
    //----------Dien ap Pin--------------------
    pData[length++] = OBIS_DEV_VOL2; // Dien ap pin
    pData[length++] = 0x02;
    pData[length++] = (sVout.mVol_u32)>>8;
    pData[length++] = (sVout.mVol_u32);
    pData[length++] = 0xFD;
    
    //----------Tan suat--------------------
    pData[length++] = OBIS_FREQ_SEND; 
    pData[length++] = 0x02;
    pData[length++] = (sFreqInfor.FreqSendUnitMin_u32 >> 8) & 0xFF;
    pData[length++] = sFreqInfor.FreqSendUnitMin_u32 & 0xFF;
    pData[length++] = 0x00;
  
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    
    return length;
}


uint8_t Modem_Packet_Event (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    
    //----------------------- start send data below ---------------------
    pData[length++] = 0x01;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
    
    //Chac chan vao day da co 1 alarm
    for (i = 0; i < _ALARM_END;i++)
    {
        if (aMARK_ALARM[i] == 1)
        {
            //Obis
            pData[length++] = i;
            //Length +Data + scale
            switch (i)
            {
                case _ALARM_FLOW_LOW:
                case _ALARM_FLOW_HIGH:
                    pData[length++] = 4;  //length
                    pData[length++] = (sPulse.Flow_i32 >> 24) & 0xFF;  //
                    pData[length++] = (sPulse.Flow_i32 >> 16) & 0xFF; //
                    pData[length++] = (sPulse.Flow_i32 >> 8) & 0xFF;  //
                    pData[length++] = sPulse.Flow_i32 & 0xFF;  // doi vi pulse/hour
                    pData[length++] = 0x00;
                    break;
                case _ALARM_PEAK_LOW:
                case _ALARM_PEAK_HIGH:
                    pData[length++] = 4;  //length
                    pData[length++] = (sPulse.Quantity_i32 >> 24) & 0xFF;   //
                    pData[length++] = (sPulse.Quantity_i32 >> 16) & 0xFF;   //
                    pData[length++] = (sPulse.Quantity_i32 >> 8) & 0xFF;    //
                    pData[length++] = sPulse.Quantity_i32 & 0xFF;           // doi vi pulse
                    pData[length++] = 0x00;
                    break;
                case _ALARM_VBAT_LOW:
                    pData[length++] = 2;  //length
                    pData[length++] = 0;   //  (sBattery.Level_u16 >> 8) & 0xFF;  //
                    pData[length++] = 1;   //  sBattery.Level_u16 & 0xFF;         // doi vi %
                    pData[length++] = 0x00;
                    break;
                case _ALARM_DIR_CHANGE:
                    pData[length++] = 1;     //length
                    pData[length++] = sPulse.Direct_u8; 
                    pData[length++] = 0x00;
                    break;
                case _ALARM_DETECT_CUT:
                    pData[length++] = 1;     //length
                    pData[length++] = sModem.DetectCutStatus_u8; 
                    pData[length++] = 0x00;
                    break;
                default:
                    break;   
            }
        }
        
        //Reset Alarm
        aMARK_ALARM[i] = 0;
    }  
            
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    return length;
}


/*
    Func: Log Data Event
        + Packet Event Data
        + Save to Flash or ExMem
*/

void Modem_Log_Data_Event (StructManageRecordFlash *sRecordEvent)
{
    uint8_t     aMessData[128] = {0};
    uint8_t     Length = 0;
    
//    if (sRTC.year <= 20)
//        return;
    
    Length = Modem_Packet_Event (&aMessData[0]);
	//Luu vao Flash
    #ifdef MEMORY_ON_FLASH
        Flash_Save_Record (sRecordEvent, &aMessData[0], Length);
    #else       
        ExMem_Save_Record (sRecordEvent, &aMessData[0], Length);
    #endif
}



/*
    Func: Log Data TSVH
        + Packet Data TSVH
        + Save to Flash or ExMem
*/
void Modem_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH)
{
    uint8_t     aMessData[128] = {0};
    uint8_t     Length = 0;
    
//    if (sRTC.year <= 20)
//        return;
    
    Length = Modem_Packet_TSVH (&aMessData[0]);
	
    //Luu vao Flash
    #ifdef MEMORY_ON_FLASH
        Flash_Save_Record (sRecordTSVH, &aMessData[0], Length);
    #else       
        ExMem_Save_Record (sRecordTSVH, &aMessData[0], Length);
    #endif
}




int32_t Modem_Cacul_Quantity (uint32_t PulseCur, uint32_t PulseOld)
{
    int32_t Qual = 0;
    
    if (PulseCur >=  PulseOld)
	{
		if ((PulseCur - PulseOld) < 0xFFFFFFAF)   //khong the quay nguoc dc 0x50 xung trong 10p va khong the quay tien 0xFFAF
			Qual = (PulseCur - PulseOld);  //binh thuong
		else
			Qual = - (0xFFFFFFFF - PulseCur + PulseOld + 1); //quay nguoc
	} else
	{
		if ((PulseOld - PulseCur) > 0x50)   //khong the quay nguoc dc 0x50 xung trong 10p
			Qual = (0xFFFFFFFF - PulseOld + PulseCur + 1);  //binh thuong
        else
			Qual = - (PulseOld - PulseCur);  // quay ngược
	}
    
    return Qual;
}


/*
    - Function tinh toan va check alarm
        + Co the doc ra tu record truoc va lay lai duoc chi so o phia truoc + thoi gian: Sau do tính
        + Neu nhu thiet bi duoc cam moi hoan toan: 
                *Index Record  = 0
                *Page cuoi cung se bi xoa roi. Nen se check ra firstbyte = 0 va length = 0.
                * Khi do check alarm se khong bao alarm
*/

uint8_t Modem_Check_Alarm (void)
{
    uint8_t         i = 0;
    uint16_t        PeriodTime = 0;
    uint32_t        sTimeSecond = 0;
    uint32_t        OltimeSecond = 0;  
    static          uint8_t Mark_Fisrt = 0;
    
    if (Mark_Fisrt == 0)
    {
        Mark_Fisrt = 1;
        //
    #ifdef MEMORY_ON_FLASH
        if (Flash_Read_Last_Record (sRecTSVH, &sLastPackInfor.Pulse_u32, &sLastPackInfor.sTime) == 0)  
            return 0;
    #else
        if (ExMem_Read_Last_Record (sRecTSVH, &sLastPackInfor.Pulse_u32, &sLastPackInfor.sTime) == 0)
            return 0;
    #endif
    } 
        
    OltimeSecond = HW_RTC_GetCalendarValue_Second(sLastPackInfor.sTime, 0);
    sTimeSecond = HW_RTC_GetCalendarValue_Second(sRTC, 0);
    
    if (OltimeSecond >= sTimeSecond)
        return 0;
    
    PeriodTime =  sTimeSecond - OltimeSecond;   //Tinh ra second
    // San luong ma cach ngay thi van thong bao alarm binh thuong
#ifdef LPTIM_ENCODER_MODE 
	sPulse.Quantity_i32 = Modem_Cacul_Quantity((sPulse.Number_u32 / 4), sLastPackInfor.Pulse_u32);
#else   
    sPulse.Quantity_i32 = Modem_Cacul_Quantity(sPulse.Number_u32, sLastPackInfor.Pulse_u32);
#endif
    sPulse.Flow_i32 = (sPulse.Quantity_i32 * 3600) / PeriodTime;    //Pulse per hour
    
    //so sanh voi cac threshold va Set flag mark alarm. 
    if (sPulse.Quantity_i32 > sMeterThreshold.PeakHigh) 
    {
        if (Modem_Handler_Alarm( _ALARM_PEAK_HIGH ) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event High Quantity=\r\n", 28);
    } else
        aMARK_ALARM_PENDING[ _ALARM_PEAK_HIGH ] = 0;
    //Peak Low
    if (sPulse.Quantity_i32 < sMeterThreshold.PeakLow)  
    {
        if (Modem_Handler_Alarm( _ALARM_PEAK_LOW ) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event Low Quantity=\r\n", 27);
    } else
        aMARK_ALARM_PENDING[_ALARM_PEAK_LOW] = 0;
    //Flow high
    if (sPulse.Flow_i32 > sMeterThreshold.FlowHigh) 
    {
        if (Modem_Handler_Alarm( _ALARM_FLOW_HIGH ) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event High Flow=\r\n", 24);
    } else
        aMARK_ALARM_PENDING[_ALARM_FLOW_HIGH] = 0;
    //Flow low
    if (sPulse.Flow_i32 < sMeterThreshold.FlowLow)  //   
    {
        if (Modem_Handler_Alarm( _ALARM_FLOW_LOW ) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event Low Flow=\r\n", 23);
    } else
        aMARK_ALARM_PENDING[_ALARM_FLOW_LOW] = 0;
    //Check Vbat threshold
    if (sBattery.Level_u16 < sMeterThreshold.LowBatery) 
    {
        if (Modem_Handler_Alarm(_ALARM_VBAT_LOW) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event Low Battery=\r\n", 26);
    } else
        aMARK_ALARM_PENDING[_ALARM_VBAT_LOW] = 0;
    
    //Check chan Detect Cut  
    sModem.DetectCutStatus_u8 = HAL_GPIO_ReadPin(CYBLE_DTC_GPIO_Port, CYBLE_DTC_Pin);
    if (sModem.DetectCutStatus_u8 == 1) 
    {
        if (Modem_Handler_Alarm( _ALARM_DETECT_CUT ) == TRUE)
            UTIL_Log((uint8_t*) "=Mark Event Cyble DTC=\r\n", 24);
    } else
        aMARK_ALARM_PENDING[_ALARM_DETECT_CUT] = 0;
    
    //Check cac mark xem return 1 hay 0
    for (i = 0; i < _ALARM_END;i++)
        if (aMARK_ALARM[i] == 1)
            return 1;
    
    return 0;
}

/*
    Func: handler when check Alarm true
        + Neu truoc do da canh bao -> Sau MAX_ALARM_CONTINUOUS lan ms dc canh bao lai
        + Neu truoc do chua co canh bao: Mark alarm
*/
uint8_t Modem_Handler_Alarm (uint8_t Index)
{
    uint8_t Result = 0;
    
    if (aMARK_ALARM_PENDING[Index] == 0)
    {
        aMARK_ALARM[Index] = TRUE;
        aMARK_ALARM_PENDING[Index]++;
        Result = TRUE;            
    } else
    {
        aMARK_ALARM[Index] = FALSE;
        aMARK_ALARM_PENDING[Index]++;    //Neu alarm tiep tuc dien ra MAX_ALARM_CONTINUOUS chu ki -> Cho phep alarm lai
        if (aMARK_ALARM_PENDING[Index] >= MAX_ALARM_CONTINUOUS)
            aMARK_ALARM_PENDING[Index] = 0;
        
        Result = FALSE;
    }
    
    return Result;
}



uint16_t AppWm_Get_VBat(void)
{
    //Get batterry
    sBattery.mVol_u32 = 2 * HW_GetBatteryLevel();
    sBattery.Level_u16 = Get_Level_Voltage (sBattery.mVol_u32, VDD_INTERNAL_MAX, VDD_INTERNAL_MIN); 

	return sBattery.Level_u16;
}

uint16_t AppWm_Get_VOut(void)
{


	return 1;
}



void AppWm_Init (void)
{
    //Init Pulse in mem
    AppWm_Init_Pulse_Infor();
    //Init threshold
    AppWm_Init_Thresh_Measure();
//    AppWm_Init_Level_Calib();
}



void AppWm_Init_Pulse_Infor (void)
{
//	//Read Mark Soft Reset: add 0x2000FC40
//	__asm("ldr r0, = 0x2000F840");
//	__asm("ldr r2, [r0, 0]");
//
//	__asm("ldr r0, = pMarkSoftResetMCU");
//	__asm("ldr r1, [r0, 0]");
//	__asm("str r2, [r1]");
//
//	UTIL_Printf( (uint8_t*) "== Mark Reset: ", 15 );
//
//	UTIL_Print_Number(sModem.MarkResetChip_u8);
//
//	UTIL_Printf( (uint8_t*) "\r\n", 2 );
//
//	if (sModem.MarkResetChip_u8 != 0xA5A5A5A5)
//	{
//		//
//		UTIL_Log((uint8_t*) "Modem: Hard reset MCU\r\n", 23);
//		sPulse.Number_u32 = 0;
//        sMQTT.aMARK_MESS_PENDING[SEND_HARDTS_MCU] = TRUE;
//	} else
//	{
//		UTIL_Log((uint8_t*) "Modem: Get Pulse by RAM\r\n", 25);
//		//Get Pulse from RAM
//		__asm("ldr r0, = 0x2000F84A");
//		__asm("ldr r2, [r0, 0]");
//
//		__asm("ldr r0, = pPulseLPTIM");
//		__asm("ldr r1, [r0, 0]");
//		__asm("str r2, [r1]");
//
//		//Xoa danh dau add 0x2000FC40
//		sModem.MarkResetChip_u8 = 0;
//
//		__asm("ldr r0, = pMarkSoftResetMCU");
//		__asm("ldr r1, [r0, 0]");
//		__asm("ldr r2, [r1, 0]");
//
//		__asm("ldr r0, = 0x2000F840");
//		__asm("str r2, [r0]");
//	}

//	sPulse.NumberStartInit_u32 = sPulse.Number_u32;   //Using in Counter mode.
}





void AppWm_Init_Thresh_Measure (void)
{
#ifdef MEMORY_ON_FLASH
    uint8_t 	temp = 0xFF;
	uint8_t		Buff_temp[16] = {0};
      
	// Doc gia tri cau hinh threshold. Dung de check cac alarm
    temp = *(__IO uint8_t*) ADDR_THRESH_MEAS;    //2 byte
	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
    {
        OnchipFlashReadData(ADDR_THRESH_MEAS, &Buff_temp[0], 16);
        //
        sMeterThreshold.FlowHigh    = (Buff_temp[2] << 8) | Buff_temp[3];
        sMeterThreshold.FlowLow     = (Buff_temp[4] << 8) | Buff_temp[5];
        
        sMeterThreshold.PeakHigh    = (Buff_temp[6] << 8) | Buff_temp[7];
        sMeterThreshold.PeakLow     = (Buff_temp[8] << 8) | Buff_temp[9];
        
        sMeterThreshold.LowBatery   = Buff_temp[10];
        
        sMeterThreshold.LevelHigh   = (Buff_temp[11] << 8) | Buff_temp[12];
        sMeterThreshold.LevelLow    = (Buff_temp[13] << 8) | Buff_temp[14];
    } else
    {
        AppWm_Save_Thresh_Measure();
    }
#else
    uint8_t Buff_temp[40] = {0};
    
    //Peak High
    UTIL_MEM_set(Buff_temp, CAT_BYTE_EMPTY, sizeof(Buff_temp));
    
    CAT24Mxx_Read_Array(CAT_ADDR_THRESH_MEAS, Buff_temp, 40);
    if ((Buff_temp[0] != CAT_BYTE_EMPTY) && (Buff_temp[1] != CAT_BYTE_EMPTY))
    {
        sMeterThreshold.FlowHigh    = (Buff_temp[2] << 8) | Buff_temp[3];
        sMeterThreshold.FlowLow     = (Buff_temp[4] << 8) | Buff_temp[5];
        
        sMeterThreshold.PeakHigh    = (Buff_temp[6] << 8) | Buff_temp[7];
        sMeterThreshold.PeakLow     = (Buff_temp[8] << 8) | Buff_temp[9];
        
        sMeterThreshold.LowBatery   = Buff_temp[10];
        
        sMeterThreshold.LevelHigh   = (Buff_temp[11] << 8) | Buff_temp[12];
        sMeterThreshold.LevelLow    = (Buff_temp[13] << 8) | Buff_temp[14];
    } else
    {
        AppWm_Save_Thresh_Measure();
    }  
#endif
    
}


void AppWm_Save_Thresh_Measure (void)
{
    uint8_t aTEMP_THRESH[24] = {0};
    
    aTEMP_THRESH[0] = BYTE_TEMP_FIRST;
    aTEMP_THRESH[1] = 13;

    aTEMP_THRESH[2] = (sMeterThreshold.FlowHigh >> 8) & 0xFF;
    aTEMP_THRESH[3] = sMeterThreshold.FlowHigh & 0xFF;
    
    aTEMP_THRESH[4] = (sMeterThreshold.FlowLow >> 8) & 0xFF;
    aTEMP_THRESH[5] = sMeterThreshold.FlowLow & 0xFF;
     
    aTEMP_THRESH[6] = (sMeterThreshold.PeakHigh >> 8) & 0xFF;  
    aTEMP_THRESH[7] = sMeterThreshold.PeakHigh & 0xFF;
    
    aTEMP_THRESH[8] = (sMeterThreshold.PeakLow >> 8) & 0xFF;
    aTEMP_THRESH[9] = sMeterThreshold.PeakLow & 0xFF;
    
    aTEMP_THRESH[10] = sMeterThreshold.LowBatery;
    
    aTEMP_THRESH[11] = (sMeterThreshold.LevelHigh >> 8) & 0xFF;
    aTEMP_THRESH[12] = sMeterThreshold.LevelHigh & 0xFF;
    
    aTEMP_THRESH[13] = (sMeterThreshold.LevelLow >> 8) & 0xFF;
    aTEMP_THRESH[14] = sMeterThreshold.LevelLow & 0xFF;

#ifdef MEMORY_ON_FLASH
    OnchipFlashPageErase(ADDR_THRESH_MEAS);
    OnchipFlashWriteData(ADDR_THRESH_MEAS, &aTEMP_THRESH[0], 16);
#else
    CAT24Mxx_Write_Array(CAT_ADDR_THRESH_MEAS, &aTEMP_THRESH[0], 16);
#endif
}







