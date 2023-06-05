

#include "user_app_lora.h"

#include "platform.h"
#include "sys_app.h"
#include "radio.h"
#include "app_version.h"

#include "stm32_seq.h"
#include "utilities_def.h"

#include "user_time.h"
#include "user_message.h"
#include "user_timer.h"




/*======================== Define ========================*/
#define MAX_APP_BUFFER_SIZE          255
#if (PAYLOAD_LEN > MAX_APP_BUFFER_SIZE)
    #error PAYLOAD_LEN must be less or equal than MAX_APP_BUFFER_SIZE
#endif /* (PAYLOAD_LEN > MAX_APP_BUFFER_SIZE) */

/* wait for remote to be in Rx, before sending a Tx frame*/
#define RX_TIME_MARGIN                50
/* Afc bandwidth in Hz */
#define FSK_AFC_BANDWIDTH             83333
/* LED blink Period*/
#define LED_PERIOD_MS                 200


/*============= Static Function====================*/

/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

static uint8_t _Cb_Lora_Tx_Process (uint8_t event);
static uint8_t _Cb_Lora_IRQ_Process (uint8_t event);
static uint8_t _Cb_Lora_Ctrl_Led2 (uint8_t event);
static uint8_t _Cb_Check_Mode(uint8_t event);

/*====================Var struct =================*/
static RadioEvents_t RadioEvents;
/*Ping Pong FSM states */
static States_t State = RX;
/* App Rx Buffer*/
static uint8_t BufferRx[MAX_APP_BUFFER_SIZE];
/*Buffer tx lora: Header and Payload */
static uint8_t aLORA_TX [MAX_APP_BUFFER_SIZE];

uint8_t aINTAN_DATA[MAX_APP_BUFFER_SIZE];

static uint16_t RxBufferSize = 0;

static UTIL_TIMER_Object_t TimerLed1;
static void _Cb_Timer_Led_Event(void *context);

static UTIL_TIMER_Object_t TimerLoraTx;
static void _Cb_Timer_Lora_Tx(void *context);

StructLoraManager    sLoraVar =
{
    .sRecv = {&BufferRx[0], 0},
    .sIntanData = {&aINTAN_DATA[0], 0},
};

sEvent_struct sEventAppLora[] =
{
	{ _EVENT_LORA_TX, 		    0, 0, 50, 	    _Cb_Lora_Tx_Process },  
    { _EVENT_LORA_IRQ, 		    0, 0, 0, 	    _Cb_Lora_IRQ_Process }, 
    
    { _EVENT_CTRL_LED2, 	    1, 0, 500, 	    _Cb_Lora_Ctrl_Led2 }, 
    { _EVENT_CHECK_MODE,		1, 0, 1000,     _Cb_Check_Mode },    
};

StructLoraHandle  shLora;

#ifdef DEVICE_TYPE_GATEWAY
    GPIO_TypeDef *aLED_PORT[LEDn] = {LED_1_GPIO_Port, LED_2_GPIO_Port};
    const uint16_t aLED_PIN[LEDn] = {LED_1_Pin, LED_2_Pin};  
#endif

/*===================== Function =========================*/


