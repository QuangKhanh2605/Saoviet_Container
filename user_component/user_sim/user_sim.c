/*
 * user_sim.c
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */
#include "string.h"
     
#include "user_sim.h"

#include "user_timer.h"
#include "user_string.h"

#include "user_internal_mem.h"  
     
     
/*=========================var struct====================*/
//Define timer handler sim Module
static UTIL_TIMER_Object_t TimerSendTimeout;
static UTIL_TIMER_Object_t TimerNextSendAT;
static UTIL_TIMER_Object_t TimerControlBC66;

/*====================Static function=================*/
/* event handler function */
static uint8_t fevent_sim_at_send_handler(uint8_t event);
static uint8_t fevent_sim_at_send_ok_handler(uint8_t event);
static uint8_t fevent_sim_at_send_timeout_handler(uint8_t event);
static uint8_t fevent_sim_uart_receive_handler(uint8_t event);
static uint8_t fevent_sim_turn_on_handler(uint8_t event);
static uint8_t fevent_sim_hard_reset_handler(uint8_t event);
static uint8_t fevent_sim_wake_up_handler(uint8_t event);
static uint8_t fevent_sim_dtr_pin_handler(uint8_t event);
static uint8_t fevent_sim_power_off_handler(uint8_t event);


//Timer event handler
static void OnTimerSendTimeOutEvent(void *context);
static void OnTimerNextSendATEvent(void *context);
static void OnTimerPowerOn(void *context);

static uint8_t Sim_Process_AT_CallBack (uint8_t Type);
static void Sim_Process_URC_Callback (uint8_t Type);
static void Sim_Process_AT_Failure (void);
static void Sim_Check_Change_Server (void);

/*==================Struct var========================*/

sEvent_struct sEventSim[] =
{
	{ _EVENT_SIM_AT_SEND, 			0, 0, SIM_CMD_FREQ,    	fevent_sim_at_send_handler },
    { _EVENT_SIM_UART_RECEIVE, 		0, 0, 5, 				fevent_sim_uart_receive_handler },  //20
	{ _EVENT_SIM_AT_SEND_OK, 		0, 0, 0, 			    fevent_sim_at_send_ok_handler },
	{ _EVENT_SIM_AT_SEND_TIMEOUT, 	0, 0, SIM_CMD_TIMEOUT, 	fevent_sim_at_send_timeout_handler },
	{ _EVENT_SIM_HARD_RESET,		0, 0, 0, 				fevent_sim_hard_reset_handler },
	{ _EVENT_SIM_TURN_ON, 			0, 0, 0, 				fevent_sim_turn_on_handler },
	{ _EVENT_SIM_WAKEUP, 			0, 0, 0, 				fevent_sim_wake_up_handler },
    { _EVENT_SIM_DTR_PIN, 	        0, 0, 1000, 			fevent_sim_dtr_pin_handler },
    { _EVENT_SIM_POWER_OFF, 		0, 0, 0, 				fevent_sim_power_off_handler },
};


/*======= Buffer Uart SIM =============*/
uint8_t uartSimBuffReceive[1200] = {0};
sData 	sUartSim = {&uartSimBuffReceive[0], 0};

//
StructSimVariable    sSimVar = 
{
    //Init Var Default Sim
    .ModeConnectNow_u8       = MODE_CONNECT_DATA_MAIN,
    .ModeConnectLast_u8      = MODE_CONNECT_DATA_MAIN,
    .ModeConnectFuture_u8    = 0,  
};

/*========= Func Pointer ===========*/
sFuncCallBackModem   *sFuncCBModem = NULL;

sFuncCallbackHandlerSim     sSimCallBackHandler = 
{
    .pSim_Common_Handler_AT     = Sim_Process_AT_CallBack,
    .pSim_Common_Handler_URC    = Sim_Process_URC_Callback,
    .pSim_Common_Config_Update  = Sim_Set_Config_UpdateFw,
    .pSim_Common_Get_Fw_Version = Sim_Get_Fw_Version,
};


static uint8_t aQSimStepComtrol[SIM_MAX_ITEM_QUEUE];
Struct_Queue_Type qSimStep;


/*=================Function callback event sim==============*/

/* 
    Func: Send Chuoi AT cmd sang Module sim
        + Neu loai AT bt: Timeout 10s
        + Neu loai AT TCP..  Timeout 60s -> Test 30s
        + Retry 3 time
*/

static uint8_t fevent_sim_at_send_handler(uint8_t event)
{
	uint8_t sim_step = 0;
    sCommand_Sim_Struct *sATCmd = NULL;
    
    if (sSimVar.IsAllowSendAT_u8 == FALSE)
    {
        //Get step current and don't clear
        sim_step = Sim_Get_Step_From_Queue(0); 
            
        if (sim_step < SIM_STEP_END)
        {
            sSimVar.IsRunningATcmd_u8 = true;
            sSimVar.LandMarkSendAT_u32 = RtCountSystick_u32;
            //Neu lenh Out Data mode -> Dieu khien chan DTR
            if (sim_step == SIM_STEP_OUT_DATA_MODE)
            {
                if (sSimVar.NumRetry_u8 == 0)   
                {
                    sSimVar.NumRetry_u8++;
                    Sim_event_active(_EVENT_SIM_DTR_PIN); 
                    sEventSim[_EVENT_SIM_AT_SEND].e_status = 0;    //Deactive wait timer Next Send AT
                    
                    UTIL_TIMER_SetPeriod(&TimerNextSendAT, 1000); //Delay 1s before and affer send +++
                    UTIL_TIMER_Start(&TimerNextSendAT);
                    return 1;
                }
            }
            
            //Increase Num At Precess
            sSimVar.NumATProcess_u8++;
            //Get Struct AT Cmd
            sATCmd = SIM_GET_AT_CMD(sim_step);        
            //Send To uart Sim string AT
            if (sATCmd->at_string  != NULL)
                Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *) sATCmd->at_string , strlen( sATCmd->at_string ), 1000);

            //Neu k co str respond -> Excute Callback -> active send OK
            if (sATCmd->at_response == NULL)
            {
                if (sATCmd->callback_success != NULL)
                    sATCmd->callback_success ( &sUartSim );

                Sim_event_active(_EVENT_SIM_AT_SEND_OK);
                sSimVar.RespStatus_u8 = TRUE;
            } else
            {
                //Lenh AT binh thuong timeout 10s. Lenh AT TCP timeout 60s.   
                if (SIM_CHECK_STEP_LONG_TIMEOUT(sim_step) == TRUE)
                    UTIL_TIMER_SetPeriod(&TimerSendTimeout, SIM_TCP_TIMEOUT);
                else
                    UTIL_TIMER_SetPeriod(&TimerSendTimeout, SIM_CMD_TIMEOUT);
                
                UTIL_TIMER_Start(&TimerSendTimeout);
                //Increase Retry in case need check AT cmd
                sSimVar.NumRetry_u8++;
            }
        } else
        {
            //Neu dang k xu lý at nao: -> Cho phep active khi push lenh
            sSimVar.IsRunningATcmd_u8 = false;
        }
    } else
    {
        fevent_enable(sEventSim, _EVENT_SIM_AT_SEND);
    }
    

	return 1;
}


