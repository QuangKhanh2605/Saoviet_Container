



#ifndef USER_APP_WM_H_
#define USER_APP_WM_H_


#include "user_util.h"
#include "event_driven.h"

#include "user_adc.h"


#define USING_APP_TEMH

#define MAX_SLAVE               5             

/*============= Define =====================*/
#define VDD_OUT_MAX             12000     
#define VDD_OUT_MIN             9000

#define VDD_BAT                 3600     
#define VDD_MIN                 2800

#define V12_IN_ON               HAL_GPIO_WritePin(ON_OFF_12V_GPIO_Port, ON_OFF_12V_Pin, GPIO_PIN_SET)
#define V12_IN_OFF              HAL_GPIO_WritePin(ON_OFF_12V_GPIO_Port, ON_OFF_12V_Pin, GPIO_PIN_RESET)

#define V_PIN_ON                HAL_GPIO_WritePin(ON_OFF_BOOT_GPIO_Port, ON_OFF_BOOT_Pin, GPIO_PIN_SET)
#define V_PIN_OFF               HAL_GPIO_WritePin(ON_OFF_BOOT_GPIO_Port, ON_OFF_BOOT_Pin, GPIO_PIN_RESET)


#define SLAVE_ID_DEFAULT        0x1A
#define NUM_SLAVE_DEFAULT       2

#define MAX_POINT_CALIB         14
#define DEFAULT_POINT_CALIB     11
#define REAL                    0
#define SAMP                    1 

#define UART_485	            huart1

#define MAX_GET_TEMH_FAIL       10                

/*================ var struct =================*/
typedef enum
{
    _EVENT_TEMH_ENTRY,
    
    _EVENT_POWER_ON_NODE,
    _EVENT_TEMH_READ_VALUE,
    _EVENT_TEMH_LOG_TSVH,             //0
    _EVENT_CONTROL_LED1,         //1
    _EVENT_CONTROL_LED2,
    _EVENT_CONTROL_LED3,
    
    _EVENT_TEST_RS485,
    
	_EVENT_END_TEMP_HUMI,
}eKindEventWm;

typedef enum
{
    _LED_STATUS,
    _LED_GPS,
    _LED_TEMH,
} Led_TypeDef;


typedef enum
{
    _LED_MODE_ONLINE_INIT,
    _LED_MODE_CONNECT_SERVER,
    _LED_MODE_UPDATE_FW,
    _LED_MODE_POWER_SAVE,
    _LED_MODE_TEST_PULSE,
}sKindModeLed;

typedef enum
{
	_ALARM_FLOW_LOW = 2,
	_ALARM_FLOW_HIGH,
	_ALARM_PEAK_LOW,
	_ALARM_PEAK_HIGH,
    _ALARM_VBAT_LOW,
    _ALARM_DIR_CHANGE,
    _ALARM_DETECT_CUT,
    
	_ALARM_END,
}Struct_Type_Alarm;


typedef struct
{
    uint16_t    FlowHigh;
    uint16_t    FlowLow;
    
    uint16_t    PeakHigh;
    uint16_t    PeakLow;
    
    uint8_t     LowBatery;
    
    int16_t     LevelHigh;
    int16_t     LevelLow;
}struct_ThresholdConfig;


typedef struct
{
    int16_t     Val_i16;
    uint16_t    Scale_u8; 
    uint16_t    Unit_u16;
}structFloatValue;


typedef struct
{
    uint8_t             ModBusStatus_u8;
    
    uint8_t             Status_u8[MAX_SLAVE]; 
    uint8_t             aSlaveID[MAX_SLAVE];
    structFloatValue    sTemperature[MAX_SLAVE];
    structFloatValue    sHumidity[MAX_SLAVE]; 
    
    structFloatValue    sCurrent;
    uint8_t             NumSlave_u8;
    uint8_t             PowerStatus_u8;
    uint8_t             CountFailGetTemh_u8;
}STempHumiVariable;


extern Struct_Battery_Status    sBattery;
extern Struct_Battery_Status    sVout;

extern struct_ThresholdConfig   sMeterThreshold;

extern uint8_t aMARK_ALARM[10];
extern uint8_t aMARK_ALARM_PENDING[10];

extern sEvent_struct sEventAppTempH[];

extern STempHumiVariable  sTempHumi;
extern sData   sUart485;
extern uint8_t aUART_485_DATA [64];

/*================ Function =================*/

//Function handler
uint8_t     AppTemH_Task(void);
void        AppTemH_Init (void);

void        AppTemH_Init_Slave_ID (void);
void        AppTemH_Save_Slave_ID (void);

void        AppTemH_Init_Thresh_Measure (void);
void        AppTemH_Save_Thresh_Measure (void);

void        AppTemH_Log_Data_TSVH (void);
uint8_t     AppTemH_Packet_TSVH (uint8_t *pData);
void        AppTemH_485_Read_Value (uint8_t SlaveID, void (*pFuncResetRecvData) (void)) ;
uint8_t     AppTemH_Set_Mode_Led (void);
void        AppTemH_Clear_Before_Recv (void);
uint8_t     AppTemH_Extract_Data (uint8_t SlaveID, uint8_t *pSource, uint16_t Length, structFloatValue *sTemperature, structFloatValue *sHumi);

uint16_t    AppTemH_Get_VBAT_mV(void);
uint16_t    AppTemH_Get_Vout_mV(void);

uint8_t     AppTemh_Check_Status_TempH (void);

void        LED_Toggle (Led_TypeDef Led);
void        LED_On (Led_TypeDef Led);
void        LED_Off (Led_TypeDef Led);

#endif