/* Exported functions ---------------------------------------------------------*/
void AppLora_Init(void)
{
    uint32_t random_delay = 0;
    
    /* USER CODE BEGIN SubghzApp_Init_1 */
#ifdef DEVICE_TYPE_GATEWAY
    APP_LOG(TS_OFF, VLEVEL_M, "\n\r***THIET BI GATEWAY***\n\r");
#else
    APP_LOG(TS_OFF, VLEVEL_M, "\n\r***THIET BI NODE***\n\r");
#endif
    /* Print APP version*/
    APP_LOG(TS_OFF, VLEVEL_M, "APP_VERSION= V%X.%X.%X\r\n",
            (uint8_t)(__APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
            (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB1_SHIFT),
            (uint8_t)(__APP_VERSION >> __APP_VERSION_SUB2_SHIFT));
    
    /* Radio initialization */
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    
    Radio.Init(&RadioEvents);
      
    /*calculate random delay for synchronization*/
    random_delay = (Radio.Random()) >> 22; /*10bits random e.g. from 0 to 1023 ms*/
    
    /* USER CODE BEGIN SubghzApp_Init_2 */
    /* Radio Set frequency */
    Radio.SetChannel(RF_FREQUENCY);
    
    /* Radio configuration */
#if ((USE_MODEM_LORA == 1) && (USE_MODEM_FSK == 0))
    APP_LOG(TS_OFF, VLEVEL_M, "---------------\n\r");
    APP_LOG(TS_OFF, VLEVEL_M, "LORA_MODULATION\n\r");
    APP_LOG(TS_OFF, VLEVEL_M, "LORA_BW=%d kHz\n\r", (1 << LORA_BANDWIDTH) * 125);
    APP_LOG(TS_OFF, VLEVEL_M, "LORA_SF=%d\n\r", LORA_SPREADING_FACTOR);
    
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
    
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    
    Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);
    
#elif ((USE_MODEM_LORA == 0) && (USE_MODEM_FSK == 1))
    APP_LOG(TS_OFF, VLEVEL_M, "---------------\n\r");
    APP_LOG(TS_OFF, VLEVEL_M, "FSK_MODULATION\n\r");
    APP_LOG(TS_OFF, VLEVEL_M, "FSK_BW=%d Hz\n\r", FSK_BANDWIDTH);
    APP_LOG(TS_OFF, VLEVEL_M, "FSK_DR=%d bits/s\n\r", FSK_DATARATE);
    
    Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                      FSK_DATARATE, 0,
                      FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, 0, TX_TIMEOUT_VALUE);
    
    Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                      0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                      0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                      0, 0, false, true);
    
    Radio.SetMaxPayloadLength(MODEM_FSK, MAX_APP_BUFFER_SIZE);
    
#else
#error "Please define a modulation in the subghz_phy_app.h file."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */ 
    APP_LOG(TS_ON, VLEVEL_L, "rand=%d\n\r", random_delay);
    
    UTIL_TIMER_Create(&TimerLed1, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, _Cb_Timer_Led_Event, NULL);
    UTIL_TIMER_SetPeriod(&TimerLed1, LED_PERIOD_MS);
    
    UTIL_TIMER_Create(&TimerLoraTx, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, _Cb_Timer_Lora_Tx, NULL);
    
#ifdef DEVICE_TYPE_GATEWAY
    Radio.Rx(RX_TIMEOUT_VALUE);
#else
    //Active Intan
    sMessVar.aMARK[_MESS_INTAN] = TRUE;
    fevent_active(sEventAppLora, _EVENT_LORA_TX);
    //Active Event Send stime
    sCtrlLora.CtrlStime_u8 = TRUE;
#endif
    /* USER CODE END SubghzApp_Init_2 */
}


/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
    Radio.Sleep();
    /* USER CODE BEGIN OnTxDone */

    /* Update the State of the FSM*/
    State = TX;
    /* Run PingPong process in background*/
    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    Radio.Sleep();
    /* Update the State of the FSM*/
    State = RX;
    /* Clear BufferRx*/
    memset(BufferRx, 0, MAX_APP_BUFFER_SIZE);
    /* Record payload size*/
    RxBufferSize = size;
    if (RxBufferSize <= MAX_APP_BUFFER_SIZE)
    {
        memcpy(BufferRx, payload, RxBufferSize);
    }
    /* Record Received Signal Strength*/
    sLoraVar.Snr_u8 = LoraSnr_FskCfo;
    sLoraVar.Rssi_u8 = rssi;
    /* Run PingPong process in background*/
    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
    Radio.Sleep();
    /* USER CODE BEGIN OnTxTimeout */

    /* Update the State of the FSM*/
    State = TX_TIMEOUT;
    /* Run PingPong process in background*/
    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
    Radio.Sleep();
    /* USER CODE BEGIN OnRxTimeout */
    /* Update the State of the FSM*/
    State = RX_TIMEOUT;
    /* Run PingPong process in background*/
    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
    Radio.Sleep();
    /* USER CODE BEGIN OnRxError */
    
    /* Update the State of the FSM*/
    State = RX_ERROR;
    /* Run PingPong process in background*/
    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    /* USER CODE END OnRxError */
}

