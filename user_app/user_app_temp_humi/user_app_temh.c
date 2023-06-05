

#include "user_app_temh.h"
#include "user_define.h"

#include "user_external_mem.h"
#include "user_internal_mem.h"

#include "user_modbus_rtu.h"
#include "user_sv_temphumi.h"

/*================ Define =================*/

/*================ Struct =================*/
sEvent_struct sEventAppTempH [] =
{
    { _EVENT_ENTRY_TEMH, 		0, 0, 0, 	    _Cb_Entry_TemH },
    { _EVENT_LOG_TSVH, 		    0, 0, 500, 	    _Cb_Log_TSVH },
    { _EVENT_CONTROL_LED1,		1, 0, 200,      _Cb_Control_Led1 }, 
    
    { _EVENT_TEST_RS485,		1, 0, 1000,     _Cb_Test_RS485 }, 
};
            
    
Struct_Battery_Status    sBattery;
Struct_Battery_Status    sVout;

struct_ThresholdConfig   sMeterThreshold =
{
    .FlowHigh = 0xFFFF,
    .FlowLow = 0,
    .PeakHigh = 0xFFFF,
    .PeakLow = 0,
    .LowBatery = 10,
    .LevelHigh = 4000,
    .LevelLow  = 10,
};

uint8_t aMARK_ALARM[10];
uint8_t aMARK_ALARM_PENDING[10];


STempHumiVariable         sTempHumi = 
{
    .SlaveID_u8 = SLAVE_ID_DEFAULT,
};


/*================ Struct =================*/

uint8_t _Cb_Entry_TemH (uint8_t event)
{
    fevent_active(sEventAppTempH, _EVENT_LOG_TSVH);

	return 1;
}


uint8_t _Cb_Log_TSVH (uint8_t event)
{
	//Log Data meter to flash
    PrintDebug(&uart_debug, (uint8_t*) "=Log TSVH data=\r\n", 17, 1000);
    //
    Get_RTC();
         
//	Get_VBAT_mV();
//    PrintDebug(&uart_debug, (uint8_t*) "=Battery Voltage: ", 18, 1000);
//    UTIL_Print_Number(sBattery.mVol_u32);
//    PrintDebug(&uart_debug, (uint8_t*) " mV=\r\n", 6, 1000);
//    
//    Get_Vout_mV();
//    //Print to debug
//    PrintDebug(&uart_debug, (uint8_t*) "=OUT Voltage: ", 14, 1000);
//    UTIL_Print_Number(sVout.mVol_u32);
//    PrintDebug(&uart_debug, (uint8_t*) " mV=\r\n", 6, 1000);
 
    //Record TSVH
    AppTemH_Log_Data_TSVH(&sRecTSVH);          
    //Mark flash mess after sleep
    #ifdef USING_APP_SIM
        sMQTT.aMARK_MESS_PENDING[DATA_TSVH_OPERA] = TRUE;
        sMQTT.aMARK_MESS_PENDING[SEND_EVENT_MESS] = TRUE;
    #endif
    
	return 1;
}

/*
    Func: Set Mode Control Led
        - Mode: + _LED_MODE_ONLINE_INIT     : Nhay deu 1s
                + _LED_MODE_CONNECT_SERVER  : Nhay Duty: 430ms off, 70ms on
                + _LED_MODE_UPDATE_FW       : Nhay deu 100ms
                + _LED_MODE_POWER_SAVE      : Off led
                + _LED_MODE_TEST_PULSE      : Nhay theo xung doc vao
*/
uint8_t Modem_Set_Mode_Led (void)
{
    uint8_t Result = 0;
    
    #ifdef USING_APP_SIM
        if ((sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE) && (sModem.rTestPulse_u8 == FALSE)) 
        {
            if (sEventAppSim[_EVENT_SIM_SEND_MESS].e_status == TRUE)
                Result = _LED_MODE_CONNECT_SERVER;
            else
                Result = _LED_MODE_ONLINE_INIT;
        } else if ((sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_MAIN) && (sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_BACKUP))  
        {
            Result = _LED_MODE_UPDATE_FW;
        } else if (sModem.rTestPulse_u8 == FALSE)
        {
            Result = _LED_MODE_POWER_SAVE; 
        } else
            Result = _LED_MODE_TEST_PULSE;
    #else
        if ((sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE) && (sModem.rTestPulse_u8 == FALSE)) 
        {
            if (sEventAppSim[_EVENT_SIM_SEND_MESS].e_status == TRUE)
                Result = _LED_MODE_CONNECT_SERVER;
            else
                Result = _LED_MODE_ONLINE_INIT;
        } else if (sModem.rTestPulse_u8 == FALSE)
        {
            Result = _LED_MODE_POWER_SAVE; 
        } else
            Result = _LED_MODE_TEST_PULSE;  
    #endif
        
    return Result;
}