/*
 *Func: Handler: Excute AT OK
 *	- Active event send
 *	- Jum step to 1
 *
 *note:
 *	- Trong trường hợp attach (k cần gửi quá nhanh)
 *	- Set timeout check attach:
 * */

static uint8_t fevent_sim_at_send_ok_handler(uint8_t event)
{
	uint8_t StepNow = 0;
    sCommand_Sim_Struct *sATCmd = NULL;
    sCommand_Sim_Struct *sATCmdNext = NULL;
    uint32_t TimeDelay2AT = 0;
    
	UTIL_TIMER_Stop(&TimerSendTimeout);

    StepNow = Sim_Get_Step_From_Queue(0);
    //ResPond True: Reset retry,...  | else CGATT retry
	if (sSimVar.RespStatus_u8 == TRUE)
	{
        //Get Struct AT Cmd
        sATCmd = SIM_GET_AT_CMD(StepNow); 
        //Neu AT have Response need Check must reset Retry and NumATProcess
        if (sATCmd->at_response != NULL)
        {
            sSimVar.NumRetry_u8 = 0;
            sSimVar.NumATProcess_u8 = 0;
        }
        //Reset count retry
		sSimVar.CountStepCGATT_u8 = 0;
        //handler Delay before send next AT
        TimeDelay2AT = SIM_TYPE_DELAY_2_STEP(StepNow);
        //Next step in queue
		StepNow = Sim_Get_Step_From_Queue(1);
        //Get Struct AT Cmd
        sATCmdNext = SIM_GET_AT_CMD(StepNow);  
        if (TimeDelay2AT != 0)
        {
            UTIL_TIMER_SetPeriod(&TimerNextSendAT, TimeDelay2AT);
        } else
        {
            //Neu nhung lenh tiep theo nao k co gui di: active ngay send at | Lenh truoc k co resp
            if ( ( sATCmdNext->at_string  == NULL) || (sATCmd->at_response == NULL) )
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND); 
            else
                fevent_enable(sEventSim, _EVENT_SIM_AT_SEND);
            
            return 1;
        }
	} else
	{
		/*=== Neu Attach can retry lai 20 lan=  | Neu Open TCP hay lenh khac ========Cho sang timeout====*/
		if ((StepNow == SIM_STEP_ATTACH) && (sSimVar.CountStepCGATT_u8 < SIM_MAX_CGATT))
		{
            sSimVar.CountStepCGATT_u8++;
            UTIL_TIMER_SetPeriod(&TimerNextSendAT, SIM_CMD_TIMEOUT2);
		} else  
		{
			sSimVar.NumRetry_u8 = SIM_CMD_RETRY;
			fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);   //Active event timeout, va cho retry = MAX
			return 1;
		}
	}

    sEventSim[_EVENT_SIM_AT_SEND].e_status = 0; //Deactive wait timer Next Send AT
	UTIL_TIMER_Start(&TimerNextSendAT);

	return 1;
}

/*
 * Func: Handler timeout AT
 * 		- Disable event
 * 		- Set lai event Send AT
 * */

static uint8_t fevent_sim_at_send_timeout_handler(uint8_t event)
{
	uint8_t StepNow = 0;
    sCommand_Sim_Struct *sATCmd = NULL;
    
    UTIL_TIMER_Stop(&TimerSendTimeout);
	//Get step Timeout and print to debug step timeout
	StepNow = Sim_Get_Step_From_Queue(0);
	UTIL_Printf_Dec(DBLEVEL_M, StepNow);

    //Get Struct AT Cmd
    sATCmd = SIM_GET_AT_CMD(StepNow); 
    UTIL_Log( DBLEVEL_M, (uint8_t *) sATCmd->at_string, strlen(sATCmd->at_string) );
    
    if (sSimVar.NumRetry_u8 >= SIM_CMD_RETRY)
    {
        //Handler timeout -> retry or excute cb_AT_Failure
        if (sATCmd->callback_failure != NULL)
            sATCmd->callback_failure(0);
    } else
    {
        //Back AT in queue and Send Again
        if (Sim_Back_Current_AT(sSimVar.NumATProcess_u8 - 1) == 1)
        {    
            //Active event Send AT
            sSimVar.NumATProcess_u8 = 0;
            fevent_active(sEventSim, _EVENT_SIM_AT_SEND);
        } else
        {
            //Handler timeout -> retry or excute cb_AT_Failure
            if (sATCmd->callback_failure != NULL)
                sATCmd->callback_failure(0);
        }
    }
    
	return 1;
}