static void _Cb_Timer_Led_Event(void *context)
{
    LED_OFF(__LED1);
}

static void _Cb_Timer_Lora_Tx(void *context)
{
    if (AppLora_Check_New_Mess() != FALSE)
    {
        fevent_active(sEventAppLora, _EVENT_LORA_TX);  
    }
}


/*============ Function user add ==========*/
static uint8_t _Cb_Lora_IRQ_Process (uint8_t event)
{
    Radio.Sleep();
    
    switch (State)
    {
        case RX:
            APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
            #if ((USE_MODEM_LORA == 1) && (USE_MODEM_FSK == 0))
                APP_LOG(TS_ON, VLEVEL_L, "RssiValue=%d dBm, SnrValue=%ddB\n\r", sLoraVar.Rssi_u8, sLoraVar.Snr_u8);
            #endif /* USE_MODEM_LORA | USE_MODEM_FSK */
                
            #if ((USE_MODEM_LORA == 0) && (USE_MODEM_FSK == 1))
                APP_LOG(TS_ON, VLEVEL_L, "RssiValue=%d dBm, Cfo=%dkHz\n\r", sLoraVa.Rssi_u8, sLoraVar.Snr_u8);
                sLoraVar.Snr_u8 = 0; /*not applicable in GFSK*/
            #endif /* USE_MODEM_LORA | USE_MODEM_FSK */
                
            /* Record payload content*/
            APP_LOG(TS_ON, VLEVEL_L, "Data received:");
            for (int i = 0; i < RxBufferSize; i++)
            {
                APP_LOG(TS_OFF, VLEVEL_L, "%02X", BufferRx[i]);
            }
            APP_LOG(TS_OFF, VLEVEL_L, "\n\r");
            
        #ifdef DEVICE_TYPE_GATEWAY
            
            if (Lora_Pro_Check_Rx(__DEVICE_GW, BufferRx, RxBufferSize, 0, &sLoraVar.sFrameRx) == TRUE)
            {
                LED_ON(__LED1); 
                UTIL_TIMER_Start(&TimerLed1);
                fevent_active(sEventAppLora, _EVENT_LORA_TX);
            }
            
            Radio.Rx(RX_TIMEOUT_VALUE);
        #else
            if (Lora_Pro_Check_Rx(__DEVICE_NODE, BufferRx, RxBufferSize, 0, &sLoraVar.sFrameRx) == TRUE)
            {
                //Send OK: Increase Index Send
                mCallback_Send_OK(); 
                mMess_Handle_Reset(sMessVar.DataType_u8);
                
                if (AppLora_Check_New_Mess() != FALSE)
                {
                    fevent_enable(sEventAppLora, _EVENT_LORA_TX);  //Thay bang timer: de cho send cach ra 1 khoang thoi gian
                }
            }
        #endif
            break;
        case TX:
            APP_LOG(TS_ON, VLEVEL_L, "OnTxDone\n\r");
            APP_LOG(TS_ON, VLEVEL_L, "Rx start\n\r");
            Radio.Rx(RX_TIMEOUT_VALUE);
            break;
        case RX_TIMEOUT:
            APP_LOG(TS_ON, VLEVEL_L, "OnRxTimeout\n\r");
        case RX_ERROR:
            APP_LOG(TS_ON, VLEVEL_L, "OnRxError\n\r");
        #ifdef DEVICE_TYPE_GATEWAY
            APP_LOG(TS_ON, VLEVEL_L, "Rx start again\n\r");
            Radio.Rx(RX_TIMEOUT_VALUE);
        #else
            //Send OK: Increase Index Send
            if (sMessVar.MessType_u8[sMessVar.DataType_u8] == FRAME_TYPE_DATA_CONFIRMED_UP)
            {
                uint8_t StatusFail = mCb_Send_Fail();
                //Neu Fail Max Retry -> Increase Index, or retry after 10s
                if (StatusFail == FALSE)      
                {
                    AppLora_Set_Timer_Tx(TIME_RETRY_SEND_CONFIRM);
                    break;
                } else if (StatusFail == PENDING)
                {
                    //Neu gui lai den khi thanh cong -> Disable TX
                    break;
                }
            } 
            
            mCallback_Send_OK();  
            mMess_Handle_Reset(sMessVar.DataType_u8);
            
            if (AppLora_Check_New_Mess() != FALSE)
            {
                fevent_active(sEventAppLora, _EVENT_LORA_TX); 
            }
        #endif
            break;
        case TX_TIMEOUT:
            APP_LOG(TS_ON, VLEVEL_L, "OnTxTimeout\n\r");
            break;
        default:
            break;
    }
    
    return 1;
}



