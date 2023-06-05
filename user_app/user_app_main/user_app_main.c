/*
 * user_app.c
 *
 *  Created on: Dec 14, 2021
 *      Author: Chien
 */

#include "user_app_main.h"
#include "user_define.h"

#include "user_uart.h"
#include "user_modem.h"
#include "user_modem_init.h"

#include "user_timer.h"
#include "user_lpm.h"
#include "gpio.h"
#include "i2c.h"
#include "adc.h"
/*==========Static func====================*/
static uint8_t _Cb_Timer_IRQ(uint8_t event);
static uint8_t _Cb_Uart_Debug_Process(uint8_t event);
static uint8_t _Cb_HandLer_IDLE(uint8_t event);
static uint8_t _Cb_Tx_Timer (uint8_t event);
static uint8_t _Cb_Set_RTC (uint8_t event);
static uint8_t _Cb_Read_Old_Record (uint8_t event);
static void Cb_TX_Timer_Event(void *context);
static uint8_t _Cb_Save_Box (uint8_t event);

static uint32_t  AppCom_Calcu_Period_To_RoudTime (uint32_t FreqWakeup);


/*=================Var struct======================*/
sEvent_struct sEventAppMain[] =
{
	{ _EVENT_TIMMER_IRQ, 		    0, 0, 0, 	    _Cb_Timer_IRQ },
	{ _EVENT_PROCESS_UART_DEBUG, 	0, 0, 5, 	    _Cb_Uart_Debug_Process },
    { _EVENT_SET_RTC, 				0, 0, 0, 	    _Cb_Set_RTC },
	{ _EVENT_IDLE, 					1, 0, 1000,     _Cb_HandLer_IDLE },
    { _EVENT_TX_TIMER,		        0, 0, 0, 	    _Cb_Tx_Timer },
    //Event Log
    { _EVENT_READ_OLD_RECORD,		0, 0, 2100,     _Cb_Read_Old_Record },
    { _EVENT_SAVE_BOX,		        0, 0, 0, 	    _Cb_Save_Box },  
};

/*===============================================*/
char aSaoVietCom[15][71] =
{
    {"       =======================================================       \r\n"},
    {"                                                                     \r\n"},
    {"* * *      * * *         *                 *         * * *      * * *\r\n"},
    {"*        *       *        *               *        *       *        *\r\n"},
    {"*        *                 *             *        *                 *\r\n"},
    {"          *                 *           *        *                   \r\n"},
    {"            *                *         *         *                   \r\n"},
    {"              *               *       *          *                   \r\n"},
    {"                *              *     *           *                   \r\n"},
    {"        *        *              *   *             *                  \r\n"},
    {"         *       *               * *               *       *         \r\n"},
    {"           * * *                  *           *      * * *           \r\n"},
    {"                                                                     \r\n"},
    {"                                                                     \r\n"},
    {"         =====================================================       \r\n"},
};


sData   sFirmVersion = {(uint8_t *) "SVWM_LORA_NO_V1_1_0", 19};

static UTIL_TIMER_Object_t TimerTx;


/*======================Function========================*/
/*====== Func Call Event ==========*/
static uint8_t _Cb_Timer_IRQ(uint8_t event)
{
	UTIL_TIMER_IRQ_Handler();

	return 1;
}

/*
    Khi co su kien:
        + Start: lay moc thoi gian systick
                 Lay moc length hien tai
        + cu 5ms vao check:
                ++ Neu length bang voi length truoc: Nhan xong
                ++ Neu length khac length truoc: thuc hien nhu buoc start
                                                 tiep tuc enable cho check tiep
*/
static uint8_t _Cb_Uart_Debug_Process(uint8_t event)
{
    static uint8_t  MarkFirstRecvUart = 0;
    static uint16_t LastLengthRecv = 0; 
        
    if (MarkFirstRecvUart == 0)
    {
        MarkFirstRecvUart = 1;
        LastLengthRecv = sUartDebug.Length_u16;
        fevent_enable(sEventAppMain, event);
    } else
    {
        if (sUartDebug.Length_u16 == LastLengthRecv)
        {
            MarkFirstRecvUart = 0;
            UTIL_Printf( sUartDebug.Data_a8, sUartDebug.Length_u16 );
            UTIL_Printf( (uint8_t*) "\r\n", 2 );

            Check_AT_User(&sUartDebug, _AT_REQUEST_SERIAL);

            Reset_Buff(&sUartDebug);
        } else
        {
            LastLengthRecv = sUartDebug.Length_u16;
            fevent_enable(sEventAppMain, event);
        }
    }
    
    sModem.CountEventWaitConfg_u8 = 0;  //Cho phep giu 20s config

	return 1;
}