static uint8_t fevent_sim_uart_receive_handler(uint8_t event)
{
    static uint8_t  MarkFirst = 0;
    static uint16_t LastLengthRecv = 0; 
	uint8_t sim_step = 0;

    if (MarkFirst == 0)
    {
        MarkFirst = 1;
        LastLengthRecv = sUartSim.Length_u16;
        fevent_enable(sEventSim, event);
    } else
    {
        if (sUartSim.Length_u16 == LastLengthRecv)
        {
            MarkFirst = 0;

            sim_step = Sim_Get_Step_From_Queue(0);
            Sim_Check_Response(sim_step);
        } else
        {
            LastLengthRecv = sUartSim.Length_u16;
            fevent_enable(sEventSim, event);
        }
    }
    
	return 1;
}


static uint8_t fevent_sim_turn_on_handler(uint8_t event)
{
    uint8_t i = 0;
    
    Sim_Common_Init_Var_Default();
    //Disable Event
	for (i = 0; i < _EVENT_SIM_END; i++)
        sEventSim[i].e_status = 0;  
    //Power on L506
	if (SIM_POWER_ON() == 1)
	{
        //Push AT cmd 
        if (sSimVar.CountHardReset_u8 < MAX_HARD_RESET)
        {
            if (sSimVar.ModeConnectFuture_u8 != 0)
            {
                sSimVar.ModeConnectLast_u8 = sSimVar.ModeConnectNow_u8;    //Luu lai mode cuoi cung
                sSimVar.ModeConnectNow_u8 = sSimVar.ModeConnectFuture_u8;
                sSimVar.ModeConnectFuture_u8 = 0; 
            }
            //Check Mode 
            if ((sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_MAIN) || (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_BACKUP))
            {
                Sim_Clear_Queue_Step();
                fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));  
                fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
            } else if (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_HTTP)
            {
                Sim_Clear_Queue_Step();
                fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit)); 
                fPushBlockSimStepToQueue(aSimStepBlockHttpInit, sizeof(aSimStepBlockHttpInit));
                fPushBlockSimStepToQueue(aSimStepBlockHttpRead, sizeof(aSimStepBlockHttpRead));
            }  //Con FTP chua lam
        } else
        {
            //Neu dang o update firm -> Ket noi lai vao reset
            if ((sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_MAIN) && (sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_BACKUP))
            {
                sSimVar.ModeConnectNow_u8 = sSimVar.ModeConnectLast_u8;
                Sim_Clear_Queue_Step();
                fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));
                fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));

                return 1;
            }
            //
            if ( (*sSimVar.ModePower_u8 == _POWER_MODE_SAVE) ||   \
                 ( (*sSimVar.ModePower_u8 == _POWER_MODE_ONLINE) && (sFuncCBModem->pSim_Reset_MCU() == 0) ) )
            {
                Sim_Clear_Queue_Step();
                fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));
                fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
            }
        }
	}

	return 1;
}


static uint8_t fevent_sim_hard_reset_handler(uint8_t event)
{
    fevent_active(sEventSim, _EVENT_SIM_TURN_ON);

	return 1;
}

static uint8_t fevent_sim_wake_up_handler(uint8_t event)
{
	return 1;
}

/*
    Func: Control DTR PIN to Out Data mode
    Lan dau tien vao keo chan DTR low -> 1s sau Keo DTR lï¿½n high: High to low?
*/

static uint8_t fevent_sim_dtr_pin_handler(uint8_t event)
{
    static uint8_t DTR_PIN_Control_u8 = 0;
    
    if (DTR_PIN_Control_u8 == 0)
    {
        SIM_DTR_ON1;
        DTR_PIN_Control_u8++;
        fevent_enable(sEventSim, _EVENT_SIM_DTR_PIN);
    } else
    {
        SIM_DTR_OFF1;
        DTR_PIN_Control_u8 = 0;
    }
    
	return 1;
}

static uint8_t fevent_sim_power_off_handler(uint8_t event)
{
    uint8_t i = 0;
    
    UTIL_Log_Str(DBLEVEL_M, "=Power off SIM=\r\n" );
    //Neu k hard reset duoc se tiep tuc vao hard Reset lai ->Vi no vao day se fail Power off ->hard Reset
    Sim_Clear_Queue_Step();
    //clear event sim
    for (i = 0; i < _EVENT_SIM_END; i++)
        sEventSim[i].e_status = 0;
    
    //Set Status FinishTransPacket
    sSimCommon.PowerStatus_u8 = _POWER_POWER_OFF;
    //Power off bang cach ngat nguon dien
    SIM_PW_OFF1;  
    //Check Reset MCU
	if (sSimVar.CountHardReset_u8 >= MAX_HARD_RESET)
    {
        if ((sFuncCBModem->pSim_Reset_MCU () == 0) && ( *sSimVar.ModePower_u8 == _POWER_MODE_ONLINE ) )
        {
            Sim_Clear_Queue_Step();
            fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));
            fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
            fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
        }
    }
    
    return 1;
}


uint8_t fPushSimStepToQueue(uint8_t sim_step)
{
	if (sim_step >= SIM_STEP_END)
		return 0;

	if (qQueue_Send(&qSimStep, (uint8_t *) &sim_step, _TYPE_SEND_TO_END) == 0)
		return 0;

    if ((sSimVar.IsRunningATcmd_u8 == false ) || (Check_Time_Out(sSimVar.LandMarkSendAT_u32, 80000) == true) )
		fevent_active(sEventSim, _EVENT_SIM_AT_SEND);

	return 1;
}

uint8_t fPushBlockSimStepToQueue(uint8_t *block_sim_step, uint8_t size)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++)
	{
		if (fPushSimStepToQueue(block_sim_step[i]) == 0)
			return 0;
	}

	return 1;
}