/*
    Func: Set Mode Control Led
        - Mode: + _LED_M_POWER      :   Nhay deu 1s 
                + _LED_M_TX         :
                + _LED_M_CARD       :
                + _LED_M_NET        :
                + _LED_M_TCP_INIT   :
                + _LED_M_CONN_SER   :   Nhay Duty: 430ms off, 70ms on
                + _LED_M_UP_FW      :   Nhay deu 100ms
                + _LED_M_SAVE       :   Off led
*/
uint8_t AppLora_Set_Led_Mode (void)
{
    #ifdef USING_APP_SIM
        if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE) 
        {
            if (sSimCommon.RxPinReady_u8 == FALSE)  
                return _LED_M_POWER;
            
            if (sSimCommon.TxPinReady_u8 == FALSE)  
                return _LED_M_TX;
            
            if (sSimCommon.CallReady_u8 == FALSE)  
                return _LED_M_CARD;

            if (sSimCommon.NetReady_u8 == FALSE)  
                return _LED_M_NET;
            
            if (sSimCommon.ServerReady_u8 == FALSE)  
                return _LED_M_TCP_INIT;
            
            if ((sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_MAIN) && (sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_BACKUP))  
                return _LED_M_UP_FW;
            
            return _LED_M_CONN_SER;
        } else 
        {
            return _LED_M_SAVE; 
        }
    #else
        return _LED_M_SAVE;
    #endif
}


