

#include "user_app_temh.h"
#include "user_define.h"

#include "user_modbus_rtu.h"
#include "user_sv_temphumi.h"

/*================ Define =================*/

/*============= Function static  ===========*/
static uint8_t _Cb_Temh_Log_TSVH (uint8_t event);
static uint8_t _Cb_Temh_Control_Led1 (uint8_t event);
static uint8_t _Cb_Temh_Entry (uint8_t event);
static uint8_t _Cb_Test_RS485 (uint8_t event);
static uint8_t _Cb_Temh_Read_Value (uint8_t event);
static uint8_t _Cb_Power_On_Temh (uint8_t event);
static uint8_t _Cb_Temh_Control_Led2(uint8_t event);
static uint8_t _Cb_Temh_Control_Led3(uint8_t event);

/*================ Struct =================*/
sEvent_struct sEventAppTempH [] =
{
    { _EVENT_TEMH_ENTRY, 		0, 0, 0, 	    _Cb_Temh_Entry },
    
    { _EVENT_POWER_ON_NODE, 	0, 0, 2000, 	_Cb_Power_On_Temh },
    { _EVENT_TEMH_READ_VALUE, 	0, 0, 500, 	    _Cb_Temh_Read_Value },
    { _EVENT_TEMH_LOG_TSVH, 	0, 0, 2000, 	_Cb_Temh_Log_TSVH },
    { _EVENT_CONTROL_LED1,		1, 0, 200,      _Cb_Temh_Control_Led1 }, 
    { _EVENT_CONTROL_LED2,		1, 0, 500,      _Cb_Temh_Control_Led2 }, 
    { _EVENT_CONTROL_LED3,		1, 0, 500,      _Cb_Temh_Control_Led3 }, 
    
    { _EVENT_TEST_RS485,		1, 0, 250,     _Cb_Test_RS485 }, 
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
    .aSlaveID = {SLAVE_ID_DEFAULT, SLAVE_ID_DEFAULT},
    .NumSlave_u8 = NUM_SLAVE_DEFAULT,
    .CountFailGetTemh_u8 = 1,
};


uint8_t aUART_485_DATA [64];
sData   sUart485 = {(uint8_t *) &aUART_485_DATA[0], 0};

static GPIO_TypeDef*  LED_PORT[3] = {LED_1_GPIO_Port, LED_2_GPIO_Port, LED_3_GPIO_Port};
static const uint16_t LED_PIN[3] = {LED_1_Pin, LED_2_Pin, LED_3_Pin};

/*================ Struct =================*/

static uint8_t _Cb_Temh_Entry (uint8_t event)
{
//    fevent_active(sEventAppTempH, _EVENT_POWER_ON_NODE);

	return 1;
}


static uint8_t _Cb_Power_On_Temh (uint8_t event)
{
    static uint8_t Step = 0;
    
    switch (Step)
    {
        case 0: //Power off
            if (sTempHumi.PowerStatus_u8 == true)
            {
                Step = 1;  //ti nua ++ de qua buoc off
            }
            break;
        case 1:
            UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: power on again!" );
            HAL_GPIO_WritePin(GPIOC, ON_OFF_12V_Pin, GPIO_PIN_RESET);
            break;
        default: //Power on and start measure
            HAL_GPIO_WritePin(GPIOC, ON_OFF_12V_Pin, GPIO_PIN_SET);
            sTempHumi.PowerStatus_u8 = true;
            Step = 0;
            fevent_active(sEventAppTempH, _EVENT_TEMH_READ_VALUE);
            return 1;
    }
    
    Step++;
    fevent_enable(sEventAppTempH, event);
    
	return 1;
}