uint8_t Sim_Check_Response(uint8_t sim_step) // alternative by ring buffer
{
    int     PosFind = 0;
    uint16_t  i = 0;
    sCommand_Sim_Struct *sATCmd = NULL;
    sData   strCheck = { NULL, 0};
    
    //Get String Response
    sATCmd = SIM_GET_AT_CMD(sim_step);  
    strCheck.Data_a8 = (uint8_t*) sATCmd->at_response;
    strCheck.Length_u16 = strlen( sATCmd->at_response );                
	//Check Respond
    if (sim_step < SIM_STEP_END)
	{
        if (sATCmd->at_response != NULL)
        {
            PosFind = Find_String_V2(&strCheck, &sUartSim);
            if (PosFind >= 0)
            {
                sSimVar.RespStatus_u8 = sATCmd->callback_success ( &sUartSim );
                Sim_event_active( _EVENT_SIM_AT_SEND_OK );
                
                sSimCommon.TxPinReady_u8 = TRUE;
            }
        }
    }
    //Check Urc
    if (SIM_IS_STEP_CHECK_URC (sim_step) == TRUE)
    {
        for (i = _SIM_URC_RESET_SIM900; i < _SIM_URC_END; i++) 
        {
            strCheck.Data_a8 = (uint8_t *) aSimUrc[i].at_response;
            strCheck.Length_u16 = strlen(aSimUrc[i].at_response);
            //
            PosFind = Find_String_V2(&strCheck, &sUartSim);
            if (PosFind >= 0)
                aSimUrc[i].callback_success(&sUartSim);
        }
    }
    
    //Printf debug
    UTIL_Printf( DBLEVEL_M, sUartSim.Data_a8, sUartSim.Length_u16);
    
	memset(&uartSimBuffReceive[0], 0, sizeof(uartSimBuffReceive));
	sUartSim.Length_u16 = 0;
    //Chan rx OK
    sSimCommon.RxPinReady_u8 = TRUE;   

	return 1;
}


/*===================Function hanlder event timer==================*/

static void OnTimerSendTimeOutEvent(void *context)
{
	UTIL_Printf_Str( DBLEVEL_M, "=Send AT Timeout=\r\n" );
	fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
}

static void OnTimerNextSendATEvent(void *context)
{
	fevent_active(sEventSim, _EVENT_SIM_AT_SEND);
}


/*
 * Func: Check xem co phai SIM dang cho Response ? -> De nhan Error -> Handler timerout
 * */
uint8_t Is_SIM_Waitting_Response (void)
{
	return TimerSendTimeout.IsRunning;
}

uint8_t Sim_event_disable(uint8_t event_sim)
{
	if (event_sim >= _EVENT_SIM_END)
		return 0;
	fevent_disable(sEventSim, event_sim);

	return 1;
}

uint8_t Sim_event_enable(uint8_t event_sim)
{
	if (event_sim >= _EVENT_SIM_END)
		return 0;
	fevent_enable(sEventSim, event_sim);

	return 1;
}

uint8_t Sim_event_active(uint8_t event_sim)
{
	if (event_sim >= _EVENT_SIM_END)
		return 0;
	fevent_active(sEventSim, event_sim);

	return 1;
}


/*
 * Type = 0 - don't clear queue
 * */
uint8_t Sim_Get_Step_From_Queue(uint8_t Type)
{
	uint8_t sim_step = 0;

	if (qGet_Number_Items(&qSimStep) == 0)
		return SIM_STEP_END;
    
	if (qQueue_Receive(&qSimStep, (uint8_t *) &sim_step, Type) == 0)
        return SIM_STEP_END;

	return sim_step;
}

/*
    Fun: back numitem in queue to send again AT
*/
uint8_t Sim_Back_Current_AT (uint8_t NumAT)
{
    return qQueue_Back_Item (&qSimStep, NumAT);
}


void Sim_Disable_All_Event (void)
{
	uint8_t i = 0;

    //Disable Event
	for (i = 0; i < _EVENT_SIM_END; i++)
        sEventSim[i].e_status = 0;  
    
    Sim_Clear_Queue_Step();
    //Disable Timer TX
    UTIL_TIMER_Stop (&TimerSendTimeout);
    UTIL_TIMER_Stop (&TimerNextSendAT);
    UTIL_TIMER_Stop (&TimerControlBC66); 
    //Set Step ON to Zero
    sSimVar.StepPowerOn_u8 = 0;
    sSimCommon.PowerStatus_u8 = _POWER_START;
}


static void OnTimerPowerOn(void *context)
{
	//active event sleep hoac powon hoac wakeup
	fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
}


void Sim_Init_Timer_PowerOn (void)
{
	UTIL_TIMER_Create(&TimerControlBC66, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerPowerOn, NULL);
}



static void Sim_Process_URC_Callback (uint8_t Type)
{
    uint8_t StepHandle = 0;
    
    switch (Type)
    {
        case _SIM_COMM_URC_RESET_SIM: 
            //Print power down
            UTIL_Log_Str (DBLEVEL_M, "=URC POWER DOWN=\r\n" );
            fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            
            sSimVar.NumRetry_u8 = SIM_CMD_RETRY;
            break;
        case _SIM_COMM_URC_SIM_LOST: 
            //Sim Remove
            UTIL_Log_Str (DBLEVEL_M, "=URC SIM LOSST=\r\n" );
            fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            sSimVar.NumRetry_u8 = SIM_CMD_RETRY;
            break;
        case _SIM_COMM_URC_CLOSED: 
            //Push lenh dong ket noi va Khoi dong lai
            UTIL_Log_Str (DBLEVEL_M, "=URC TCP CLOSE=\r\n" );

            //Trong trương hợp ngat nguon: power off dot ngot: -> k cho no la tcp close
            if (sSimCommon.PowerStatus_u8 != _POWER_POWER_OFF)
            {
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);  
                sSimVar.NumRetry_u8 = SIM_CMD_RETRY;
            }
            break;
        case _SIM_COMM_URC_CALLING: 
          
            break;
        case _SIM_COMM_URC_SMS_CONTENT: 
            //Check at cmd trong noi dung sms
            sFuncCBModem->pSim_Process_Sms (&sUartSim);
            break;    
        case _SIM_COMM_URC_SMS_IND: 
            //push at cmd read sms
            fPushBlockSimStepToQueue(aSimStepBlockSMS, sizeof(aSimStepBlockSMS)); 
            break;                     
        case _SIM_COMM_URC_ERROR: 
            UTIL_Printf_Str ( DBLEVEL_M, "=URC ERROR=\r\n" );
            
            StepHandle = Sim_Get_Step_From_Queue(0);
            
            if (SIM_IS_STEP_SKIP_ERROR(StepHandle) == true)
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND_OK );
            else
                fevent_active(sEventSim, _EVENT_SIM_AT_SEND_TIMEOUT);
            break;
        case _SIM_COMM_URC_RECEIV_SERVER: 
            sFuncCBModem->pSim_Handler_Downlink_Server (&sUartSim);
            break;
        default:
            break;
    }
}