/*Control led status*/
static uint8_t _Cb_Lora_Ctrl_Led2 (uint8_t event)
{
#ifdef DEVICE_TYPE_GATEWAY
    static uint8_t CountNhay = 0;
    
    //Handler in step control one
    switch ( AppLora_Set_Led_Mode() )
    {
        case _LED_M_POWER:
            LED_TOGGLE(__LED2);
            sEventAppLora[event].e_period = 1000;
            fevent_enable(sEventAppLora, event);
            break;
        case _LED_M_TX:
            LED_TOGGLE(__LED2);
            CountNhay++;
            if (CountNhay < 12)
            {
                sEventAppLora[event].e_period = 200;
            } else
            {
                CountNhay = 0;
                sEventAppLora[event].e_period = 2000;
            }
            
            fevent_enable(sEventAppLora, event);
            
            break;
        case _LED_M_CARD:
            LED_TOGGLE(__LED2);
            CountNhay++;
            if (CountNhay < 8)
            {
                sEventAppLora[event].e_period = 200;
            } else
            {
                CountNhay = 0;
                sEventAppLora[event].e_period = 2000;
            }
            
            fevent_enable(sEventAppLora, event);
            break;
        case _LED_M_NET:
            LED_TOGGLE(__LED2);
            sEventAppLora[event].e_period = 2000;
            fevent_enable(sEventAppLora, event);
            break;  
        case _LED_M_TCP_INIT:
            LED_TOGGLE(__LED2);
            CountNhay++;
            if (CountNhay < 4)
            {
                sEventAppLora[event].e_period = 200;
            } else
            {
                CountNhay = 0;
                sEventAppLora[event].e_period = 2000;
            }
            
            fevent_enable(sEventAppLora, event);
            break; 
        case _LED_M_CONN_SER:
            if (sEventAppLora[event].e_period != 430) 
            {
                LED_OFF(__LED2); 
                sEventAppLora[event].e_period = 430;
            } else
            {
                LED_ON(__LED2); 
                sEventAppLora[event].e_period = 70;
            }
            fevent_enable(sEventAppLora, event);
            break;
        case _LED_M_UP_FW:
            LED_TOGGLE(__LED2);
            sEventAppLora[event].e_period = 100;
            fevent_enable(sEventAppLora, event);
            break;      
        case _LED_M_SAVE:
            LED_OFF(__LED2); 
            break;
        default:
            fevent_enable(sEventAppLora, event);
            break;
    }
    
#endif
    
    return 1;
}



static uint8_t _Cb_Check_Mode(uint8_t event)
{
#ifdef DEVICE_TYPE_GATEWAY
    //Read Pin Power 5V to detect change mode
    #ifdef USING_APP_SIM         
        if (sSimVar.StepPowerOn_u8 == 0)
        {
            if (AppLora_Check_Mode(event) == true)
            {
            #ifdef USING_APP_SIM
                Sim_Disable_All_Event();
                fevent_active(sEventAppSim, _EVENT_SIM_SEND_MESS);
            #endif
            }
        } else
        {
            fevent_enable(sEventAppLora, event); 
        }

        if ((sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_MAIN) && (sSimVar.ModeConnectNow_u8 != MODE_CONNECT_DATA_BACKUP))
            fevent_enable(sEventAppLora, _EVENT_CTRL_LED2);        
    #endif
        
    /*======= Cho Offsim khong cho chip di ngu =============*/
    sModem.IRQPowerOutAgain_u8 = TRUE;
    
#endif
    return 1;
}


uint8_t AppLora_Check_Mode (uint8_t event)   
{
#ifdef DEVICE_TYPE_GATEWAY
    static uint8_t CountCheckMode = 0;
    static uint8_t LastMode = _POWER_MODE_ONLINE;
    uint8_t Result = FALSE;
    
//    AppLora_Get_Vout_mV();
//
//    //Neu dien ap ngoai > 4V la nguon ngoai: con lai la nguon trong
//    if (sVout.mVol_u32 > 4000)

    if (POWER_5V_STATUS == false)
    {
        if (LastMode == _POWER_MODE_SAVE)
        {
            CountCheckMode++;
            if (CountCheckMode >= 2)
            {
                CountCheckMode = 0;
                Modem_Log_And_Send_Emergency((uint8_t*) "=Change To Mode Running=\r\n", 26);
                sModem.ModeSimPower_u8 = _POWER_MODE_ONLINE;
                
                LastMode = sModem.ModeSimPower_u8;
                Result = TRUE;
            } else
            {     
                //enable to check again
                fevent_enable(sEventAppLora, event);
            }
        } else
        {
            CountCheckMode = 0;
        }
        
    } else
    {
        if (LastMode == _POWER_MODE_ONLINE)
        {
            CountCheckMode++;
            if (CountCheckMode >= 2)
            {
                Modem_Log_And_Send_Emergency( (uint8_t*) "=Change To Mode Save Power=\r\n", 29 );
                sModem.ModeSimPower_u8 = _POWER_MODE_SAVE;
                
                LastMode = sModem.ModeSimPower_u8;
                Result = TRUE;
            } else
            {
                //enable to check again
                fevent_enable(sEventAppLora, event);
            }
        } else
        {
            CountCheckMode = 0;
        }
    }
    
    return Result;
#else
    return 1;
#endif
}