static uint8_t _Cb_Temh_Read_Value (uint8_t event)
{
    static uint8_t  Step = 0;
    static uint8_t  Retry = 0;
    
    static structFloatValue  sTempTemperature[5] = {0};
    static structFloatValue  sTempHumidity[5] = {0};
    int32_t ValueTotal = 0;
    static uint8_t Sample_u8 = 0;
    
    uint8_t IsFinish_u8 = false;
    
    static uint8_t IndexSlave = 0;
    
    if (Retry < 3)
    {
        switch (Step)
        {
            case 0:
                AppTemH_485_Read_Value(sTempHumi.aSlaveID[IndexSlave], &AppTemH_Clear_Before_Recv);

                fevent_enable(sEventAppTempH, event);
                Step++;
                break;
            case 1:
                Retry++;
                if (sTempHumi.ModBusStatus_u8 == true)
                {
                    UTIL_MEM_set ( (uint8_t *) &sTempTemperature[Sample_u8], 0, sizeof (structFloatValue));   
                    UTIL_MEM_set ( (uint8_t *) &sTempHumidity[Sample_u8], 0, sizeof (structFloatValue));  
                        
                    if (AppTemH_Extract_Data (sTempHumi.aSlaveID[IndexSlave], sUart485.Data_a8, sUart485.Length_u16, &sTempTemperature[Sample_u8], &sTempHumidity[Sample_u8]) == true)
                    {
                        UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_temh: read tempe humi ok!");
                                              
                        Retry=0;
                        Sample_u8++;
                        
                        if (Sample_u8 >= 3)
                        {
                            //Get Value
                            ValueTotal = sTempTemperature[0].Val_i16 + sTempTemperature[1].Val_i16 + sTempTemperature[2].Val_i16; 
                            
                            sTempHumi.sTemperature[IndexSlave].Val_i16 = ValueTotal/3;     
                            sTempHumi.sTemperature[IndexSlave].Scale_u8 = sTempTemperature[0].Scale_u8;
                            sTempHumi.sTemperature[IndexSlave].Unit_u16 = sTempTemperature[0].Unit_u16;
                            
                            ValueTotal = sTempHumidity[0].Val_i16 + sTempHumidity[1].Val_i16 + sTempHumidity[2].Val_i16;
                            
                            sTempHumi.sHumidity[IndexSlave].Val_i16 = ValueTotal/3; 
                            sTempHumi.sHumidity[IndexSlave].Scale_u8 = sTempHumidity[0].Scale_u8;
                            sTempHumi.sHumidity[IndexSlave].Unit_u16 = sTempHumidity[0].Unit_u16;
                            
                            UTIL_Printf_Str(DBLEVEL_M,  "\r\nu_app_temh: temp average: " );
                            UTIL_Printf_Dec(DBLEVEL_M, sTempHumi.sTemperature[IndexSlave].Val_i16);
                            UTIL_Printf_Str(DBLEVEL_M, "\r\nu_app_temh: humi average: " );
                            UTIL_Printf_Dec(DBLEVEL_M, sTempHumi.sHumidity[IndexSlave].Val_i16);
    
                            sTempHumi.Status_u8[IndexSlave] = TRUE;
                            IsFinish_u8 = TRUE;
                            
                            break;
                        }
                    }
                } else
                {
                    //Init Uart again
                    MX_USART1_UART_Init();
                    Init_Uart_485_Rx_IT();
                }
                                      
                fevent_enable(sEventAppTempH, event);
                Step = 0;
                break;
            default:
                Step = 0;
                break;
        }
    } else
    {
        if (sTempHumi.ModBusStatus_u8 == true)
        {
            UTIL_Log_Str (DBLEVEL_M, "u_app_temh: read temh fail format!\r\n" );
            UTIL_Log (DBLEVEL_M, sUart485.Data_a8, sUart485.Length_u16 );
        } else
        {
            UTIL_Log_Str (DBLEVEL_M, "u_app_temh: read temh no response!\r\n" );
        }
            
        sTempHumi.ModBusStatus_u8 = false; 
        sTempHumi.Status_u8[IndexSlave] = false;
        IsFinish_u8 = true;
    }
                                      
    if (IsFinish_u8 == true)
    {
        //reset static var
        Step = 0;
        Sample_u8 = 0;
        Retry = 0;

        //Tang index slave request
        IndexSlave++;
        if (IndexSlave < sTempHumi.NumSlave_u8)
        {
            fevent_enable(sEventAppTempH, event);
        } else
        {         
            IndexSlave = 0;
            sTempHumi.PowerStatus_u8 = AppTemh_Check_Status_TempH();            
            fevent_enable(sEventAppTempH, _EVENT_TEMH_LOG_TSVH);
        }
    }
                                      
    
	return 1;
}