static uint8_t _Cb_Set_RTC(uint8_t event)
{
    UTIL_Set_RTC(sRTCSet);

    fevent_enable(sEventAppMain, _EVENT_IDLE);

    return 1;
}


static uint8_t _Cb_HandLer_IDLE(uint8_t event)
{
	Get_RTC();

#ifdef USING_APP_WM
    AppWm_Get_VBat();
    AppWm_Get_VOut();
#endif 
   
#ifdef USING_APP_SIM
    //Check True Update firmware -> wait Pulse increase to reset.
    if ((sModem.IsOverFivePulse_u8 == TRUE) && (sSimVar.IsUpdateFinish_u8 == 1))
        Reset_Chip();
    
    static uint8_t MarkGetStime_u8 = 0;
    static uint8_t MarkResetDaily = 0;
    //Check Request Stime To Server
    if ((sRTC.hour == 0) && (MarkGetStime_u8 == 0))
    {
        MarkGetStime_u8 = 1;
        sMQTT.aMARK_MESS_PENDING[SEND_SERVER_TIME_PENDING] = TRUE;
    } else if (sRTC.hour != 0)
    {
        MarkGetStime_u8 = 0;
    }
    
    //Check reset daily
    if ( (sRTC.hour == 14) && (MarkResetDaily == 1) )
    {
        //Reset thiet bi
        Reset_Chip();
    } else if (sRTC.hour != 14)
    {
        MarkResetDaily = 1;
    }
    
#endif

	return 1;
}


static uint8_t _Cb_Tx_Timer(uint8_t event)
{
#ifdef USING_APP_WM
    //Active Event Check Alarm
    fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
#endif
    
#ifdef USING_APP_EMET
    //Active Event Check Alarm
    fevent_active(sEventAppEmet, _EVENT_LOG_TSVH);
    fevent_active(sEventAppEmet, _EVENT_SCAN_ALARM);
#endif

#ifdef USING_APP_TEMH
    fevent_active(sEventAppTempH, _EVENT_ENTRY_TEMH);
#endif
 
    //Increase Count To Send Mess
    sModem.CountSleepToSend_u8++;
	if (sModem.CountSleepToSend_u8 >= sFreqInfor.NumWakeup_u8)
	{
        sModem.CountSleepToSend_u8 = 0;
    #ifdef USING_APP_SIM
        fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);
        sModem.IsReqGetLocation_u8 = true;
    #endif
	}

    return 1;
}

/*
    Func: handler send log
    + to Uart
    + to Server
*/
static uint8_t _Cb_Read_Old_Record (uint8_t event)
{
    fevent_enable(sEventAppMain, event);

    if (LOG_Read_Old_Record ( &sOldRec ) == FALSE)
    {
        //Enable event again
        fevent_disable(sEventAppMain, event);
    }

    return 1;
}

static uint8_t _Cb_Save_Box (uint8_t event)
{
    
   return 1;
}

/*=========================== Func App Main ========================*/
void SysApp_Init (void)
{       
#ifdef USING_APP_EMET
    __HAL_IWDG_START(&hiwdg);
#endif

#ifdef USING_APP_WM
    LPTIM_Init_Feature();
#endif
       
#ifdef USING_APP_LORA
    AppLora_Init ();
#else
    //Init timer: Init RTC...c?u h�nh alarm profile
	UTIL_TIMER_Init();
#endif
    
	//Get RTC
	Get_RTC();
    
    /*Init low power manager*/
    UTIL_LPM_Init();
}