uint16_t AppLora_Get_Vout_mV(void)
{
//    AdcInitialized = 0; 
//    sVout.mVol_u32 = Get_Value_ADC(ADC_CHANNEL_4);
//    sVout.mVol_u32 = sVout.mVol_u32 * 247 / 47;
//    sVout.Level_u16 = Get_Level_Voltage (sVout.mVol_u32, VDD_OUT_MAX, VDD_OUT_MIN); 
//   
//	return sVout.Level_u16;
    return 1;
}



/*
    Func Handler Tx: Ham xu ly callback event tx
        + Tx Data water meter
        + Respond to gw tx
        + Request cmd to gw
    * Packet Header:
    * Packet Payload:
*/

static uint8_t _Cb_Lora_Tx_Process (uint8_t event)
{
    uint8_t FrameTypeDataSend_u8 = FRAME_TYPE_DATA_DEFAULT;
    
    Radio.Sleep();
    

#ifdef DEVICE_TYPE_GATEWAY    
    sMessVar.sPayload.Length_u16 = 0;
    sMessVar.DataType_u8 = Lora_Pro_Master_Packet_Payload(sMessVar.sPayload.Data_a8, (uint8_t *) &sMessVar.sPayload.Length_u16);
           
    if (AppLora_Send(sMessVar.sPayload.Data_a8, sMessVar.sPayload.Length_u16, FrameTypeDataSend_u8, sMessVar.DataType_u8) == FALSE) 
    {
        APP_LOG(TS_ON, VLEVEL_L, "subghz_phy_app.c: no data send\n\r");
    }
#else   
    uint8_t TypeDataLora = _DATA_NONE;
    
    //check stime
    if (sRTC.year < 22)
    {
        sCtrlLora.CtrlStime_u8 = true;
    }
    
    if (sCtrlLora.CtrlResp_u8 == FALSE)
    {
        sMessVar.sPayload.Length_u16 = 0;
        
        sMessVar.DataType_u8 = mCheck_Mess_Mark();
        switch(sMessVar.DataType_u8)
        {
            case _MESS_TSVH:
                TypeDataLora = _DATA_TSVH;
                break;
            case _MESS_INTAN:
                TypeDataLora = _DATA_INTAN;
                break;
            case _MESS_EVENT:
                TypeDataLora = _DATA_EVENT;
                break;
            case _MESS_RESPONSE_AT:
                TypeDataLora = _DATA_RESPONSE;
                FrameTypeDataSend_u8 = FRAME_TYPE_DATA_UNCONFIRMED_UP;
                break;
            default:
                TypeDataLora = _DATA_NONE;
                FrameTypeDataSend_u8 = FRAME_TYPE_DATA_UNCONFIRMED_UP;
                if (AppLora_Check_New_Mess() != FALSE)
                {
                    fevent_enable(sEventAppLora, event);
                }
                break;
            
        }
        
        sMessVar.MessType_u8[sMessVar.DataType_u8] = FrameTypeDataSend_u8;  //Danh dau Mess gui la Confirm | Unconfirm
        if (AppLora_Send(sMessVar.sPayload.Data_a8, sMessVar.sPayload.Length_u16, FrameTypeDataSend_u8, TypeDataLora) == FALSE)
        {
            APP_LOG(TS_ON, VLEVEL_L, "subghz_phy_app.c: no data send\n\r");
        }
        
    } else
    {     
        if (AppLora_Send( (uint8_t *) "OK!", 3, FRAME_TYPE_DATA_DEFAULT, _DATA_RESPONSE) == FALSE)
        {
            APP_LOG(TS_ON, VLEVEL_L, "subghz_phy_app.c: no data send\n\r");
        }
    }

#endif
    
    return 1;
}