static uint8_t _Cb_Temh_Log_TSVH (uint8_t event)
{
	//Log Data meter to flash
    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: packet tsvh!\r\n" );
    //
    Get_RTC();
         
	AppTemH_Get_VBAT_mV();
    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: dien ap pin trong: " );
    UTIL_Printf_Dec(DBLEVEL_M, sBattery.mVol_u32);
    UTIL_Printf_Str(DBLEVEL_M, " mV\r\n" );
    
    AppTemH_Get_Vout_mV();
    //Print to debug
    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: dien ap pin ngoai: " );
    UTIL_Printf_Dec(DBLEVEL_M, sVout.mVol_u32);
    UTIL_Printf_Str(DBLEVEL_M, " mV\r\n" );
 
    //Record TSVH   
    AppTemH_Log_Data_TSVH();    
        
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
uint8_t AppTemH_Set_Mode_Led (void)
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
static uint8_t _Cb_Temh_Control_Led1(uint8_t event)
{   
    //Handler in step control one
    switch ( AppTemH_Set_Mode_Led() )
    {
        case _LED_MODE_ONLINE_INIT:
            sEventAppTempH[event].e_period = 1000;
            LED_Toggle (_LED_STATUS);
            break;
        case _LED_MODE_CONNECT_SERVER:
            if (sEventAppTempH[event].e_period != 430)
            {
                sEventAppTempH[event].e_period = 430;
                LED_Off (_LED_STATUS);
            } else
            {
                sEventAppTempH[event].e_period = 70;
                LED_On (_LED_STATUS);
            }
            break;
        case _LED_MODE_UPDATE_FW:
            sEventAppTempH[event].e_period = 100;
            LED_Toggle (_LED_STATUS);
            break;      
        case _LED_MODE_POWER_SAVE:
            LED_Off (_LED_STATUS);
            break;
       
        default:
            break;
    }
       
    fevent_enable(sEventAppTempH, event);
    
    return 1;
}

  
static uint8_t _Cb_Temh_Control_Led2(uint8_t event)
{  

#ifdef USING_APP_SIM
    if (sSimCommon.sGPS.Status_u8 == true)
    {
        LED_Toggle (_LED_GPS);
    } else if (sSimCommon.sGPS.Status_u8  == error)
    {
        LED_Off (_LED_GPS);
    } 
#endif
    
    fevent_enable(sEventAppTempH, event);
    
    return 1;
}

static uint8_t _Cb_Temh_Control_Led3(uint8_t event)
{  
    if (sTempHumi.CountFailGetTemh_u8 == 0)
    {
        LED_Toggle (_LED_TEMH);
    } else
    {
        LED_Off (_LED_TEMH);
    } 
    
    fevent_enable(sEventAppTempH, event);
    
    return 1;
}



uint32_t count_error=0;
uint32_t count_send=0;


static uint8_t _Cb_Test_RS485 (uint8_t event)
{  
    static uint8_t step = 0;
    uint8_t aTMEP[8] = { 0x01, 0x06, 0x03, 0xE8, 0x00, 0x00, 0x09, 0xBA};
    
    switch (step)
    {
        case 0:
            HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_SET);  
            HAL_Delay(10);
            
            Reset_Buff(&sUart485);
            sTempHumi.ModBusStatus_u8 = false;
            
            HAL_UART_Transmit(&UART_485, &aTMEP[0], 8, 1000); 
            //Dua DE ve Receive
            HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_RESET);
            count_send++;
            step++;
            break;
        case 1:
            if (sTempHumi.ModBusStatus_u8 == true)
            {
                if (sUart485.Length_u16 == 8)
                {
                    UTIL_Printf_Str(DBLEVEL_M, "\r\n485 OK: "); 
                    UTIL_Printf_Hex(DBLEVEL_M, sUart485.Data_a8, sUart485.Length_u16); 
                    UTIL_Printf_Str(DBLEVEL_M, "\r\n"); 
                } else
                {
                    UTIL_Printf_Str(DBLEVEL_M, "\r\n485 Fail: "); 
                    UTIL_Printf_Hex(DBLEVEL_M, sUart485.Data_a8, sUart485.Length_u16); 
                    UTIL_Printf_Str(DBLEVEL_M, "\r\n"); 
                }
                if(sUart485.Length_u16 < 8)
                {
                    count_error++;
                }
            }
            
            step = 0;
            break;
        default:
            break;
        
    }
    
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
            if ((i != _EVENT_CONTROL_LED1) && (i != _EVENT_CONTROL_LED2) && (i != _EVENT_CONTROL_LED3) )
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


uint16_t AppTemH_Get_VBAT_mV(void)
{
    //Get batterry
    sBattery.mVol_u32 = Get_Value_ADC (ADC_CHANNEL_14);
    sBattery.mVol_u32 *= 3;   //Phan ap chia 2
    sBattery.Level_u16 = Get_Level_Voltage (sBattery.mVol_u32, VDD_BAT, VDD_MIN); 

	return sBattery.Level_u16;
}