static uint8_t Sim_Process_AT_CallBack (uint8_t Type)
{
    switch (Type)
    {
        case _SIM_COMM_EVENT_AT_FAILURE:
            Sim_Process_AT_Failure ();
            break; 
        case _SIM_COMM_EVENT_GET_CONN:  
            return sSimVar.ModeConnectNow_u8;
        case _SIM_COMM_EVENT_HTTP_READ_OK:
            fPushBlockSimStepToQueue(aSimStepBlockHttpRead, sizeof(aSimStepBlockHttpRead)); 
            break; 
        case _SIM_COMM_EVENT_SIM_CARD:
        case _SIM_COMM_EVENT_GET_STIME: 
        case _SIM_COMM_EVENT_GPS_OK:
        case _SIM_COMM_EVENT_GPS_ERROR:
        case _SIM_COMM_EVENT_TCP_SEND_1:
        case _SIM_COMM_EVENT_TCP_SEND_2:
        case _SIM_COMM_EVENT_CONN_MQTT_1:
        case _SIM_COMM_EVENT_CONN_MQTT_2:
        case _SIM_COMM_EVENT_SUB_MQTT_1:
        case _SIM_COMM_EVENT_SUB_MQTT_2:
        case _SIM_COMM_EVENT_PUB_MQTT_1:    
        case _SIM_COMM_EVENT_PUB_MQTT_2:
        case _SIM_COMM_EVENT_HTTP_UPDATE_OK:  
        case _SIM_COMM_EVENT_HTTP_UPDATE_FAIL:
            sFuncCBModem->pSim_Handler_AT_Cmd (Type);
            break;
        default: 
            break;
    }
    
    return 1;
}

  

static void Sim_Check_Change_Server (void)
{
    uint8_t Diff = 0;
    static uint8_t LastCountHdReset_u8 = 0;
        
    if (sSimVar.CountHardReset_u8 == 0)
        LastCountHdReset_u8 = 0;
    else
    {
        //Cacul Diff Hard RS
        if (sSimVar.CountHardReset_u8 > LastCountHdReset_u8)
            Diff = sSimVar.CountHardReset_u8 - LastCountHdReset_u8;
        else
            Diff = LastCountHdReset_u8 - sSimVar.CountHardReset_u8;

        //Check Change Server
        if ((Diff % MAX_RETRY_CHANGE_SERVER) == 0)
        {
            //Doi server
            if (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_BACKUP)
                sSimVar.ModeConnectNow_u8 = MODE_CONNECT_DATA_MAIN;
            else if (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_MAIN)
                sSimVar.ModeConnectNow_u8 = MODE_CONNECT_DATA_BACKUP;
            
            UTIL_Log_Str(DBLEVEL_M, "=Change Server Infor=\r\n" );
            //Mark Count HRS
            LastCountHdReset_u8 = sSimVar.CountHardReset_u8;
        }
    }
}

static void Sim_Process_AT_Failure (void)
{
    MX_USART2_UART_Init();
    Init_Uart_Sim_Rx_IT();
    //Tang soft Reset
	sSimVar.CountSoftReset_u8++;
    sSimVar.ConnSerStatus_u8 = FALSE;
	Sim_Clear_Queue_Step();
	//Neu Soft Reset > 2
	if (sSimVar.CountSoftReset_u8 < MAX_SOFT_RESET)
	{
        sSimCommon.PowerStatus_u8 = _POWER_INIT;
        
		switch (sSimCommon.GroupStepID_u8)
		{
			case _GR_PREE_INIT:
			case _GR_CHECK_ATTACH:
			case _GR_SETTING:
				fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));
                fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
                sSimCommon.GroupStepID_u8 = _GR_PREE_INIT; 
				break;
			case _GR_INIT_TCP:
                fPushBlockSimStepToQueue(aSimStepBlockInit, sizeof(aSimStepBlockInit));
                fPushBlockSimStepToQueue(aSimStepBlockNework, sizeof(aSimStepBlockNework));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
                sSimCommon.GroupStepID_u8 = _GR_PREE_INIT; 
                break;
			case _GR_SEND_MQTT:
				fPushBlockSimStepToQueue(aSimStepBlockDisConnect, sizeof(aSimStepBlockDisConnect));
                fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
                sSimCommon.GroupStepID_u8 = _GR_INIT_TCP; 
				break;
			case _GR_CLOSE_TCP:
				break;
            case _GR_READ_HTTP:
                Sim_Clear_Queue_Step();
                Sim_event_active(_EVENT_SIM_TURN_ON);
                break;
			default:
				break;
		}
	} else
	{
        sSimVar.CountSoftReset_u8 = 0;
        sSimVar.CountHardReset_u8++;
        //Kiem tra chuyen server khac
        Sim_Check_Change_Server();
        //Neu dang Case Online ->Pow on lai | Neu case Save Mode -> Pow off module sim
        if (*sSimVar.ModePower_u8 == _POWER_MODE_SAVE)
            fevent_active(sEventSim, _EVENT_SIM_POWER_OFF);
        else
            fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
	}
}



/*
    Func: Set Config Update Fw
        + Set Add new FW
        + Set MaxSizePage
*/