void SysApp_Setting (void)
{
	//Init Modem
	Init_Uart_Module();
    //Init information in Memory
	Init_Memory_Infor();
    //Init Func Pointer to Peripheral
    Modem_Init_Feature();
    //Dang ki cac event
	UTIL_TIMER_Create(&TimerTx,  0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Cb_TX_Timer_Event, NULL);
	UTIL_TIMER_SetPeriod (&TimerTx, sFreqInfor.FreqSendUnitMin_u32 * 60000);   //Set 2 phut quay lai truyen ban tin thu 2
	UTIL_TIMER_Start (&TimerTx);
}

/*
    Note:
    - khi gen lai
        + Phai sua lai chan uart 2 thanh Pull up o trong code
        + Comment fun init in rtc

*/

/*
    Func: Init Queue trong main
*/

void Main_Task (void)
{
    uint8_t TaskStatus_u8 = 0;
    
    SysApp_Init();
    SysApp_Setting();
    
    UTIL_Log((uint8_t *) "=Day la version:", 16);
	UTIL_Log(sFirmVersion.Data_a8, sFirmVersion.Length_u16);
    
#ifdef USING_APP_EMET
    Screen_Init();
    Button_Init();
    EMeasure_Init();
    Relay_Init();
    LED_Init();
    Temp_Humid_Init();
#endif
    
    HAL_Delay(3000);
    
	for (;;)
	{
		TaskStatus_u8 = 0;

		TaskStatus_u8 |= AppCom_Task();

    #ifdef USING_APP_SIM
        TaskStatus_u8 |= AppSim_Task();
        TaskStatus_u8 |= Sim_Task();
    #endif
      
    #ifdef USING_APP_TEMH
        TaskStatus_u8 |= AppTemH_Task(); 
    #endif
            
    #ifdef USING_APP_WM
        TaskStatus_u8 |= AppWm_Task(); 
    #endif
        
    #ifdef USING_APP_EMET
        Button_Task();
        Screen_Task();
        EMeasure_Task();
        Relay_Task();
        LED_Task();
        Temp_Humid_Task();
    #endif
      
    #ifdef USING_APP_LORA
        TaskStatus_u8 |= AppLora_Process();
    #endif
        
        if ((TaskStatus_u8 == 0) && (sModem.ModeSimPower_u8 == _POWER_MODE_SAVE) && (sModem.IRQPowerOutAgain_u8 == FALSE))
        {
        #ifdef USING_APP_SIM
            //Neu thuc hien xong tranfer qua module sim (hoac sim deep sleep | Poweroff) -> set che do ngu sau
            if (sSimCommon.PowerStatus_u8 == _POWER_POWER_OFF)
            {
                UTIL_LPM_SetStopMode((UTIL_LPM_State_t) LPM_FALSE);
            } else
                UTIL_LPM_SetStopMode((UTIL_LPM_State_t) LPM_TRUE);
        #else
            UTIL_LPM_SetStopMode((UTIL_LPM_State_t) LPM_FALSE);
        #endif
            //Func Lowpower
            UTIL_LPM_EnterLowPower();
        }
	}
}


/*=================Func handler=====================*/

void AppCom_Init (void)
{
    //Loai DCU
    sModem.TypeModem_u8 = _TEM_HUMI_GSM;   
    //Set Con tro mode Power: Save_Mode. OnlineMode
    sModem.ModeSimPower_u8 = _POWER_MODE_ONLINE;
    //Func Pointer Lib Timer
    pModemProcessIRQTimer = AppCom_IRQ_Timer_CallBack;
    //Func Pointer Lib LPM
    sLPMFeatureHandler = &sModemLPMHandler; 
    //Init Func Pointer Log to mem
    pFunc_Log_To_Mem = &LOG_Save_Mess;
    
    //Tinh toan gia tri random tx
    uint16_t Index = sModem.sDCU_id.Length_u16 - 1;
    
    sModem.TimeDelayTx_u32 = (*(sModem.sDCU_id.Data_a8 + Index) - 0x30) + (*(sModem.sDCU_id.Data_a8 + Index - 1) - 0x30) * 10 ;
    
    if (sFreqInfor.FreqWakeup_u32 != 0)
    {
        sModem.TimeDelayTx_u32 = (sModem.TimeDelayTx_u32 * 2) % (sFreqInfor.FreqWakeup_u32 * 60);
        sModem.TimeDelayTx_u32 *= 1000;
    } else
    {
        sFreqInfor.FreqWakeup_u32 = 10000;
    }
}