uint16_t AppTemH_Get_Vout_mV(void)
{
    AdcInitialized = 0; 
    sVout.mVol_u32 = Get_Value_ADC(ADC_CHANNEL_7); 
    sVout.mVol_u32 = sVout.mVol_u32 * 247 / 47 + 600;  //qua 2 diode
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
       
    if (sTempHumi.Status_u8[0] == true)
    {        
        //----------Nhiet do--------------------
        pData[length++] = OBIS_ENVI_TEMP_1; // Dien ap pin   
        pData[length++] = 0x02;
        pData[length++] = (sTempHumi.sTemperature[0].Val_i16)>>8;
        pData[length++] = (sTempHumi.sTemperature[0].Val_i16);
        pData[length++] = sTempHumi.sTemperature[0].Scale_u8;
        
        //----------do am--------------------
        pData[length++] = OBIS_ENVI_HUMI_1; // Dien ap pin
        pData[length++] = 0x02;
        pData[length++] = (sTempHumi.sHumidity[0].Val_i16)>>8;
        pData[length++] = (sTempHumi.sHumidity[0].Val_i16);
        pData[length++] = sTempHumi.sHumidity[0].Scale_u8;
    }
        
    if (sTempHumi.Status_u8[1] == true)
    {
        //----------Nhiet do--------------------
        pData[length++] = OBIS_ENVI_TEMP_2; // Dien ap pin   
        pData[length++] = 0x02;
        pData[length++] = (sTempHumi.sTemperature[1].Val_i16)>>8;
        pData[length++] = (sTempHumi.sTemperature[1].Val_i16);
        pData[length++] = sTempHumi.sTemperature[1].Scale_u8;
        
        //----------do am--------------------
        pData[length++] = OBIS_ENVI_HUMI_2; // Dien ap pin
        pData[length++] = 0x02;
        pData[length++] = (sTempHumi.sHumidity[1].Val_i16)>>8;
        pData[length++] = (sTempHumi.sHumidity[1].Val_i16);
        pData[length++] = sTempHumi.sHumidity[1].Scale_u8;
    }
    
    //----------Dong dien --------------------
    pData[length++] = OBIS_EMET_CUR; // Dien ap pin
    pData[length++] = 0x02;
    pData[length++] = (sTempHumi.sCurrent.Val_i16)>>8;
    pData[length++] = (sTempHumi.sCurrent.Val_i16);
    pData[length++] = 0xFD;
    
    //----------Toa do GPS --------------------
#ifdef USING_APP_SIM
//    if (sSimCommon.sGPS.Status_u8 == true)
//    {
        if ( (sSimCommon.sGPS.LengData_u8 != 0) && (sSimCommon.sGPS.LengData_u8 < 30) )
        {
            pData[length++] = OBIS_GPS_LOC;  
            pData[length++] = sSimCommon.sGPS.LengData_u8;
            
            for (i = 0; i < sSimCommon.sGPS.LengData_u8; i++)
                pData[length++] = sSimCommon.sGPS.aPOS_INFOR[i]; 
        }
//    }
#endif

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

void AppTemH_Log_Data_Event (void)
{
    uint8_t     aMessData[64] = {0};
    uint8_t     Length = 0;
    
    if (sRTC.year <= 20)
        return;
    
    Length = Modem_Packet_Event (&aMessData[0]);
    AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_EVENT_A, &aMessData[0], Length);
}

/*
    Func: Log Data TSVH
        + Packet Data TSVH
        + Save to Flash or ExMem
*/

void AppTemH_Log_Data_TSVH (void)
{
    uint8_t     aMessData[256] = {0};
    uint8_t     Length = 0;
    
//    if (sRTC.year <= 20)
//        return;
    
    Length = AppTemH_Packet_TSVH (&aMessData[0]);
    AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_A, &aMessData[0], Length);
}



void AppTemH_Init_Thresh_Measure (void)
{
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
        AppTemH_Save_Thresh_Measure();
    }    
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


    OnchipFlashPageErase(ADDR_THRESH_MEAS);
    OnchipFlashWriteData(ADDR_THRESH_MEAS, &aTEMP_THRESH[0], 16);
}

	 
void AppTemH_Init_Slave_ID (void)
{
    uint8_t 	temp = 0xFF;
    //Doc List Slave ra
    temp = *(__IO uint8_t*) ADDR_SLAVE_ID;  
	if (temp != FLASH_BYTE_EMPTY) 
	{
        sTempHumi.NumSlave_u8 = *(__IO uint8_t*) (ADDR_SLAVE_ID + 2); 
        
        if (sTempHumi.NumSlave_u8 >= MAX_SLAVE) 
            sTempHumi.NumSlave_u8 = MAX_SLAVE;
        
        OnchipFlashReadData (ADDR_SLAVE_ID + 3, &sTempHumi.aSlaveID[0], sTempHumi.NumSlave_u8);
    } else
    {
        AppTemH_Save_Slave_ID();
    }  
    
}