void Sim_Set_Config_UpdateFw (void)
{
    uint32_t AddNewFw = 0;
    uint16_t MaxSizeConfig = 0;
    
    sFuncCBModem->pSim_Get_Config_UpdateFw(&AddNewFw, &MaxSizeConfig);
    
    //Config to struct update
    Sim_Common_Config_UpdateFw (AddNewFw, MaxSizeConfig);
}

sData * Sim_Get_Fw_Version (void)
{
    return sFuncCBModem->pSim_Get_Fw_Version();
}

/*
    Func:Power ON module SIM
        + Giu chan Boot va Reset o muc Thap
        + Cap nguon cho module bang chan POW
*/

uint8_t SIM_POWER_ON(void)
{   
	switch (sSimVar.StepPowerOn_u8)
	{
		case 0:      
			UTIL_Log_Str(DBLEVEL_M, "\r\n=POWER ON SIM=\r\n" );
            MX_USART2_UART_Init();
            Init_Uart_Sim_Rx_IT();
            
            SIM_PW_OFF1;
            
            UTIL_TIMER_SetPeriod(&TimerControlBC66, ((sSimVar.CountHardReset_u8 % 10) * 1000) + 3000);
            UTIL_TIMER_Start(&TimerControlBC66);
			break;
        case 1:   
            SIM_PWKEY_OFF1;
            SIM_RESET_OFF1;
            SIM_PW_ON1;

			UTIL_TIMER_SetPeriod(&TimerControlBC66, 1000);
            UTIL_TIMER_Start(&TimerControlBC66);
			break;
        case 2:      
            SIM_PWKEY_ON1;
        #ifdef SIM_L506_LIB
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 500);
            UTIL_TIMER_Start(&TimerControlBC66);
        #else 
            UTIL_TIMER_SetPeriod(&TimerControlBC66, 3000);
            UTIL_TIMER_Start(&TimerControlBC66);
        #endif
			break;
        case 3:      
            SIM_PWKEY_OFF1;
        #ifdef SIM_L506_LIB
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 10000);
            UTIL_TIMER_Start(&TimerControlBC66);
        #else
            UTIL_TIMER_SetPeriod(&TimerControlBC66, 5000);
            UTIL_TIMER_Start(&TimerControlBC66);
        #endif
			break;
		default:
			sSimVar.StepPowerOn_u8 = 0;
            UTIL_Printf_Str( DBLEVEL_M, "\r\n=POWER ON SIM OK=\r\n" );
			return 1;
	}

	sSimVar.StepPowerOn_u8++;

	return 0;
}
            


void Sim_Push_AT_Publish (void)
{
    fPushBlockSimStepToQueue(aSIM_STEP_PUBLISH, sizeof(aSIM_STEP_PUBLISH)); 
}

void Sim_Push_AT_Publish_Fb (void)
{
    fPushBlockSimStepToQueue(aSIM_STEP_PUBLISH_FB, sizeof(aSIM_STEP_PUBLISH));
}


/*
    Func: Set infor Server main
*/
void Sim_Set_Server_Infor_Main (sData sIP, sData Port, sData sUserName, sData sPass)
{
    uint16_t i = 0; 
    
    //Must memset avoid error char*
    UTIL_MEM_set(&aIP_SERVER_MAIN, 0, sizeof(aIP_SERVER_MAIN));
    UTIL_MEM_set(&aPORT_SERVER_MAIN, 0, sizeof(aPORT_SERVER_MAIN));
    UTIL_MEM_set(&aUSER_MQTT_MAIN, 0, sizeof(aUSER_MQTT_MAIN));
    UTIL_MEM_set(&aPASS_MQTT_MAIN, 0, sizeof(aPASS_MQTT_MAIN)); 
    //Set IP
    sSimInfor.sServer.sIPMain.Length_u16 = sIP.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sIPMain.Length_u16; i++)
        *(sSimInfor.sServer.sIPMain.Data_a8 + i) = *(sIP.Data_a8 + i);
    //Set Port
    sSimInfor.sServer.sPortMain.Length_u16 = Port.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sPortMain.Length_u16; i++)
        *(sSimInfor.sServer.sPortMain.Data_a8 + i) = *(Port.Data_a8 + i);
    //Set User
    sSimInfor.sServer.sUserNameMQTTMain.Length_u16 = sUserName.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sUserNameMQTTMain.Length_u16; i++)
        *(sSimInfor.sServer.sUserNameMQTTMain.Data_a8 + i) = *(sUserName.Data_a8 + i);
    //Set User
    sSimInfor.sServer.sPasswordMQTTMain.Length_u16 = sPass.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sPasswordMQTTMain.Length_u16; i++)
        *(sSimInfor.sServer.sPasswordMQTTMain.Data_a8 + i) = *(sPass.Data_a8 + i);
}


/*
    Func: Get infor Server main
*/
uint8_t Sim_Get_Server_Infor_Main (uint8_t *pData)
{
    uint16_t i = 0; 
    uint8_t Length = 0;
    //Packet IP
	for(i = 0; i < sSimInfor.sServer.sIPMain.Length_u16; i++)
		pData[Length++] = *(sSimInfor.sServer.sIPMain.Data_a8 + i);
	//Packet Port
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sPortMain.Length_u16; i++)
		pData[Length++] = *(sSimInfor.sServer.sPortMain.Data_a8 + i);
	//Packet User
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sUserNameMQTTMain.Length_u16; i++)
		pData[Length++] = '*';   // *(sSimInfor.sServer.sUserNameMQTT.Data_a8 + i);
	//Packet Pass
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sPasswordMQTTMain.Length_u16; i++)
		pData[Length++] = '*';    //  *(sSimInfor.sServer.sPasswordMQTT.Data_a8 + i);
    
    return Length;
}