/*
    Send Data wm to lora phy
*/
uint8_t AppLora_Send (uint8_t *pData, uint8_t Length, uint8_t MessType, uint8_t DataType)
{
    uint8_t i = 0;
    uint8_t CheckACKRequest = 0;
    sData   sMessTx = {&aLORA_TX[0], 0};
    LoRaFrame_t sFrameMess;
    
    //Packet Header
    if (Length != 0)
    {
        CheckACKRequest = Lora_Pro_Packet_Header(&sFrameMess, MessType, DataType);
    } else
    {
        CheckACKRequest = Lora_Pro_Packet_Header(&sFrameMess, MessType, _DATA_NONE);     
    }
    //Neu co data truyen di, hoac 1 ACK, request -> Send
    if ( (Length != 0) || (CheckACKRequest == TRUE) )
    {
        UTIL_MEM_set( aLORA_TX, 0, sizeof(aLORA_TX) );
        //Copy Header
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = sFrameMess.Header.Value;
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = sFrameMess.FCtrl.Value;
        
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) (sFrameMess.DevAddr >> 24);
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) (sFrameMess.DevAddr >> 16);
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) (sFrameMess.DevAddr >> 8);
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) sFrameMess.DevAddr;
        
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) (sFrameMess.FCnt >> 8); 
        *(sMessTx.Data_a8 + sMessTx.Length_u16++) = (uint8_t) sFrameMess.FCnt;
        //Copy Payload
        for (i = 0; i < Length; i++)
        {
            if ( sMessTx.Length_u16 >= sizeof(aLORA_TX) )
                break;
            
            *(sMessTx.Data_a8 + sMessTx.Length_u16++) = *(pData + i);
        }
        //Send
        APP_LOG(TS_ON, VLEVEL_L, "subghz_phy_app.c: send data: ");
        for (i = 0; i < sMessTx.Length_u16; i++)
            APP_LOG(TS_OFF, VLEVEL_L, "%02X", sMessTx.Data_a8[i]);
        
        APP_LOG(TS_OFF, VLEVEL_L, "\n\r");
        
        sCtrlLora.MessCount_u16++;
        
        Radio.Send(sMessTx.Data_a8, sMessTx.Length_u16);
        
        return 1;
    }
    
    return 0;
}





/*
Func: Check Event Lora
*/
uint8_t AppLora_Process(void)
{
	uint8_t i = 0;
	uint8_t Result = 0;
    
	for (i = 0; i < _EVENT_LORA_END; i++)
	{
		if (sEventAppLora[i].e_status == 1)
		{
        #ifdef DEVICE_TYPE_GATEWAY 
            if ( (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE) && (i != _EVENT_CHECK_MODE) )
                Result = 1;
        #else
            Result = 1;
        #endif
			if ((sEventAppLora[i].e_systick == 0) ||
                ((HAL_GetTick() - sEventAppLora[i].e_systick)  >=  sEventAppLora[i].e_period))
			{
                sEventAppLora[i].e_status = 0;  //Disable event
				sEventAppLora[i].e_systick = HAL_GetTick();
				sEventAppLora[i].e_function_handler(i);
			}
		}
	}
    
    #ifdef DEVICE_TYPE_GATEWAY
        if ( (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE) && (sEventAppLora[_EVENT_CHECK_MODE].e_status == 0) )
            fevent_enable(sEventAppLora, _EVENT_CHECK_MODE);
    #endif
    
	return Result;
}



/*
    Func: Test Slave Rx decode
        1. Decode config sTime
           
*/



uint8_t App_Test_Decode_Slave (void)
{
    uint8_t i = 0;
    uint8_t aTEMP_SLAVE_RX[17] = {32, 0x00, '1', '2', '3', '4', 0x01, 0x02, 0x01, 0x06, 23, 01, 03, 12, 00, 00, 9}; //config stime
    
    State = RX;
    
    for (i = 0; i < 17; i++)
        BufferRx[i] = aTEMP_SLAVE_RX[i];
    
    RxBufferSize = 17;

    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    
    return 1;
}