uint8_t AppCom_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;

	for (i = 0; i < _EVENT_END_MAIN; i++)
	{
		if (sEventAppMain[i].e_status == 1)
		{
            if (i != _EVENT_IDLE)
                Result = 1;

			if ((sEventAppMain[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppMain[i].e_systick)  >=  sEventAppMain[i].e_period))
			{
                sEventAppMain[i].e_status = 0;  //Disable event
				sEventAppMain[i].e_systick = HAL_GetTick();
				sEventAppMain[i].e_function_handler(i);
			}
		}
	}
    
    //Enable Event Period again
    if (sEventAppMain[_EVENT_IDLE].e_status == 0)
        fevent_enable(sEventAppMain, _EVENT_IDLE);

	return Result;
}


 
void AppCom_IRQ_Timer_CallBack (void)
{
    fevent_active(sEventAppMain, _EVENT_TIMMER_IRQ);
}




/*
    - Func: Tinh lai thoi gian sai lệch de hẹn gio tròn
*/
static uint32_t AppCom_Calcu_Period_To_RoudTime (uint32_t FreqWakeup)
{
    uint32_t FreqCacul = 0;

    Get_RTC();

    FreqCacul = FreqWakeup - (sRTC.min % FreqWakeup);   //Phep tinh nay tinh ra gia tri 1->Stimer.min. Khong co gia tri 0
    //Sub second current, and add to 2 (dư phong)
    if (sRTC.sec < (FreqCacul * 60))
    {
        FreqCacul  = FreqCacul * 60 - sRTC.sec + 2;
        return (FreqCacul * 1000);
    }

    return (2 * 60000); //default
}


static void Cb_TX_Timer_Event(void *context)
{
	Modem_Log_And_Send_Emergency((uint8_t*) "=Timer TX Event callback=\r\n", 27);

    fevent_active(sEventAppMain, _EVENT_TX_TIMER);

    AppCom_Set_Next_TxTimer();
}


void AppCom_Set_Next_TxTimer (void)
{
    static uint8_t CountMessInit = 0;
    uint32_t FreqCacul = 10000;

    //Get chu ki gui tiep theo
	if (CountMessInit < MAX_NUNBER_SEND_INIT)
    {
		CountMessInit ++;
        sModem.CountSleepToSend_u8 = sFreqInfor.NumWakeup_u8;  //cho send luon ban tin
	} else
    {
        if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
        {
            sFreqInfor.FreqSendUnitMin_u32 = sFreqInfor.FreqSendOnline_u32;  //Doi sang minute
            Get_RTC();
            FreqCacul = sFreqInfor.FreqSendUnitMin_u32 - (sRTC.min % sFreqInfor.FreqSendUnitMin_u32);
            FreqCacul = FreqCacul * 60 - sRTC.sec;
            FreqCacul *= 1000;
        } else
        {
            sFreqInfor.FreqSendUnitMin_u32 = sFreqInfor.NumWakeup_u8 * sFreqInfor.FreqWakeup_u32;
            //cacul period again to send at rounding stime:
            FreqCacul = AppCom_Calcu_Period_To_RoudTime (sFreqInfor.FreqWakeup_u32);
        }

        //set Period again.
        UTIL_TIMER_SetPeriod(&TimerTx, FreqCacul);
    }
    //Set tiep timer Tx vơi Duty = Thơi gian tròn cua chu ki
	UTIL_TIMER_Start(&TimerTx);
}




/*
    Func: Callback IRQ gpio:
        + PIN 6: DTC
        + PIN 14: CONFIG Button
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)
    {
       case GPIO_PIN_6:
            
            break;
        case GPIO_PIN_14:
        
    
            break;
        default:
            break;           
    }        
}