/*
    Func: Set infor Server Backup
*/
void Sim_Set_Server_Infor_Backup (sData sIP, sData Port, sData sUserName, sData sPass)
{
    uint16_t i = 0; 
    
    //Must memset avoid error char*
    UTIL_MEM_set(&aIP_SERVER_BACKUP, 0, sizeof(aIP_SERVER_BACKUP));
    UTIL_MEM_set(&aPORT_SERVER_BACKUP, 0, sizeof(aPORT_SERVER_BACKUP));
    UTIL_MEM_set(&aUSER_MQTT_BACKUP, 0, sizeof(aUSER_MQTT_BACKUP));
    UTIL_MEM_set(&aPASS_MQTT_BACKUP, 0, sizeof(aPASS_MQTT_BACKUP)); 
    //Set IP
    sSimInfor.sServer.sIPBackUp.Length_u16 = sIP.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sIPBackUp.Length_u16; i++)
        *(sSimInfor.sServer.sIPBackUp.Data_a8 + i) = *(sIP.Data_a8 + i);
    //Set Port
    sSimInfor.sServer.sPortBackUp.Length_u16 = Port.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sPortBackUp.Length_u16; i++)
        *(sSimInfor.sServer.sPortBackUp.Data_a8 + i) = *(Port.Data_a8 + i);
    //Set User
    sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16 = sUserName.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16; i++)
        *(sSimInfor.sServer.sUserNameMQTTBackUp.Data_a8 + i) = *(sUserName.Data_a8 + i);
    //Set User
    sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16 = sPass.Length_u16;
    for (i = 0; i < sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16; i++)
        *(sSimInfor.sServer.sPasswordMQTTBackUp.Data_a8 + i) = *(sPass.Data_a8 + i);
}


/*
    Func: Get infor Server backup
*/
uint8_t Sim_Get_Server_Infor_Backup (uint8_t *pData)
{
    uint16_t i = 0; 
    uint8_t Length = 0;
    //Packet IP
	for(i = 0; i < sSimInfor.sServer.sIPBackUp.Length_u16; i++)
		pData[Length++] = *(sSimInfor.sServer.sIPBackUp.Data_a8 + i);
	//Packet Port
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sPortBackUp.Length_u16; i++)
		pData[Length++] = *(sSimInfor.sServer.sPortBackUp.Data_a8 + i);
	//Packet User
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16; i++)
		pData[Length++] = '*';   // *(sSimInfor.sServer.sUserNameMQTTBackUp.Data_a8 + i);
	//Packet Pass
	pData[Length++] = ',';
	for(i = 0; i < sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16; i++)
		pData[Length++] = '*';    //  *(sSimInfor.sServer.sPasswordMQTTBackUp.Data_a8 + i);
    
    return Length;
}

Struct_Sim_Information *pSim_Get_Infor (void)
{
    return &sSimInfor;
}

/*============ Func Init ============*/

/*
 * Func: Init IP va Port server
 * */

void Init_IP_Port_Server(void)
{
	uint8_t     temp = 0;
	uint8_t 	PosGet = 0;   //Vi tri cat data ra

	//MQTT server info
	temp = *(__IO uint8_t*)ADDR_SERVER_INFOR;
	if (temp != FLASH_BYTE_EMPTY)
	{
		//Get IP
		PosGet = 1;
        Flash_Get_Infor (ADDR_SERVER_INFOR + PosGet, sSimInfor.sServer.sIPMain.Data_a8, &sSimInfor.sServer.sIPMain.Length_u16, MAX_IP_LENGTH);
		PosGet = PosGet + 1 + sSimInfor.sServer.sIPMain.Length_u16;
        //Get Port
        Flash_Get_Infor (ADDR_SERVER_INFOR + PosGet, sSimInfor.sServer.sPortMain.Data_a8, &sSimInfor.sServer.sPortMain.Length_u16, MAX_PORT_LENGTH);
        PosGet = PosGet + 1 + sSimInfor.sServer.sPortMain.Length_u16;
        //Get User
        Flash_Get_Infor (ADDR_SERVER_INFOR + PosGet, sSimInfor.sServer.sUserNameMQTTMain.Data_a8, &sSimInfor.sServer.sUserNameMQTTMain.Length_u16, MAX_USER_PASS_LENGTH);
        PosGet = PosGet + 1 + sSimInfor.sServer.sUserNameMQTTMain.Length_u16;
		//Get Pass MQTT
        Flash_Get_Infor (ADDR_SERVER_INFOR + PosGet, sSimInfor.sServer.sPasswordMQTTMain.Data_a8, &sSimInfor.sServer.sPasswordMQTTMain.Length_u16, MAX_USER_PASS_LENGTH);
	} else
		Save_Server_Infor();

    sSimInfor.sServer.KeepAlive_u32 = 60;
}



void Init_Server_BackUp_Infor(void)
{
	uint8_t     temp = 0;
	uint8_t 	PosGet = 0;   //Vi tri cat data ra

	//MQTT server info
	temp = *(__IO uint8_t*)ADDR_SERVER_BACKUP;
	if (temp != FLASH_BYTE_EMPTY)
	{
        //Get IP
		PosGet = 1;
        Flash_Get_Infor (ADDR_SERVER_BACKUP + PosGet, sSimInfor.sServer.sIPBackUp.Data_a8, &sSimInfor.sServer.sIPBackUp.Length_u16, MAX_IP_LENGTH);
		PosGet = PosGet + 1 + sSimInfor.sServer.sIPBackUp.Length_u16;
        //Get Port
        Flash_Get_Infor (ADDR_SERVER_BACKUP + PosGet, sSimInfor.sServer.sPortBackUp.Data_a8, &sSimInfor.sServer.sPortBackUp.Length_u16, MAX_PORT_LENGTH);
        PosGet = PosGet + 1 + sSimInfor.sServer.sPortBackUp.Length_u16;
        //Get User
        Flash_Get_Infor (ADDR_SERVER_BACKUP + PosGet, sSimInfor.sServer.sUserNameMQTTBackUp.Data_a8, &sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16, MAX_USER_PASS_LENGTH);
        PosGet = PosGet + 1 + sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16;
		//Get Pass MQTT
        Flash_Get_Infor (ADDR_SERVER_BACKUP + PosGet, sSimInfor.sServer.sPasswordMQTTBackUp.Data_a8, &sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16, MAX_USER_PASS_LENGTH);
	} else
		Save_Server_BackUp_Infor();
}