uint8_t App_Test_Handler_Rx_Master (void)
{
    uint8_t i = 0;
    uint8_t aTEMP_MASTER_RX[17] = {0x0A, 0x08, '1', '2', '3', '4', 0x01, 0x02, 0x01, 0x06, 'a', 'b', 'c', 'd', 'e', 'f', 9};  //Req sTime + TSVH
    
    State = RX;
    
    for (i = 0; i < 17; i++)
        BufferRx[i] = aTEMP_MASTER_RX[i];
    
    RxBufferSize = 17;

    fevent_active(sEventAppLora, _EVENT_LORA_IRQ);
    
    return 1;
}


uint8_t App_Test_AtCmd_Server_ConFig (uint8_t *pData, uint8_t Length)
{
    uint8_t i = 0;
    
    shLora.rAtcmd_u8 = TRUE;
    for (i = 0; i < Length; i++)
        shLora.aDATA_CONFIG[i] = pData[i];
    
    shLora.LengthConfig_u8 = Length;
    
    return 1;
}



StructTestFunction   sTestFunc;

void AppLora_Func_Test (void)
{         
    if (sTestFunc.Start_u8 == TRUE)
    {
        sTestFunc.Start_u8 = FALSE;

        switch (sTestFunc.StepFunc_u8)
        {
            case 0:
            #ifdef DEVICE_TYPE_GATEWAY
                //App_Test_Handler_Rx_Master();
            #else
                sCtrlLora.CtrlStime_u8 = TRUE;  //Request get Time
                //App_Test_Decode_Slave();
            #endif
                break;
            case 1:
            #ifdef DEVICE_TYPE_GATEWAY
                App_Test_AtCmd_Server_ConFig(sTestFunc.aDATA_TEST, sTestFunc.LengthDataTest_u8);
            #endif
                break;
            default:
                break;
        }
    }    
}



void LED_ON (eLed_TypeDef Led)
{
#ifdef DEVICE_TYPE_GATEWAY
    HAL_GPIO_WritePin(aLED_PORT[Led], aLED_PIN[Led], GPIO_PIN_SET);
#endif
}


void LED_OFF (eLed_TypeDef Led)
{
#ifdef DEVICE_TYPE_GATEWAY
    HAL_GPIO_WritePin(aLED_PORT[Led], aLED_PIN[Led], GPIO_PIN_RESET);
#endif
}


void LED_TOGGLE (eLed_TypeDef Led)
{
#ifdef DEVICE_TYPE_GATEWAY
    HAL_GPIO_TogglePin(aLED_PORT[Led], aLED_PIN[Led]);
#endif
}







/*
    Func: Check New Mess in memory
        return: 0: Empty
                1: TSVH
                2: Event
*/

uint8_t AppLora_Check_New_Mess (void)
{
    if (sRecTSVH.IndexSend_u16 != sRecTSVH.IndexSave_u16)
    {
        sMessVar.aMARK[_MESS_TSVH] = TRUE;
        return 1;
    }
    
    if (sRecEvent.IndexSend_u16 != sRecEvent.IndexSave_u16)
    {
        sMessVar.aMARK[_MESS_EVENT] = TRUE;
        return 2;
    }
    
    return 0;
}

void AppLora_Deinit_IO_Radio (void)
{
    Sx_Board_IoDeInit();
}

void AppLora_Init_IO_Radio (void)
{
    Sx_Board_IoInit();
}


void AppLora_Set_Timer_Tx (uint32_t Time)
{
    if (Time != 0)
    {
        UTIL_TIMER_SetPeriod(&TimerLoraTx, Time);
        UTIL_TIMER_Start(&TimerLoraTx);
    } else
    {
        _Cb_Timer_Lora_Tx(NULL);
    }
}