void AppTemH_Save_Slave_ID (void)
{
    uint8_t aTEMP[64] = {0};
    uint8_t Count = 0;
    
    aTEMP[Count++] = BYTE_TEMP_FIRST;
    aTEMP[Count++] = sTempHumi.NumSlave_u8 + 1;
    aTEMP[Count++] = sTempHumi.NumSlave_u8;
        
    for (uint8_t i = 0; i < sTempHumi.NumSlave_u8; i++)
    {
        aTEMP[Count++] = sTempHumi.aSlaveID[i];
    }
    
    OnchipFlashPageErase(ADDR_SLAVE_ID);
    OnchipFlashWriteData(ADDR_SLAVE_ID, &aTEMP[0], 64);
}


void AppTemH_Init (void)
{
    AppTemH_Init_Thresh_Measure();
    AppTemH_Init_Slave_ID();
    //On power to 485 and tempetature 
    V12_IN_ON;
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


void AppTemH_Clear_Before_Recv (void)
{
    Reset_Buff(&sUart485);
}

/*
    Fun:extract data Sao Viet TEMP humi
               Regis             Parameter
        Read:
                0                   slave ID
                1                   Baurate
                2                   Temp Value
                3                   Temp Unit
                4                   Temp Decimal
                5                   Humi Value
                6                   Humi Unit
                7                   Humi Decimal

        write   0                   Slave ID
                1                   Baurate
*/

uint8_t AppTemH_Extract_Data (uint8_t SlaveID, uint8_t *pSource, uint16_t Length, structFloatValue *sTemperature, structFloatValue *sHumi)
{
    uint16_t crc_calcu = 0;
    uint8_t Crc_Cut[2] = {0};
    uint16_t Pos = 0;
    uint8_t FuncCode = 0;
    uint8_t LengthData = 0;
    uint8_t SlaveIDGet = 0;
    uint16_t TempValue = 0;
    
    if (Length < 4)
        return FALSE;
    
    crc_calcu = ModRTU_CRC(pSource, Length - 2);
    
    Crc_Cut[0] = (uint8_t) (crc_calcu & 0x00FF);
    Crc_Cut[1] = (uint8_t) ( (crc_calcu >> 8) & 0x00FF );
    
    if ( (Crc_Cut[0] != *(pSource + Length - 2)) || (Crc_Cut[1] != *(pSource + Length - 1)) )
        return FALSE;
    
    SlaveIDGet = *(pSource + Pos++);
    FuncCode = *(pSource + Pos++);
    
    LengthData = *(pSource + Pos++);
    //check frame
    if ( (SlaveIDGet != SlaveID) || (FuncCode != FUN_READ_BYTE) || (LengthData != (NUM_REGISTER_READ * 2)))
        return FALSE;
       
    //Get data begin Unit: skip 2byte slaveID, 2byte baurate   
    //Nhiet do
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sTemperature->Val_i16 = (int16_t) TempValue;
    
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sTemperature->Unit_u16 = TempValue;
    
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sTemperature->Scale_u8 = (256 - TempValue) % 256;
    
    //Do am
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sHumi->Val_i16 = (int16_t) TempValue;
    
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sHumi->Unit_u16 = TempValue;
    
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    sHumi->Scale_u8 = (256 - TempValue) % 256;
    
    return TRUE;
}

/*
    Func: Check thu thap Temh co day du k?
        + Neu qua 10 lan: Reset cac node temp humi: Poweroff -> On again
*/

uint8_t AppTemh_Check_Status_TempH (void)
{
    uint8_t i = 0;
    uint8_t Result = true;
    
    //Check status
    for ( i = 0; i < sTempHumi.NumSlave_u8; i++)
    {
        if (sTempHumi.Status_u8[i] == false)
            break;         
    }
    //Neu tat ca deu true: i = sTempHumi.NumSlave_u8
    if (i == sTempHumi.NumSlave_u8)
    {
        //tat ca status deu true:
        sTempHumi.CountFailGetTemh_u8 = 0;
    } else
    {
        sTempHumi.CountFailGetTemh_u8++;
        if (sTempHumi.CountFailGetTemh_u8 >= MAX_GET_TEMH_FAIL)
        {
            sTempHumi.CountFailGetTemh_u8 = 0;

            Result = false;
        }
    }
    
    return Result;
}




void LED_Toggle (Led_TypeDef Led)
{
    HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}


void LED_On (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}



void LED_Off (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}