void Save_Server_Infor (void)
{
	uint8_t     aTEMP_SEVER_INFOR[64] = {0};
	uint8_t 	PosGet = 0;   //Vi tri cat data ra
	uint16_t    i = 0;

	aTEMP_SEVER_INFOR[PosGet++] = BYTE_TEMP_FIRST;
	//Packet IP
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sIPMain.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sIPMain.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sIPMain.Data_a8 + i);
	//Packet Port
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sPortMain.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sPortMain.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sPortMain.Data_a8 + i);
	//Packet User
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sUserNameMQTTMain.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sUserNameMQTTMain.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sUserNameMQTTMain.Data_a8 + i);
	//Packet Pas
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sPasswordMQTTMain.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sPasswordMQTTMain.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sPasswordMQTTMain.Data_a8 + i);
	//Luu
    OnchipFlashPageErase(ADDR_SERVER_INFOR);
	OnchipFlashWriteData(ADDR_SERVER_INFOR, aTEMP_SEVER_INFOR, 64);
}



void Save_Server_BackUp_Infor (void)
{
	uint8_t     aTEMP_SEVER_INFOR[64] = {0};
	uint8_t 	PosGet = 0;   //Vi tri cat data ra
	uint16_t    i = 0;

	aTEMP_SEVER_INFOR[PosGet++] = BYTE_TEMP_FIRST;
	//Packet IP
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sIPBackUp.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sIPBackUp.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sIPBackUp.Data_a8 + i);
	//Packet Port
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sPortBackUp.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sPortBackUp.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sPortBackUp.Data_a8 + i);
	//Packet User
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sUserNameMQTTBackUp.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sUserNameMQTTBackUp.Data_a8 + i);
	//Packet Pas
	aTEMP_SEVER_INFOR[PosGet++] = sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16;
	for (i = 0; i < sSimInfor.sServer.sPasswordMQTTBackUp.Length_u16; i++)
		aTEMP_SEVER_INFOR[PosGet++] = *(sSimInfor.sServer.sPasswordMQTTBackUp.Data_a8 + i);
	//Luu
    OnchipFlashPageErase(ADDR_SERVER_BACKUP);
	OnchipFlashWriteData(ADDR_SERVER_BACKUP, aTEMP_SEVER_INFOR, 64);
}





/*===============Func Handler Module SIM=============*/

/*
    Func: Init Func Pointer handler to L506
*/

void Sim_Init_Handler_Process (void)
{   
    //Init Func Pointer Component module common: L506, BC66...
    sSimCommFuncCallBack = &sSimCallBackHandler;
}

void Sim_Init(void)
{
    //Init Server Infor
	Init_IP_Port_Server();
    Init_Server_BackUp_Infor();
    //Init Queue Sim Step
    qQueue_Create (&qSimStep, SIM_MAX_ITEM_QUEUE, sizeof (uint8_t), (uint8_t *) &aQSimStepComtrol); 
    //Set Callback Handler to L506,...
    Sim_Init_Handler_Process();
	//Khoi tao timer moudle sim: TỉmerATsend, TimerATSendOK...
	UTIL_TIMER_Create(&TimerSendTimeout, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerSendTimeOutEvent, NULL);
	UTIL_TIMER_SetPeriod(&TimerSendTimeout, sEventSim[_EVENT_SIM_AT_SEND_TIMEOUT].e_period); 

	UTIL_TIMER_Create(&TimerNextSendAT, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerNextSendATEvent, NULL);
	UTIL_TIMER_SetPeriod(&TimerNextSendAT, sEventSim[_EVENT_SIM_AT_SEND].e_period);
    
    Sim_Init_Timer_PowerOn();  
	//Bat dau active task SIM_TURN_ON
	fevent_active(sEventSim, _EVENT_SIM_TURN_ON);
}


uint8_t Sim_Task(void)
{
	uint8_t Result = 0;

	uint8_t i = 0;

	for (i = 0; i < _EVENT_SIM_END; i++)
	{
		if (sEventSim[i].e_status == 1)
		{
			Result = 1;
			if ((sEventSim[i].e_systick == 0) || ((HAL_GetTick() - sEventSim[i].e_systick) >= sEventSim[i].e_period))
			{
                sEventSim[i].e_status = 0;   //Disable event
				sEventSim[i].e_systick = HAL_GetTick();
				sEventSim[i].e_function_handler(i);
                
                sSimVar.LastTimeExcuteEvent_u32 = HAL_GetTick(); 
			}
		}
	}

	return Result;
}





uint8_t Sim_Analys_Error (void)
{
    if (sSimCommon.RxPinReady_u8 == FALSE)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error power or rx pin uart\r\n" );
        return _SIM_ERROR_PIN_RX;
    }
    
    if (sSimCommon.TxPinReady_u8 == FALSE)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error tx pin uart\r\n" );
        return _SIM_ERROR_PIN_TX;
    }
    
    if (sSimCommon.CallReady_u8 == FALSE)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error card sim\r\n" );
        return _SIM_ERROR_CARD;
    }
    
    if (sSimCommon.NetReady_u8 == FALSE)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error network\r\n" );
        return _SIM_ERROR_ATTACH;
    }
    
    if (sSimCommon.ServerReady_u8 == FALSE)  
    {
        UTIL_Printf_Str( DBLEVEL_M, "u_sim: error connect server\r\n" );
        return _SIM_ERROR_TCP;
    }
    
    return _SIM_ERROR_NONE;
}


void Sim_Defaul_Struct_GPS (void)
{
    Sim_Common_Default_Struct_GPS();
}


void Sim_Clear_Queue_Step (void)
{
    sSimVar.IsRunningATcmd_u8 = false;
    qQueue_Clear(&qSimStep);
}