/*
    Func: CB Event Control Led
*/
uint8_t _Cb_Control_Led1(uint8_t event)
{   
    //Handler in step control one
    switch ( Modem_Set_Mode_Led() )
    {
        case _LED_MODE_ONLINE_INIT:
            sEventAppTempH[event].e_period = 1000;
            HAL_GPIO_TogglePin(LED_SIM_GPIO_Port, LED_SIM_Pin);
            break;
        case _LED_MODE_CONNECT_SERVER:
            if (sEventAppTempH[event].e_period != 430)
            {
                sEventAppTempH[event].e_period = 430;
                HAL_GPIO_WritePin(LED_SIM_GPIO_Port, LED_SIM_Pin, GPIO_PIN_RESET);
            } else
            {
                sEventAppTempH[event].e_period = 70;
                HAL_GPIO_WritePin(LED_SIM_GPIO_Port, LED_SIM_Pin, GPIO_PIN_SET);
            }
            break;
        case _LED_MODE_UPDATE_FW:
            sEventAppTempH[event].e_period = 100;
            HAL_GPIO_TogglePin(LED_SIM_GPIO_Port, LED_SIM_Pin);
            break;      
        case _LED_MODE_POWER_SAVE:
            HAL_GPIO_WritePin(LED_SIM_GPIO_Port, LED_SIM_Pin, GPIO_PIN_RESET);
            break;
       
        default:
            break;
    }
       
    fevent_enable(sEventAppTempH, event);
    
    return 1;
}


uint8_t _Cb_Test_RS485 (uint8_t event)
{  
    
    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_SET);  
    HAL_Delay(10);
    
    HAL_UART_Transmit(&UART_485, (uint8_t *)"Vo van chien\r\n", 14, 1000); 
    //Dua DE ve Receive
    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_RESET);
    
    fevent_enable(sEventAppTempH, event);

    return 1;
}

/*================ Function Handler =================*/

uint8_t AppTemH_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_END_TEMP_HUMI; i++)
	{
		if (sEventAppTempH[i].e_status == 1)
		{
            Result = true;

			if ((sEventAppTempH[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppTempH[i].e_systick)  >=  sEventAppTempH[i].e_period))
			{
                sEventAppTempH[i].e_status = 0;  //Disable event
				sEventAppTempH[i].e_systick = HAL_GetTick();
				sEventAppTempH[i].e_function_handler(i);
			}
		}
	}
    
	return Result;
}


uint16_t Get_VBAT_mV(void)
{
    //Get batterry
    sBattery.mVol_u32 = Get_Value_ADC (ADC_CHANNEL_15);
    sBattery.mVol_u32 *= 2;   //Phan ap chia 2
    sBattery.Level_u16 = Get_Level_Voltage (sBattery.mVol_u32, VDD_BAT, VDD_MIN); 

	return sBattery.Level_u16;
}

uint16_t Get_Vout_mV(void)
{
    AdcInitialized = 0; 
    sVout.mVol_u32 = Get_Value_ADC(ADC_CHANNEL_14);
    sVout.mVol_u32 = sVout.mVol_u32 * 247 / 47;
    sVout.Level_u16 = Get_Level_Voltage (sVout.mVol_u32, VDD_OUT_MAX, VDD_OUT_MIN); 
   
	return sVout.Level_u16;
}



uint8_t AppTemH_Packet_TSVH (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;   
    //----------------------- start send data below --------------------- //
    //----------sTime--------------------
    pData[length++] = OBIS_TIME_DEVICE;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
       
    //----------luu luong --------------------
    pData[length++] = OBIS_WM_FLOW;  // Luu luong
    pData[length++] = 0x02;
    pData[length++] = 0;
    pData[length++] = 0;
    pData[length++] = 0x00;    

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
    
    //Chăc chan vao day da co 1 alarm
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
                    break;
                case _ALARM_PEAK_LOW:
                case _ALARM_PEAK_HIGH:
                    break;
                case _ALARM_VBAT_LOW:
                    break;
                case _ALARM_DIR_CHANGE:
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
    Func: Log Data Event
        + Packet Event Data
        + Save to Flash or ExMem
*/

void Modem_Log_Data_Event (StructManageRecordFlash *sRecordEvent)
{
    uint8_t     aMessData[64] = {0};
    uint8_t     Length = 0;
    
    if (sRTC.year <= 20)
        return;
    
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

void AppTemH_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH)
{
    uint8_t     aMessData[128] = {0};
    uint8_t     Length = 0;
    
    if (sRTC.year <= 20)
        return;
    
    Length = AppTemH_Packet_TSVH (&aMessData[0]);
	
    //Luu vao Flash
    #ifdef MEMORY_ON_FLASH
        Flash_Save_Record (sRecordTSVH, &aMessData[0], Length);
    #else       
        ExMem_Save_Record (sRecordTSVH, &aMessData[0], Length);
    #endif
}



void AppTemH_Init_Thresh_Measure (void)
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
        AppTemH_Save_Thresh_Measure();
    }  
#endif
    
}



void AppTemH_Save_Thresh_Measure (void)
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



void AppTemH_Init (void)
{
    AppTemH_Init_Thresh_Measure();
}




/*
    FuncTest: Master Read
*/

void AppTemH_485_Read_Value (uint8_t SlaveID, void (*pFuncResetRecvData) (void)) 
{
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    ModRTU_Master_Read_Frame(&strFrame, SlaveID, FUN_READ_BYTE, REGIS_SLAVE_ID, NUM_REGISTER_READ);

    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_SET);  
    HAL_Delay(10);
    // Send
    pFuncResetRecvData();
    
    HAL_UART_Transmit(&UART_485, strFrame.Data_a8, strFrame.Length_u16, 1000); 
    //Dua DE ve Receive
    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_RESET);
}



