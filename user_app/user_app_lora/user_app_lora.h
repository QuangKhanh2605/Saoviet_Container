
#ifndef USER_APP_LORA_H
#define USER_APP_LORA_H

#include "user_util.h"

#include "event_driven.h"
#include "user_define.h"
#include "user_lora_protocol.h"
#include "user_message.h"

#include "user_adc.h"
#include "radio_board_if.h"

/*======================== Define ========================*/
#define USING_APP_LORA

#ifdef DEVICE_TYPE_GATEWAY
    #define RX_TIMEOUT_VALUE                3605000      
    #define TX_TIMEOUT_VALUE                2000    
    
    #define FRAME_TYPE_DATA_DEFAULT         FRAME_TYPE_DATA_UNCONFIRMED_DOWN
#else
    #define RX_TIMEOUT_VALUE                2000      
    #define TX_TIMEOUT_VALUE                2000    

    #define FRAME_TYPE_DATA_DEFAULT         FRAME_TYPE_DATA_CONFIRMED_UP
#endif



#define MAX_RETRY_SEND_LORA                 0xFF  //0xFF: Send Util Send OK    
#define MAX_RETRY_WITHOUT_SKIP              2             // Retry and not skip mess

#define TIME_RETRY_SEND_CONFIRM             10000

#ifdef DEVICE_TYPE_GATEWAY
    /*Read Logic Pin PA4 to detect power 5V*/
    #define POWER_5V_STATUS                 HAL_GPIO_ReadPin(VOL_IN_5V_GPIO_Port, VOL_IN_5V_Pin)
#endif
/*============== RF module ==============*/
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#define REGION_AS923

#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 /* Hz */
#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 /* Hz */

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 /* Hz */

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 /* Hz */

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 /* Hz */

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 /* Hz */

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 /* Hz */

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 /* Hz */

#else
#error "Please define a frequency band in the compiler options."
#endif /* REGION_XXxxx */

#define TX_OUTPUT_POWER                             0        /* dBm */

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       7         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         5         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define PAYLOAD_LEN                                 64



#define VDD_OUT_MAX             5000     
#define VDD_OUT_MIN             2500



/*====================== Structs var======================*/
typedef enum
{
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;



typedef enum
{
    __LED1,
    __LED2,    
}eLed_TypeDef;

/*====================== External Var struct =============*/    
typedef enum
{
	_EVENT_LORA_TX,
	_EVENT_LORA_IRQ,
    _EVENT_CTRL_LED2,
    _EVENT_CHECK_MODE,
    
    _EVENT_LORA_END,
}sTypeEventLora;

typedef struct
{
    int8_t          Rssi_u8;
    int8_t          Snr_u8;
    
    sData           sRecv;
    uint8_t         NewMessType_u8;  
    
    LoRaFrame_t     sFrameRx;
    sData           sIntanData;
}StructLoraManager;


typedef struct
{
    uint8_t     Ack_U8;
    uint8_t     ReqStime_u8;
    uint8_t     AdrReq_u8;    
}StructHeaderMessSet;


typedef struct
{
    uint8_t         rSetStime_u8;       //Mark request set stime to master
    uint8_t         rSetDuty_u8;        //Mark request set Duty
    uint8_t         rSetThreshold_u8;   //Mark request set Duty
    uint8_t         rPulse_u8;          //Mark request set Pulse
    
    uint8_t         rAtcmd_u8;                // Mark Last config Server
    uint8_t         aDATA_CONFIG[256];   //buff chua last Data config server
    uint8_t         LengthConfig_u8;      //Length    
}StructLoraHandle;


typedef struct
{
    uint8_t     Start_u8;
    uint8_t     StepFunc_u8;
    
    uint8_t     aDATA_TEST[48];
    uint8_t     LengthDataTest_u8;
}StructTestFunction;

typedef enum
{
    _LED_M_POWER,
    _LED_M_TX,
    _LED_M_CARD,
    _LED_M_NET,
    _LED_M_TCP_INIT,
    _LED_M_CONN_SER,
    _LED_M_UP_FW,
    _LED_M_SAVE,
}sKindModeLed;




extern StructLoraManager    sLoraVar;
extern sEvent_struct        sEventAppLora[];
extern StructLoraHandle     shLora;

extern StructTestFunction   sTestFunc;


/*===================== Function =========================*/
void    AppLora_Init(void);

uint8_t AppLora_Process(void);
uint8_t AppLora_Send (uint8_t *pData, uint8_t Length, uint8_t MessType, uint8_t DataType);

void    AppLora_Func_Test (void);

void LED_TOGGLE (eLed_TypeDef Led);
void LED_OFF (eLed_TypeDef Led);
void LED_ON (eLed_TypeDef Led);

uint8_t     AppLora_Check_New_Mess (void);
void        AppLora_Deinit_IO_Radio (void);
void        AppLora_Init_IO_Radio (void);
void        AppLora_Set_Timer_Tx (uint32_t Time);

uint8_t     AppLora_Check_Mode (uint8_t event);
uint16_t    AppLora_Get_Vout_mV(void);




#endif

